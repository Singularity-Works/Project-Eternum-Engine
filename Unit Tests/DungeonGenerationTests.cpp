/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: DungeonGenerationTests
* Description:
*     Tests the generation algorithms. The two properties that matter are that a seed
*     always rebuilds the same dungeon, and that every dungeon is one connected space
*     with no room walled off from the rest. Both are checked across many seeds.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/Random/Random.h>
#include <Systems/Dungeon System/Generation/BspGenerator.h>
#include <Systems/Dungeon System/Generation/CaveGenerator.h>
#include <Systems/Dungeon System/Generation/CorridorGraph.h>
#include <Systems/Dungeon System/Generation/GridRegions.h>
#include <Systems/Dungeon System/Generation/RoomGenerator.h>
#include <Systems/Dungeon System/DungeonSystem.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Core/ECS/Component/Transform/Transform.h>

namespace
{
    constexpr int WIDTH  = 60;
    constexpr int HEIGHT = 22;

    // enough seeds to catch an algorithm that only usually works
    constexpr int SEED_SAMPLES = 40;

    // flattens a map so two of them can be compared directly
    std::string Fingerprint(GridSystem::Grid const& grid) {
        return std::string(grid.cells.begin(), grid.cells.end());
    }

    std::vector<const DungeonGenerator*> AllGenerators() {
        static const BspGenerator  bsp;
        static const RoomGenerator rooms;
        static const CaveGenerator cave;
        return { &bsp, &rooms, &cave };
    }
}

// ----------------------------
// Random
// ----------------------------

TEST(RandomTests, SameSeedGivesTheSameSequence) {
    Random a(1234);
    Random b(1234);

    for (int i = 0; i < 50; ++i)
        EXPECT_EQ(a.Range(0, 1000), b.Range(0, 1000));
}

TEST(RandomTests, DifferentSeedsDiverge) {
    Random a(1);
    Random b(2);

    bool sawDifference = false;
    for (int i = 0; i < 50 && !sawDifference; ++i)
        sawDifference = a.Range(0, 1000000) != b.Range(0, 1000000);

    EXPECT_TRUE(sawDifference);
}

TEST(RandomTests, RangeStaysInBounds) {
    Random rng(7);

    for (int i = 0; i < 200; ++i) {
        const int value = rng.Range(3, 9);
        EXPECT_GE(value, 3);
        EXPECT_LE(value, 9);
    }
}

TEST(RandomTests, BackwardsRangeIsSwappedNotUndefined) {
    Random rng(7);

    for (int i = 0; i < 50; ++i) {
        const int value = rng.Range(9, 3);
        EXPECT_GE(value, 3);
        EXPECT_LE(value, 9);
    }
}

TEST(RandomTests, ChanceHonoursTheExtremes) {
    Random rng(7);

    for (int i = 0; i < 50; ++i) {
        EXPECT_FALSE(rng.Chance(0));
        EXPECT_TRUE(rng.Chance(100));
    }
}

TEST(RandomTests, IndexOfAnEmptyContainerIsZero) {
    Random rng(7);
    EXPECT_EQ(rng.Index(0), 0u);
}

TEST(RandomTests, ReseedingBackToBackStillChangesTheSeed) {
    // time() only ticks once a second, which used to make two dungeons in a row identical
    Random rng(1);
    const unsigned first = rng.Reseed();
    const unsigned second = rng.Reseed();

    EXPECT_NE(first, second);
}

// ----------------------------
// Regions
// ----------------------------

TEST(GridRegionsTests, FindsSeparateRegions) {
    GridSystem::Grid grid(5, 5, Tiles::WALL);
    grid.SetCell(1, 1, Tiles::FLOOR);
    grid.SetCell(3, 3, Tiles::FLOOR);
    grid.SetCell(3, 4, Tiles::FLOOR);

    const auto regions = GridRegions::FindRegions(grid);

    ASSERT_EQ(regions.size(), 2u);

    // largest first
    EXPECT_EQ(regions[0].size(), 2u);
    EXPECT_EQ(regions[1].size(), 1u);
    EXPECT_FALSE(GridRegions::IsFullyConnected(grid));
}

