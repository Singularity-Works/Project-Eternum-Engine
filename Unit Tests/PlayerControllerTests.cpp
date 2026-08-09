/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PlayerControllerTests
* Description:
*     Tests movement on the grid, including walls and map edges blocking a step, and the
*     Glyph component that puts the player on screen.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <sstream>

class PlayerControllerTest : public ::testing::Test {
protected:
    Entity* player = nullptr;
    PlayerController* controller = nullptr;
    Transform* transform = nullptr;

    void SetUp() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();

        // a 5x5 room walled all the way round, floor in the middle
        GridSystem::GetInstance()->CreateMap("TestRoom", GridSystem::Dimension(5, 5), '#');
        GridSystem::GetInstance()->LoadMap("TestRoom");
        for (int y = 1; y <= 3; ++y)
            for (int x = 1; x <= 3; ++x)
                GridSystem::GetInstance()->SetCell(x, y, '.');

        player = Entities()->CreateEntity("Player");
        transform = new Transform(Vec2f{ 2.0f, 2.0f });
        controller = new PlayerController();
        player->AddComponent(transform);
        player->AddComponent(controller);
        player->AddComponent(new Glyph('@', 10));
    }

    void TearDown() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
    }

    // where the player is standing, in cells
    static Vec2i Cell(const Transform* t) {
        return Vec2i{ static_cast<int>(t->GetTranslation().x()),
                      static_cast<int>(t->GetTranslation().y()) };
    }
};

// ----------------------------
// Walkability
// ----------------------------

TEST_F(PlayerControllerTest, FloorIsWalkableAndWallsAreNot) {
    EXPECT_TRUE(GridSystem::GetInstance()->IsWalkable(2, 2));
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(0, 0));
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(4, 2));
}

TEST_F(PlayerControllerTest, OffTheMapIsNeverWalkable) {
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(-1, 2));
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(2, -1));
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(99, 2));
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(2, 99));
}

TEST_F(PlayerControllerTest, WithNoMapLoadedNothingIsWalkable) {
    GridSystem::GetInstance()->ClearMaps();
    EXPECT_FALSE(GridSystem::GetInstance()->IsWalkable(2, 2));
}

// ----------------------------
// Movement
// ----------------------------

TEST_F(PlayerControllerTest, MovesOntoOpenFloor) {
    EXPECT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));

    EXPECT_EQ(Cell(transform), (Vec2i{ 3, 2 }));
    EXPECT_EQ(controller->GetStepsTaken(), 1);
}

TEST_F(PlayerControllerTest, EveryDirectionWorks) {
    ASSERT_TRUE(controller->TryMove(Vec2i{ 0, -1 }));
    EXPECT_EQ(Cell(transform), (Vec2i{ 2, 1 }));

    ASSERT_TRUE(controller->TryMove(Vec2i{ 0, 1 }));
    EXPECT_EQ(Cell(transform), (Vec2i{ 2, 2 }));

    ASSERT_TRUE(controller->TryMove(Vec2i{ -1, 0 }));
    EXPECT_EQ(Cell(transform), (Vec2i{ 1, 2 }));

    ASSERT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(Cell(transform), (Vec2i{ 2, 2 }));

    EXPECT_EQ(controller->GetStepsTaken(), 4);
}

TEST_F(PlayerControllerTest, WallsBlockTheStep) {
    // walk to the west floor tile, the next step west is wall
    ASSERT_TRUE(controller->TryMove(Vec2i{ -1, 0 }));
    ASSERT_EQ(Cell(transform), (Vec2i{ 1, 2 }));

    EXPECT_FALSE(controller->TryMove(Vec2i{ -1, 0 }));

    // the player did not move and the blocked step did not count
    EXPECT_EQ(Cell(transform), (Vec2i{ 1, 2 }));
    EXPECT_EQ(controller->GetStepsTaken(), 1);
}

TEST_F(PlayerControllerTest, CannotWalkOffTheMap) {
    // the map edge is wall, so the player is boxed in regardless of direction
    for (int i = 0; i < 10; ++i)
        controller->TryMove(Vec2i{ -1, 0 });

    EXPECT_TRUE(GridSystem::GetInstance()->IsWalkable(Cell(transform).x(), Cell(transform).y()));
    EXPECT_EQ(Cell(transform), (Vec2i{ 1, 2 }));
}

TEST_F(PlayerControllerTest, ResetStepsClearsTheCounter) {
    ASSERT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));
    ASSERT_EQ(controller->GetStepsTaken(), 1);

    controller->ResetSteps();

    EXPECT_EQ(controller->GetStepsTaken(), 0);
}

