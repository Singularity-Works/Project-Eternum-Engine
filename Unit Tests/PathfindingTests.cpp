/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PathfindingTests
* Description:
*     Tests the three routing algorithms against hand built maps, with no engine running.
*     The interesting cases are where they are supposed to disagree: breadth first takes
*     the fewest steps even when they are expensive, Dijkstra takes the cheapest route even
*     when it is longer, and A star matches Dijkstra's answer while opening far fewer cells.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Systems/Pathfinding/NavGrid.h>
#include <Systems/Pathfinding/Pathfinding.h>

using Pathfinding::Algorithm;

namespace
{
    // an open room with a solid wall around the outside
    NavGrid OpenRoom(const int width, const int height) {
        NavGrid grid(width, height);

        for (int x = 0; x < width; ++x) {
            grid.SetCost(x, 0, NavGrid::BLOCKED);
            grid.SetCost(x, height - 1, NavGrid::BLOCKED);
        }
        for (int y = 0; y < height; ++y) {
            grid.SetCost(0, y, NavGrid::BLOCKED);
            grid.SetCost(width - 1, y, NavGrid::BLOCKED);
        }

        return grid;
    }

    std::vector<Algorithm> AllAlgorithms() {
        return { Algorithm::BreadthFirst, Algorithm::Dijkstra, Algorithm::AStar };
    }

    // every step in a route has to be one cell away from the last, no teleporting
    bool IsContiguous(Path const& path) {
        for (std::size_t i = 1; i < path.m_Cells.size(); ++i)
            if (Pathfinding::ManhattanDistance(path.m_Cells[i - 1], path.m_Cells[i]) != 1)
                return false;

        return true;
    }
}

// ----------------------------
// NavGrid
// ----------------------------

TEST(NavGridTests, OffTheMapIsAlwaysBlocked) {
    const NavGrid grid(5, 5);

    EXPECT_FALSE(grid.Contains(-1, 0));
    EXPECT_FALSE(grid.Contains(0, 5));
    EXPECT_EQ(grid.GetCost(-1, 0), NavGrid::BLOCKED);
    EXPECT_EQ(grid.GetCost(99, 99), NavGrid::BLOCKED);
    EXPECT_FALSE(grid.IsWalkable(-1, -1));
}

TEST(NavGridTests, CostsRoundTrip) {
    NavGrid grid(5, 5);

    EXPECT_EQ(grid.GetCost(2, 2), NavGrid::NORMAL_COST);

    grid.SetCost(2, 2, 7);
    EXPECT_EQ(grid.GetCost(2, 2), 7);
    EXPECT_TRUE(grid.IsWalkable(2, 2));

    grid.SetCost(2, 2, NavGrid::BLOCKED);
    EXPECT_FALSE(grid.IsWalkable(2, 2));
}

TEST(NavGridTests, IndexAndCellAreInverses) {
    const NavGrid grid(7, 4);

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 7; ++x)
            EXPECT_EQ(grid.ToCell(grid.ToIndex(x, y)), (Vec2i{ x, y }));
}

TEST(NavGridTests, CheapestStepIgnoresWalls) {
    NavGrid grid(5, 5, 4);
    grid.SetCost(1, 1, NavGrid::BLOCKED);
    grid.SetCost(2, 2, 2);

    // a wall costs zero but you cannot walk on it, so it does not count
    EXPECT_EQ(grid.GetCheapestStep(), 2);
}

TEST(NavGridTests, BuildsFromAMapUsingTheBlockingTiles) {
    GridSystem::Grid map(4, 3, '.');
    map.SetCell(1, 1, '#');
    map.SetCell(2, 1, '~');

    const NavGrid grid = NavGrid::FromGrid(map, "#~");

    EXPECT_TRUE(grid.IsWalkable(0, 0));
    EXPECT_FALSE(grid.IsWalkable(1, 1));
    EXPECT_FALSE(grid.IsWalkable(2, 1));
}

TEST(NavGridTests, ADegenerateSizeIsEmptyRatherThanBroken) {
    const NavGrid grid(0, 5);

    EXPECT_EQ(grid.GetCellCount(), 0);
    EXPECT_FALSE(grid.IsWalkable(0, 0));
}

