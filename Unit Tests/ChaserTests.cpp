/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: ChaserTests
* Description:
*     Tests the Entity that hunts the player. The interesting part is the state machine:
*     it should only hunt what it can actually see, walk to where the player was last seen
*     after losing them, and fall back to wandering when that turns up nothing.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Solid/Solid.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <Core/Random/Random.h>

class ChaserTest : public ::testing::Test {
protected:
    Entity* player = nullptr;

    void SetUp() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();

        // a 9x5 room, open floor inside a solid border
        GridSystem::GetInstance()->CreateMap("Room", GridSystem::Dimension(9, 5), '#');
        GridSystem::GetInstance()->LoadMap("Room");
        for (int y = 1; y <= 3; ++y)
            for (int x = 1; x <= 7; ++x)
                GridSystem::GetInstance()->SetCell(x, y, '.');

        Paths()->InvalidateNavGrid();
        Paths()->SetAlgorithm(Pathfinding::Algorithm::AStar);

        player = Entities()->CreateEntity("Player");
        player->AddComponent(new Transform(Vec2f{ 7.0f, 2.0f }));
        player->AddComponent(new PlayerController());
        player->AddComponent(new Glyph('@', 10));
        player->AddComponent(new Solid());
    }

    void TearDown() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();
        Paths()->InvalidateNavGrid();
    }

    static Chaser* SpawnChaser(const std::string& name, const Vec2i& cell) {
        Entity* enemy = Entities()->CreateEntity(name);
        enemy->AddComponent(new Transform(Vec2f{ static_cast<float>(cell.x()),
                                                 static_cast<float>(cell.y()) }));
        enemy->AddComponent(new Glyph(Chaser::GetSymbolFor(ChaserState::Roaming), 5));
        enemy->AddComponent(new Solid());

        auto* chaser = new Chaser();
        enemy->AddComponent(chaser);
        return chaser;
    }

    static Vec2i CellOf(const Entity* entity) {
        const Vec2f& position = entity->GetComponent<Transform>()->GetTranslation();
        return Vec2i{ static_cast<int>(position.x()), static_cast<int>(position.y()) };
    }

    static void MoveTo(Entity* entity, const Vec2i& cell) {
        entity->GetComponent<Transform>()->SetTranslation(
            Vec2f{ static_cast<float>(cell.x()), static_cast<float>(cell.y()) });
    }

    // drops a wall in and tells the pathfinding the map moved under it
    static void WallOff(const int x, const int y) {
        GridSystem::GetInstance()->SetCell(x, y, '#');
        Paths()->InvalidateNavGrid();
    }
};

// ----------------------------
// Seeing
// ----------------------------

TEST_F(ChaserTest, AnOpenRoomMeansThePlayerIsVisible) {
    const Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    EXPECT_TRUE(chaser->CanSeeTarget());
}

TEST_F(ChaserTest, AWallBreaksLineOfSight) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    ASSERT_TRUE(chaser->CanSeeTarget());

    // straight between the two of them
    WallOff(4, 2);

    EXPECT_FALSE(chaser->CanSeeTarget());
}

TEST_F(ChaserTest, APlayerBeyondSightRangeIsNotVisible) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->SetSightRange(2);

    // six cells away with nothing in the way, but still too far to notice
    EXPECT_FALSE(chaser->CanSeeTarget());
}

TEST_F(ChaserTest, AMissingTargetIsNotVisible) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->SetTargetName("NobodyHere");

    EXPECT_FALSE(chaser->CanSeeTarget());
}

// ----------------------------
// States
// ----------------------------

TEST_F(ChaserTest, ItStartsOutRoaming) {
    const Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    EXPECT_EQ(chaser->GetState(), ChaserState::Roaming);
}

TEST_F(ChaserTest, SeeingThePlayerStartsAHunt) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();

    EXPECT_EQ(chaser->GetState(), ChaserState::Hunting);
    EXPECT_EQ(chaser->GetDestination(), (Vec2i{ 7, 2 }));
}