TEST(GridRegionsTests, AnEmptyMapIsConsideredConnected) {
    const GridSystem::Grid grid(5, 5, Tiles::WALL);

    EXPECT_TRUE(GridRegions::FindRegions(grid).empty());
    EXPECT_TRUE(GridRegions::IsFullyConnected(grid));
    EXPECT_EQ(GridRegions::CountFloor(grid), 0);
}

TEST(GridRegionsTests, KeepLargestWallsOffEverythingElse) {
    GridSystem::Grid grid(5, 5, Tiles::WALL);
    grid.SetCell(1, 1, Tiles::FLOOR);
    grid.SetCell(3, 3, Tiles::FLOOR);
    grid.SetCell(3, 4, Tiles::FLOOR);

    const int filled = GridRegions::KeepLargestRegion(grid);

    EXPECT_EQ(filled, 1);
    EXPECT_EQ(grid.GetCell(1, 1), Tiles::WALL);
    EXPECT_EQ(grid.GetCell(3, 3), Tiles::FLOOR);
    EXPECT_TRUE(GridRegions::IsFullyConnected(grid));
}

// ----------------------------
// Corridor Graph
// ----------------------------

TEST(CorridorGraphTests, SpanningTreeReachesEveryRoomExactlyOnce) {
    const std::vector<Room> rooms = {
        Room(0, 0, 3, 3), Room(20, 0, 3, 3), Room(0, 15, 3, 3), Room(20, 15, 3, 3)
    };

    const auto tree = CorridorGraph::BuildMinimumSpanningTree(rooms);

    // a tree over n nodes always has exactly n - 1 edges
    ASSERT_EQ(tree.size(), rooms.size() - 1);

    // and every room has to appear in it somewhere
    std::vector<bool> reached(rooms.size(), false);
    reached[0] = true;
    for (const auto& edge : tree) {
        reached[edge.m_A] = true;
        reached[edge.m_B] = true;
    }
    EXPECT_TRUE(std::all_of(reached.begin(), reached.end(), [](const bool r) { return r; }));
}

TEST(CorridorGraphTests, FewerThanTwoRoomsNeedsNoCorridors) {
    EXPECT_TRUE(CorridorGraph::BuildMinimumSpanningTree({}).empty());
    EXPECT_TRUE(CorridorGraph::BuildMinimumSpanningTree({ Room(0, 0, 3, 3) }).empty());
}

TEST(CorridorGraphTests, ExtraEdgesNeverRepeatOneAlreadyInTheTree) {
    const std::vector<Room> rooms = {
        Room(0, 0, 3, 3), Room(20, 0, 3, 3), Room(0, 15, 3, 3), Room(20, 15, 3, 3)
    };

    Random rng(99);
    const auto tree = CorridorGraph::BuildMinimumSpanningTree(rooms);
    const auto extras = CorridorGraph::PickExtraEdges(rooms, tree, 2, rng);

    EXPECT_FALSE(extras.empty());

    for (const auto& extra : extras) {
        const bool duplicate = std::any_of(tree.begin(), tree.end(),
            [&extra](CorridorGraph::Edge const& edge) { return edge.SamePair(extra); });
        EXPECT_FALSE(duplicate);
    }
}

TEST(CorridorGraphTests, ACarvedCorridorJoinsBothEnds) {
    GridSystem::Grid grid(20, 12, Tiles::WALL);
    Random rng(5);

    CorridorGraph::CarveCorridor(grid, Vec2i{ 2, 2 }, Vec2i{ 17, 9 }, rng);

    EXPECT_EQ(grid.GetCell(2, 2), Tiles::FLOOR);
    EXPECT_EQ(grid.GetCell(17, 9), Tiles::FLOOR);

    // the whole corridor is one unbroken run, no gap at the corner
    EXPECT_TRUE(GridRegions::IsFullyConnected(grid));
}

// ----------------------------
// Generators
// ----------------------------