TEST_F(PlayerControllerTest, WithNoDirectionHeldNothingMoves) {
    const Vec2i before = Cell(transform);

    // no keys are down in a test, so this is the idle case
    EXPECT_EQ(controller->GetHeldDirection(), (Vec2i{ 0, 0 }));

    for (int frame = 0; frame < 60; ++frame)
        controller->OnUpdate(0.016);

    EXPECT_EQ(Cell(transform), before);
    EXPECT_EQ(controller->GetStepsTaken(), 0);
}

TEST_F(PlayerControllerTest, TheMoveIntervalIsAdjustable) {
    EXPECT_DOUBLE_EQ(controller->GetMoveInterval(), PlayerController::DEFAULT_MOVE_INTERVAL);

    controller->SetMoveInterval(0.25);
    EXPECT_DOUBLE_EQ(controller->GetMoveInterval(), 0.25);
}

TEST_F(PlayerControllerTest, AControllerWithNoTransformDoesNotMoveOrCrash) {
    Entity* ghost = Entities()->CreateEntity("Ghost");
    auto* lonely = new PlayerController();
    ghost->AddComponent(lonely);

    EXPECT_FALSE(lonely->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(lonely->GetStepsTaken(), 0);
}

// ----------------------------
// Glyph
// ----------------------------

TEST_F(PlayerControllerTest, GlyphCarriesSymbolAndDrawOrder) {
    const Glyph* glyph = player->GetComponent<Glyph>();

    ASSERT_NE(glyph, nullptr);
    EXPECT_EQ(glyph->GetSymbol(), '@');
    EXPECT_EQ(glyph->GetDrawOrder(), 10);
    EXPECT_TRUE(glyph->IsVisible());
}

TEST(GlyphTests, DefaultsToAPlaceholderSymbol) {
    const Glyph glyph;

    EXPECT_EQ(glyph.GetSymbol(), '?');
    EXPECT_EQ(glyph.GetDrawOrder(), 0);
    EXPECT_TRUE(glyph.IsVisible());
}

TEST(GlyphTests, CloneCopiesTheSymbol) {
    Glyph original('g', 5);
    original.SetVisible(false);

    Component* clone = original.Clone();
    auto* cloned = dynamic_cast<Glyph*>(clone);

    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->GetSymbol(), 'g');
    EXPECT_EQ(cloned->GetDrawOrder(), 5);
    EXPECT_FALSE(cloned->IsVisible());
    EXPECT_NE(cloned->GetId(), original.GetId());

    delete clone;
}

// ----------------------------
// Rendering
// ----------------------------

TEST_F(PlayerControllerTest, RenderingDrawsThePlayerWithoutTouchingTheMap) {
    // a full repaint, otherwise the renderer only emits what changed since the last test
    GridSystem::GetInstance()->ForceFullRedraw();

    // swallow the console output, this test only cares about the map afterwards
    std::stringstream sink;
    std::streambuf* previous = std::cout.rdbuf(sink.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(previous);

    EXPECT_NE(sink.str().find('@'), std::string::npos);

    // the stored map still has plain floor where the player is standing
    EXPECT_EQ(GridSystem::GetInstance()->GetCell(2, 2), '.');
}

TEST_F(PlayerControllerTest, AStepRepaintsOnlyWhatMoved) {
    GridSystem::GetInstance()->ForceFullRedraw();

    std::stringstream firstFrame;
    std::streambuf* previous = std::cout.rdbuf(firstFrame.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(previous);

    ASSERT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));

    std::stringstream secondFrame;
    previous = std::cout.rdbuf(secondFrame.rdbuf());
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(previous);

    // the first frame paints the whole map, the second paints two cells and a status line
    EXPECT_FALSE(firstFrame.str().empty());
    EXPECT_FALSE(secondFrame.str().empty());
    EXPECT_LT(secondFrame.str().size(), firstFrame.str().size())
        << "a step should cost less than a full repaint";
}

TEST_F(PlayerControllerTest, AnUnchangedFrameWritesNothing) {
    GridSystem::GetInstance()->ForceFullRedraw();

    std::stringstream sink;
    std::streambuf* previous = std::cout.rdbuf(sink.rdbuf());
    GridSystem::GetInstance()->Render();

    // nothing moved, so the dirty flag is down and there is nothing to send
    sink.str("");
    GridSystem::GetInstance()->Render();
    std::cout.rdbuf(previous);

    EXPECT_TRUE(sink.str().empty());
}