TEST_F(ChaserTest, ItKeepsRoamingWhenItCannotSeeAnything) {
    WallOff(4, 2);
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();

    EXPECT_EQ(chaser->GetState(), ChaserState::Roaming);
}

TEST_F(ChaserTest, LosingSightSendsItSearchingWhereTheyWere) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Hunting);

    WallOff(4, 2);
    chaser->LookAround();

    EXPECT_EQ(chaser->GetState(), ChaserState::Searching);

    // it still knows where they were, even though it cannot see them now
    EXPECT_EQ(chaser->GetDestination(), (Vec2i{ 7, 2 }));
}

TEST_F(ChaserTest, ArrivingWhereTheyWereGivesUpAndRoams) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();
    WallOff(4, 2);
    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Searching);

    // stand it on the cell it was heading for, the player is long gone
    MoveTo(chaser->GetEntity(), chaser->GetDestination());
    MoveTo(player, Vec2i{ 1, 1 });

    chaser->TakeStep();

    EXPECT_EQ(chaser->GetState(), ChaserState::Roaming);
}

TEST_F(ChaserTest, SeeingThePlayerAgainCancelsTheSearch) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();
    WallOff(4, 2);
    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Searching);

    // the player steps out from behind the wall
    MoveTo(player, Vec2i{ 5, 1 });
    chaser->LookAround();

    EXPECT_EQ(chaser->GetState(), ChaserState::Hunting);
}

// ----------------------------
// Moving
// ----------------------------

TEST_F(ChaserTest, HuntingStepsTowardsThePlayer) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->LookAround();

    ASSERT_TRUE(chaser->TakeStep());

    EXPECT_EQ(CellOf(chaser->GetEntity()), (Vec2i{ 2, 2 }));
    EXPECT_TRUE(chaser->GetPath().IsValid());
}

TEST_F(ChaserTest, HuntingClosesTheDistance) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    int previous = Pathfinding::ManhattanDistance(CellOf(chaser->GetEntity()), CellOf(player));

    for (int step = 0; step < 4; ++step) {
        chaser->LookAround();
        ASSERT_TRUE(chaser->TakeStep()) << "step " << step;

        const int distance = Pathfinding::ManhattanDistance(CellOf(chaser->GetEntity()),
                                                            CellOf(player));
        EXPECT_LT(distance, previous);
        previous = distance;
    }
}

TEST_F(ChaserTest, ItStopsNextToThePlayerRatherThanOnTop) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 6, 2 });
    chaser->LookAround();

    EXPECT_FALSE(chaser->TakeStep());
    EXPECT_EQ(CellOf(chaser->GetEntity()), (Vec2i{ 6, 2 }));
}

TEST_F(ChaserTest, ARoamingChaserStillWanders) {
    WallOff(4, 2);
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Roaming);

    bool moved = false;
    for (int attempt = 0; attempt < 10 && !moved; ++attempt)
        moved = chaser->TakeStep();

    EXPECT_TRUE(moved);
    EXPECT_NE(CellOf(chaser->GetEntity()), (Vec2i{ 1, 2 }));
}

TEST_F(ChaserTest, ItRoutesAroundAWallToReachThePlayer) {
    // wall the middle column except the bottom row, so there is one way through
    GridSystem::GetInstance()->SetCell(4, 1, '#');
    WallOff(4, 2);

    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    // force a hunt even though the wall hides the player, to test routing not seeing
    MoveTo(player, Vec2i{ 7, 2 });
    chaser->SetSightRange(99);
    MoveTo(chaser->GetEntity(), Vec2i{ 3, 3 });
    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Hunting);

    ASSERT_TRUE(chaser->TakeStep());

    const bool usesTheGap = std::any_of(
        chaser->GetPath().m_Cells.begin(), chaser->GetPath().m_Cells.end(),
        [](Vec2i const& cell) { return cell == Vec2i{ 4, 3 }; });

    EXPECT_TRUE(usesTheGap);
}