TEST(DungeonGenerationTests, EverySeedProducesOneConnectedSpace) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        for (int seed = 1; seed <= SEED_SAMPLES; ++seed) {
            Random rng(static_cast<unsigned>(seed));
            const DungeonLayout layout = generator->Generate(WIDTH, HEIGHT, rng);

            EXPECT_TRUE(GridRegions::IsFullyConnected(layout.m_Grid))
                << generator->GetName() << " left an unreachable pocket at seed " << seed;
        }
    }
}

TEST(DungeonGenerationTests, EverySeedProducesSomewhereToStand) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        for (int seed = 1; seed <= SEED_SAMPLES; ++seed) {
            Random rng(static_cast<unsigned>(seed));
            const DungeonLayout layout = generator->Generate(WIDTH, HEIGHT, rng);

            EXPECT_GT(GridRegions::CountFloor(layout.m_Grid), 0)
                << generator->GetName() << " produced a solid map at seed " << seed;
        }
    }
}

TEST(DungeonGenerationTests, TheSameSeedRebuildsTheSameDungeon) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        Random first(4242);
        Random second(4242);

        const DungeonLayout a = generator->Generate(WIDTH, HEIGHT, first);
        const DungeonLayout b = generator->Generate(WIDTH, HEIGHT, second);

        EXPECT_EQ(Fingerprint(a.m_Grid), Fingerprint(b.m_Grid)) << generator->GetName();
        EXPECT_EQ(a.m_Rooms.size(), b.m_Rooms.size()) << generator->GetName();
    }
}

TEST(DungeonGenerationTests, DifferentSeedsProduceDifferentDungeons) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        Random first(1);
        Random second(2);

        const DungeonLayout a = generator->Generate(WIDTH, HEIGHT, first);
        const DungeonLayout b = generator->Generate(WIDTH, HEIGHT, second);

        EXPECT_NE(Fingerprint(a.m_Grid), Fingerprint(b.m_Grid)) << generator->GetName();
    }
}

TEST(DungeonGenerationTests, TheBorderIsAlwaysSolid) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        for (int seed = 1; seed <= 10; ++seed) {
            Random rng(static_cast<unsigned>(seed));
            const DungeonLayout layout = generator->Generate(WIDTH, HEIGHT, rng);

            for (int x = 0; x < WIDTH; ++x) {
                ASSERT_EQ(layout.m_Grid.GetCell(x, 0), Tiles::WALL) << generator->GetName();
                ASSERT_EQ(layout.m_Grid.GetCell(x, HEIGHT - 1), Tiles::WALL) << generator->GetName();
            }
            for (int y = 0; y < HEIGHT; ++y) {
                ASSERT_EQ(layout.m_Grid.GetCell(0, y), Tiles::WALL) << generator->GetName();
                ASSERT_EQ(layout.m_Grid.GetCell(WIDTH - 1, y), Tiles::WALL) << generator->GetName();
            }
        }
    }
}

TEST(DungeonGenerationTests, ATinyMapIsHandledInsteadOfCrashing) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        Random rng(1);
        const DungeonLayout layout = generator->Generate(3, 3, rng);

        EXPECT_EQ(layout.m_Grid.m_Dimension.m_Width, 3) << generator->GetName();
        EXPECT_EQ(layout.m_Grid.m_Dimension.m_Height, 3) << generator->GetName();
    }
}

// ----------------------------
// Room Placement
// ----------------------------

TEST(DungeonGenerationTests, BspRoomsNeverOverlap) {
    const BspGenerator generator;

    for (int seed = 1; seed <= SEED_SAMPLES; ++seed) {
        Random rng(static_cast<unsigned>(seed));
        const DungeonLayout layout = generator.Generate(WIDTH, HEIGHT, rng);

        ASSERT_FALSE(layout.m_Rooms.empty()) << "seed " << seed;

        for (std::size_t a = 0; a < layout.m_Rooms.size(); ++a)
            for (std::size_t b = a + 1; b < layout.m_Rooms.size(); ++b)
                EXPECT_FALSE(layout.m_Rooms[a].Intersects(layout.m_Rooms[b]))
                    << "seed " << seed << " rooms " << a << " and " << b;
    }
}