// ----------------------------
// Shared Behaviour
// ----------------------------

TEST(PathfindingTests, EveryAlgorithmWalksAContiguousWalkableRoute) {
    const NavGrid grid = OpenRoom(12, 8);

    for (const Algorithm algorithm : AllAlgorithms()) {
        const Path path = Pathfinding::FindPath(grid, Vec2i{ 1, 1 }, Vec2i{ 10, 6 }, algorithm);

        ASSERT_TRUE(path.IsValid()) << Pathfinding::GetAlgorithmName(algorithm);
        EXPECT_EQ(path.m_Cells.front(), (Vec2i{ 1, 1 }));
        EXPECT_EQ(path.m_Cells.back(), (Vec2i{ 10, 6 }));
        EXPECT_TRUE(IsContiguous(path)) << Pathfinding::GetAlgorithmName(algorithm);

        for (Vec2i const& cell : path.m_Cells)
            EXPECT_TRUE(grid.IsWalkable(cell)) << Pathfinding::GetAlgorithmName(algorithm);
    }
}

TEST(PathfindingTests, OnAUniformGridEveryAlgorithmAgrees) {
    const NavGrid grid = OpenRoom(12, 8);

    const Path breadthFirst = Pathfinding::FindPathBreadthFirst(grid, Vec2i{ 1, 1 }, Vec2i{ 10, 6 });
    const Path dijkstra     = Pathfinding::FindPathDijkstra(grid, Vec2i{ 1, 1 }, Vec2i{ 10, 6 });
    const Path aStar        = Pathfinding::FindPathAStar(grid, Vec2i{ 1, 1 }, Vec2i{ 10, 6 });

    // with no walls in the way the shortest route is just the manhattan distance
    EXPECT_EQ(breadthFirst.GetStepCount(), 14);
    EXPECT_EQ(dijkstra.GetStepCount(), 14);
    EXPECT_EQ(aStar.GetStepCount(), 14);
}

TEST(PathfindingTests, StartingOnTheGoalIsAOneCellRoute) {
    const NavGrid grid = OpenRoom(8, 8);

    for (const Algorithm algorithm : AllAlgorithms()) {
        const Path path = Pathfinding::FindPath(grid, Vec2i{ 3, 3 }, Vec2i{ 3, 3 }, algorithm);

        ASSERT_TRUE(path.IsValid()) << Pathfinding::GetAlgorithmName(algorithm);
        EXPECT_EQ(path.m_Cells.size(), 1u);
        EXPECT_EQ(path.GetStepCount(), 0);
        EXPECT_EQ(path.GetNextStep(), (Vec2i{ 3, 3 }));
    }
}

TEST(PathfindingTests, AWalledOffGoalHasNoRoute) {
    NavGrid grid = OpenRoom(9, 5);

    // split the room down the middle
    for (int y = 0; y < 5; ++y)
        grid.SetCost(4, y, NavGrid::BLOCKED);

    for (const Algorithm algorithm : AllAlgorithms()) {
        const Path path = Pathfinding::FindPath(grid, Vec2i{ 1, 2 }, Vec2i{ 7, 2 }, algorithm);

        EXPECT_FALSE(path.IsValid()) << Pathfinding::GetAlgorithmName(algorithm);
        EXPECT_EQ(path.GetStepCount(), 0);
    }
}

TEST(PathfindingTests, AnUnwalkableEndpointHasNoRoute) {
    const NavGrid grid = OpenRoom(8, 8);

    for (const Algorithm algorithm : AllAlgorithms()) {
        EXPECT_FALSE(Pathfinding::FindPath(grid, Vec2i{ 0, 0 }, Vec2i{ 3, 3 }, algorithm).IsValid());
        EXPECT_FALSE(Pathfinding::FindPath(grid, Vec2i{ 3, 3 }, Vec2i{ 0, 0 }, algorithm).IsValid());
    }
}