TEST_F(ChaserTest, ChasersDoNotStackOnTopOfEachOther) {
    Chaser* behind = SpawnChaser("EnemyBehind", Vec2i{ 1, 2 });
    SpawnChaser("EnemyAhead", Vec2i{ 2, 2 });

    behind->LookAround();

    // the only step towards the player is already taken
    EXPECT_FALSE(behind->TakeStep());
    EXPECT_EQ(CellOf(behind->GetEntity()), (Vec2i{ 1, 2 }));
}

// ----------------------------
// Pacing
// ----------------------------

TEST_F(ChaserTest, TheCooldownPacesMovementInsteadOfTheFrameRate) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->OnUpdate(0.016);
    const Vec2i afterFirst = CellOf(chaser->GetEntity());
    ASSERT_NE(afterFirst, (Vec2i{ 1, 2 }));

    // a second frame arrives almost immediately, far too soon to move again
    chaser->OnUpdate(0.016);
    EXPECT_EQ(CellOf(chaser->GetEntity()), afterFirst);

    // let the cooldown run out
    for (int frame = 0; frame < 20; ++frame)
        chaser->OnUpdate(0.016);

    EXPECT_NE(CellOf(chaser->GetEntity()), afterFirst);
}

TEST_F(ChaserTest, ARoamingChaserKeepsMovingWithNobodyAround) {
    // an empty room, so nothing can be hunted and nothing can be bumped into
    Entities()->ClearEntities();
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->SetTargetName("NobodyHere");

    Vec2i previous = CellOf(chaser->GetEntity());
    int moves = 0;

    // nine seconds of frames, and not one keypress in any of them
    for (int frame = 0; frame < 600; ++frame) {
        chaser->OnUpdate(0.016);

        const Vec2i now = CellOf(chaser->GetEntity());
        if (!(now == previous)) {
            ++moves;
            previous = now;
        }
    }

    // this is the whole point of real time, it acts on its own clock not on the player's
    EXPECT_GT(moves, 10);
    EXPECT_EQ(chaser->GetState(), ChaserState::Roaming);
}

TEST_F(ChaserTest, AFasterFrameRateDoesNotMakeItFaster) {
    // one lone chaser from a known random stream each time, so the only thing that differs
    // between the two runs is how the same seconds are chopped into frames
    const auto countMoves = [](const double step, const int frames) {
        Entities()->ClearEntities();
        Rng().Seed(2024);

        Chaser* chaser = SpawnChaser("Lonely", Vec2i{ 1, 2 });
        chaser->SetTargetName("NobodyHere");

        Vec2i previous = CellOf(chaser->GetEntity());
        int moves = 0;

        for (int frame = 0; frame < frames; ++frame) {
            chaser->OnUpdate(step);

            const Vec2i now = CellOf(chaser->GetEntity());
            if (!(now == previous)) {
                ++moves;
                previous = now;
            }
        }

        return moves;
    };

    // the same three seconds, once at 30fps and once at 120fps
    const int atThirty = countMoves(1.0 / 30.0, 90);
    const int atOneTwenty = countMoves(1.0 / 120.0, 360);

    EXPECT_GT(atThirty, 0);
    EXPECT_NEAR(atThirty, atOneTwenty, 1);
}

TEST_F(ChaserTest, ARoamingChaserRoutesAroundBodiesInsteadOfJamming) {
    // the pathfinding only knows about walls, not about who is standing where, so a route
    // can quite happily lead straight through another body. what matters is what happens
    // when the step is refused
    Entities()->ClearEntities();

    SpawnChaser("Blocker", Vec2i{ 2, 2 });
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->SetTargetName("NobodyHere");

    Vec2i previous = CellOf(chaser->GetEntity());
    int moves = 0;

    for (int frame = 0; frame < 600; ++frame) {
        chaser->OnUpdate(0.016);

        const Vec2i now = CellOf(chaser->GetEntity());
        if (!(now == previous)) {
            ++moves;
            previous = now;
        }
    }

    // one body in the way must not stop it wandering, it should pick somewhere else
    // rather than pushing at the same cell for nine seconds
    EXPECT_GT(moves, 5);
}