TEST(DungeonGenerationTests, ScatteredRoomsNeverOverlap) {
    const RoomGenerator generator;

    for (int seed = 1; seed <= SEED_SAMPLES; ++seed) {
        Random rng(static_cast<unsigned>(seed));
        const DungeonLayout layout = generator.Generate(WIDTH, HEIGHT, rng);

        ASSERT_FALSE(layout.m_Rooms.empty()) << "seed " << seed;

        for (std::size_t a = 0; a < layout.m_Rooms.size(); ++a)
            for (std::size_t b = a + 1; b < layout.m_Rooms.size(); ++b)
                EXPECT_FALSE(layout.m_Rooms[a].Intersects(layout.m_Rooms[b]))
                    << "seed " << seed << " rooms " << a << " and " << b;
    }
}

TEST(DungeonGenerationTests, EveryRoomIsInsideTheMap) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        for (int seed = 1; seed <= 10; ++seed) {
            Random rng(static_cast<unsigned>(seed));
            const DungeonLayout layout = generator->Generate(WIDTH, HEIGHT, rng);

            for (Room const& room : layout.m_Rooms) {
                EXPECT_GE(room.Left(), 1) << generator->GetName();
                EXPECT_GE(room.Top(), 1) << generator->GetName();
                EXPECT_LE(room.Right(), WIDTH - 2) << generator->GetName();
                EXPECT_LE(room.Bottom(), HEIGHT - 2) << generator->GetName();
            }
        }
    }
}

TEST(DungeonGenerationTests, ARoomCentreIsAlwaysStandableFloor) {
    for (const DungeonGenerator* generator : AllGenerators()) {
        for (int seed = 1; seed <= 10; ++seed) {
            Random rng(static_cast<unsigned>(seed));
            const DungeonLayout layout = generator->Generate(WIDTH, HEIGHT, rng);

            for (Room const& room : layout.m_Rooms) {
                const Vec2i centre = room.Center();
                EXPECT_EQ(layout.m_Grid.GetCell(centre.x(), centre.y()), Tiles::FLOOR)
                    << generator->GetName() << " seed " << seed;
            }
        }
    }
}

// ----------------------------
// Generator Selection
// ----------------------------

TEST(DungeonGenerationTests, KeysAreUniqueAndLowercase) {
    std::vector<std::string> seen;

    for (const DungeonGenerator* generator : AllGenerators()) {
        const std::string key = generator->GetKey();

        EXPECT_FALSE(key.empty());

        const bool lowercase = std::none_of(key.begin(), key.end(),
            [](const unsigned char c) { return std::isupper(c) != 0; });
        EXPECT_TRUE(lowercase) << key << " should be lowercase";

        EXPECT_EQ(std::find(seen.begin(), seen.end(), key), seen.end()) << key << " is used twice";
        seen.push_back(key);
    }
}

TEST(DungeonGenerationTests, EachKeySelectsADifferentAlgorithm) {
    DungeonSystem* dungeons = DungeonSystem::GetInstance().get();
    const std::size_t count = dungeons->GetGeneratorCount();

    const std::size_t bsp   = dungeons->FindGenerator("bsp");
    const std::size_t rooms = dungeons->FindGenerator("rooms");
    const std::size_t cave  = dungeons->FindGenerator("cave");

    ASSERT_LT(bsp, count);
    ASSERT_LT(rooms, count);
    ASSERT_LT(cave, count);

    // "rooms" used to match "BSP rooms" as well, which picked the wrong algorithm
    EXPECT_NE(bsp, rooms);
    EXPECT_NE(rooms, cave);
    EXPECT_NE(bsp, cave);
}