TEST(PathfindingTests, RoutingRoundAWallTakesTheDetour) {
    NavGrid grid = OpenRoom(9, 7);

    // a wall with one gap at the bottom
    for (int y = 1; y <= 4; ++y)
        grid.SetCost(4, y, NavGrid::BLOCKED);

    for (const Algorithm algorithm : AllAlgorithms()) {
        const Path path = Pathfinding::FindPath(grid, Vec2i{ 2, 2 }, Vec2i{ 6, 2 }, algorithm);

        ASSERT_TRUE(path.IsValid()) << Pathfinding::GetAlgorithmName(algorithm);
        EXPECT_TRUE(IsContiguous(path));

        // it has to go down through the gap, so it costs more than the straight four steps
        EXPECT_GT(path.GetStepCount(), 4) << Pathfinding::GetAlgorithmName(algorithm);

        const bool usesTheGap = std::any_of(path.m_Cells.begin(), path.m_Cells.end(),
            [](Vec2i const& cell) { return cell == Vec2i{ 4, 5 }; });
        EXPECT_TRUE(usesTheGap) << Pathfinding::GetAlgorithmName(algorithm);
    }
}

// ----------------------------
// Where They Disagree
// ----------------------------

TEST(PathfindingTests, CostChangesWhatDijkstraPicksButNotBreadthFirst) {
    NavGrid grid(5, 3);

    // the direct line through the middle is passable but expensive
    grid.SetCost(1, 1, 50);
    grid.SetCost(2, 1, 50);
    grid.SetCost(3, 1, 50);

    const Path breadthFirst = Pathfinding::FindPathBreadthFirst(grid, Vec2i{ 0, 1 }, Vec2i{ 4, 1 });
    const Path dijkstra     = Pathfinding::FindPathDijkstra(grid, Vec2i{ 0, 1 }, Vec2i{ 4, 1 });

    ASSERT_TRUE(breadthFirst.IsValid());
    ASSERT_TRUE(dijkstra.IsValid());

    // breadth first only counts steps, so it walks straight through the expensive cells
    EXPECT_EQ(breadthFirst.GetStepCount(), 4);
    EXPECT_EQ(breadthFirst.m_Cost, 151);

    // dijkstra pays attention to cost, so it goes the long way round for almost nothing
    EXPECT_EQ(dijkstra.GetStepCount(), 6);
    EXPECT_EQ(dijkstra.m_Cost, 6);
}

TEST(PathfindingTests, AStarMatchesDijkstrasAnswerOnWeightedGround) {
    NavGrid grid(5, 3);
    grid.SetCost(1, 1, 50);
    grid.SetCost(2, 1, 50);
    grid.SetCost(3, 1, 50);

    const Path dijkstra = Pathfinding::FindPathDijkstra(grid, Vec2i{ 0, 1 }, Vec2i{ 4, 1 });
    const Path aStar    = Pathfinding::FindPathAStar(grid, Vec2i{ 0, 1 }, Vec2i{ 4, 1 });

    ASSERT_TRUE(aStar.IsValid());

    // the guess steers the search, it must not change the answer
    EXPECT_EQ(aStar.m_Cost, dijkstra.m_Cost);
    EXPECT_EQ(aStar.GetStepCount(), dijkstra.GetStepCount());
}

TEST(PathfindingTests, AStarOpensFewerCellsThanDijkstra) {
    const NavGrid grid = OpenRoom(40, 30);

    const Path dijkstra = Pathfinding::FindPathDijkstra(grid, Vec2i{ 1, 1 }, Vec2i{ 38, 28 });
    const Path aStar    = Pathfinding::FindPathAStar(grid, Vec2i{ 1, 1 }, Vec2i{ 38, 28 });

    ASSERT_TRUE(dijkstra.IsValid());
    ASSERT_TRUE(aStar.IsValid());
    ASSERT_EQ(aStar.GetStepCount(), dijkstra.GetStepCount());

    // this is the whole point of A star, the same answer for a lot less searching.
    // measured on this map it is 278 against 1064, so half is a loose bound that still bites
    // if the heuristic ever stops being applied
    EXPECT_LT(aStar.m_Expanded * 2, dijkstra.m_Expanded)
        << "A* opened " << aStar.m_Expanded << ", Dijkstra opened " << dijkstra.m_Expanded;
}

TEST(PathfindingTests, EveryAlgorithmReportsWhatItOpened) {
    const NavGrid grid = OpenRoom(20, 12);

    for (const Algorithm algorithm : AllAlgorithms()) {
        const Path path = Pathfinding::FindPath(grid, Vec2i{ 1, 1 }, Vec2i{ 18, 10 }, algorithm);

        ASSERT_TRUE(path.IsValid()) << Pathfinding::GetAlgorithmName(algorithm);
        EXPECT_GT(path.m_Expanded, 0) << Pathfinding::GetAlgorithmName(algorithm);
    }
}