TEST_F(ChaserTest, HuntingIsFasterThanRoaming) {
    EXPECT_LT(Chaser::HUNT_INTERVAL, Chaser::ROAM_INTERVAL);
}

TEST_F(ChaserTest, ResetPutsItBackToRoaming) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    chaser->LookAround();
    ASSERT_EQ(chaser->GetState(), ChaserState::Hunting);

    chaser->Reset();

    EXPECT_EQ(chaser->GetState(), ChaserState::Roaming);
    EXPECT_FALSE(chaser->GetPath().IsValid());
    EXPECT_EQ(chaser->GetDestination(), (Vec2i{ -1, -1 }));
}

// ----------------------------
// What The Player Sees
// ----------------------------

TEST_F(ChaserTest, TheSymbolShowsWhatItIsDoing) {
    EXPECT_EQ(Chaser::GetSymbolFor(ChaserState::Roaming), 'e');
    EXPECT_EQ(Chaser::GetSymbolFor(ChaserState::Hunting), 'E');
    EXPECT_EQ(Chaser::GetSymbolFor(ChaserState::Searching), 'E');

    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    const Glyph* glyph = chaser->GetEntity()->GetComponent<Glyph>();
    ASSERT_NE(glyph, nullptr);
    EXPECT_EQ(glyph->GetSymbol(), 'e');

    chaser->LookAround();
    EXPECT_EQ(glyph->GetSymbol(), 'E');
}

TEST_F(ChaserTest, EveryStateHasAName) {
    EXPECT_EQ(Chaser::GetStateName(ChaserState::Roaming), "roaming");
    EXPECT_EQ(Chaser::GetStateName(ChaserState::Hunting), "hunting");
    EXPECT_EQ(Chaser::GetStateName(ChaserState::Searching), "searching");
}

TEST_F(ChaserTest, TheRouteIsHandedToTheGridToDraw) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    GridSystem::GetInstance()->SetOverlayVisible(true);

    chaser->LookAround();
    ASSERT_TRUE(chaser->TakeStep());
    GridSystem::GetInstance()->ForceFullRedraw();

    std::stringstream sink;
    std::streambuf* redirected = std::cout.rdbuf(sink.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(redirected);

    EXPECT_NE(sink.str().find(GridSystem::OVERLAY_SYMBOL), std::string::npos);
}

TEST_F(ChaserTest, HidingTheTrailKeepsItOffTheScreen) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });

    chaser->LookAround();
    ASSERT_TRUE(chaser->TakeStep());
    GridSystem::GetInstance()->SetOverlayVisible(false);
    GridSystem::GetInstance()->ForceFullRedraw();

    std::stringstream sink;
    std::streambuf* redirected = std::cout.rdbuf(sink.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(redirected);

    EXPECT_EQ(sink.str().find(GridSystem::OVERLAY_SYMBOL), std::string::npos);

    GridSystem::GetInstance()->SetOverlayVisible(true);
}

TEST_F(ChaserTest, LeavingTheSceneTakesTheTrailWithIt) {
    Chaser* chaser = SpawnChaser("Enemy", Vec2i{ 1, 2 });
    GridSystem::GetInstance()->SetOverlayVisible(true);

    chaser->LookAround();
    ASSERT_TRUE(chaser->TakeStep());

    chaser->GetEntity()->Destroy();
    Entities()->Update(0.016);

    GridSystem::GetInstance()->ForceFullRedraw();

    std::stringstream sink;
    std::streambuf* redirected = std::cout.rdbuf(sink.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(redirected);

    EXPECT_EQ(sink.str().find(GridSystem::OVERLAY_SYMBOL), std::string::npos);
}