TEST(DungeonGenerationTests, KeyLookupIgnoresCaseAndRejectsNonsense) {
    DungeonSystem* dungeons = DungeonSystem::GetInstance().get();

    EXPECT_EQ(dungeons->FindGenerator("CAVE"), dungeons->FindGenerator("cave"));
    EXPECT_EQ(dungeons->FindGenerator("not a generator"), dungeons->GetGeneratorCount());
    EXPECT_EQ(dungeons->FindGenerator(""), dungeons->GetGeneratorCount());
}

// ----------------------------
// End To End
// ----------------------------

TEST(DungeonSystemTests, ASeedRebuildsTheMapAndTheSpawnTogether) {
    Entities()->ClearEntities();
    DungeonSystem* dungeons = DungeonSystem::GetInstance().get();

    // the map used to reproduce but the spawn did not, because placement drew from
    // the shared source instead of the seeded one
    dungeons->StartNewDungeon(42);
    const std::string firstMap = Fingerprint(dungeons->GetCurrentGrid());
    const Vec2f firstSpawn =
        Entities()->FindEntity("Player")->GetComponent<Transform>()->GetTranslation();

    dungeons->StartNewDungeon(42);
    const std::string secondMap = Fingerprint(dungeons->GetCurrentGrid());
    const Vec2f secondSpawn =
        Entities()->FindEntity("Player")->GetComponent<Transform>()->GetTranslation();

    EXPECT_EQ(firstMap, secondMap);
    EXPECT_TRUE(firstSpawn.AlmostEqual(secondSpawn));

    Entities()->ClearEntities();
    GridSystem::GetInstance()->ClearMaps();
}

TEST(DungeonSystemTests, ThePlayerAlwaysSpawnsSomewhereItCanStand) {
    Entities()->ClearEntities();
    DungeonSystem* dungeons = DungeonSystem::GetInstance().get();

    for (std::size_t generator = 0; generator < dungeons->GetGeneratorCount(); ++generator) {
        dungeons->SetGenerator(generator);

        for (unsigned seed = 1; seed <= 15; ++seed) {
            dungeons->StartNewDungeon(seed);

            Entity* player = Entities()->FindEntity("Player");
            ASSERT_NE(player, nullptr);

            const Vec2f position = player->GetComponent<Transform>()->GetTranslation();
            EXPECT_TRUE(GridSystem::GetInstance()->IsWalkable(
                static_cast<int>(position.x()), static_cast<int>(position.y())))
                << dungeons->GetGeneratorName() << " seed " << seed;
        }
    }

    Entities()->ClearEntities();
    GridSystem::GetInstance()->ClearMaps();
}

// ----------------------------
// Room
// ----------------------------

TEST(RoomTests, EdgesAndCentreAreWhereExpected) {
    const Room room(4, 6, 5, 3);

    EXPECT_EQ(room.Left(), 4);
    EXPECT_EQ(room.Right(), 8);
    EXPECT_EQ(room.Top(), 6);
    EXPECT_EQ(room.Bottom(), 8);
    EXPECT_EQ(room.Center(), (Vec2i{ 6, 7 }));
    EXPECT_EQ(room.Area(), 15);
}

TEST(RoomTests, ContainsCoversTheWholeRectangle) {
    const Room room(4, 6, 5, 3);

    EXPECT_TRUE(room.Contains(4, 6));
    EXPECT_TRUE(room.Contains(8, 8));
    EXPECT_TRUE(room.Contains(6, 7));
    EXPECT_FALSE(room.Contains(3, 6));
    EXPECT_FALSE(room.Contains(9, 8));
}

TEST(RoomTests, PaddingForcesAWallBetweenRooms) {
    const Room room(0, 0, 4, 4);

    // one clear cell between them, fine without padding, rejected with it
    const Room oneApart(5, 0, 4, 4);
    EXPECT_FALSE(room.Intersects(oneApart));
    EXPECT_FALSE(room.Intersects(oneApart, 1));

    // directly touching, no wall between them
    const Room touching(4, 0, 4, 4);
    EXPECT_FALSE(room.Intersects(touching));
    EXPECT_TRUE(room.Intersects(touching, 1));

    const Room overlapping(2, 2, 4, 4);
    EXPECT_TRUE(room.Intersects(overlapping));
}