// ----------------------------
// Naming and Cycling
// ----------------------------

TEST(PathfindingTests, EveryAlgorithmHasAName) {
    for (const Algorithm algorithm : AllAlgorithms())
        EXPECT_FALSE(Pathfinding::GetAlgorithmName(algorithm).empty());
}

TEST(PathfindingTests, CyclingVisitsEveryAlgorithmAndWrapsRound) {
    Algorithm algorithm = Algorithm::BreadthFirst;

    std::vector<std::string> seen;
    for (int i = 0; i < Pathfinding::ALGORITHM_COUNT; ++i) {
        seen.push_back(Pathfinding::GetAlgorithmName(algorithm));
        algorithm = Pathfinding::GetNextAlgorithm(algorithm);
    }

    // back where it started, and it hit each one exactly once on the way
    EXPECT_EQ(algorithm, Algorithm::BreadthFirst);

    std::sort(seen.begin(), seen.end());
    EXPECT_EQ(std::unique(seen.begin(), seen.end()), seen.end());
}

// ----------------------------
// Line Of Sight
// ----------------------------

TEST(PathfindingTests, AnOpenRoomIsVisibleEndToEnd) {
    const NavGrid grid = OpenRoom(12, 8);

    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 1, 1 }, Vec2i{ 10, 6 }));
    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 10, 6 }, Vec2i{ 1, 1 }));
}

TEST(PathfindingTests, AWallInTheWayBlocksSight) {
    NavGrid grid = OpenRoom(12, 8);
    grid.SetCost(6, 4, NavGrid::BLOCKED);

    // straight through the blocked cell
    EXPECT_FALSE(Pathfinding::HasLineOfSight(grid, Vec2i{ 2, 4 }, Vec2i{ 9, 4 }));

    // a row above it, nothing in the way
    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 2, 3 }, Vec2i{ 9, 3 }));
}

TEST(PathfindingTests, SightIsSymmetric) {
    NavGrid grid = OpenRoom(12, 8);
    grid.SetCost(6, 4, NavGrid::BLOCKED);

    EXPECT_EQ(Pathfinding::HasLineOfSight(grid, Vec2i{ 2, 4 }, Vec2i{ 9, 4 }),
              Pathfinding::HasLineOfSight(grid, Vec2i{ 9, 4 }, Vec2i{ 2, 4 }));
}

TEST(PathfindingTests, YouCanAlwaysSeeYourOwnCellAndYourNeighbours) {
    NavGrid grid = OpenRoom(8, 8);
    grid.SetCost(4, 4, NavGrid::BLOCKED);

    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 3, 3 }, Vec2i{ 3, 3 }));
    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 3, 3 }, Vec2i{ 4, 3 }));

    // a wall you are standing next to is still something you can look at
    EXPECT_TRUE(Pathfinding::HasLineOfSight(grid, Vec2i{ 3, 4 }, Vec2i{ 4, 4 }));
}

TEST(PathfindingTests, AFullWallBlocksEveryLineThroughIt) {
    NavGrid grid = OpenRoom(11, 9);

    for (int y = 1; y <= 7; ++y)
        grid.SetCost(5, y, NavGrid::BLOCKED);

    for (int y = 1; y <= 7; ++y)
        EXPECT_FALSE(Pathfinding::HasLineOfSight(grid, Vec2i{ 2, 4 }, Vec2i{ 8, y }))
            << "row " << y;
}

TEST(PathfindingTests, ManhattanDistanceIgnoresWalls) {
    EXPECT_EQ(Pathfinding::ManhattanDistance(Vec2i{ 0, 0 }, Vec2i{ 3, 4 }), 7);
    EXPECT_EQ(Pathfinding::ManhattanDistance(Vec2i{ 3, 4 }, Vec2i{ 0, 0 }), 7);
    EXPECT_EQ(Pathfinding::ManhattanDistance(Vec2i{ 2, 2 }, Vec2i{ 2, 2 }), 0);
}
