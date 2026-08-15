/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Pathfinding
* Description:
*     Route finding over a NavGrid, four way, no diagonals.
*
*     Three algorithms, which are really two. Breadth first ignores cost and finds the
*     fewest steps. Dijkstra respects cost and finds the cheapest route. A star is
*     Dijkstra with a guess at the distance still to go, which steers it towards the goal
*     and makes it expand far fewer cells for the same answer.
*
*     Every result carries how many cells the search expanded, which is the honest way to
*     compare them.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <pch.h>
#include <Systems/Pathfinding/NavGrid.h>

//-----------------------------------------------------------------------------
// Path
//-----------------------------------------------------------------------------

/// @brief  A route from one cell to another.
struct Path
{
    /// @brief  every cell on the route, start first, goal last
    std::vector< Vec2i > m_Cells;

    /// @brief  what walking the route costs, not counting standing on the start
    int m_Cost = 0;

    /// @brief  how many cells the search pulled off its frontier to get here
    /// @note   this is what separates A star from Dijkstra, the answer is the same but
    ///         the work to find it is not
    int m_Expanded = 0;

    /// @brief  whether a route was found at all
    /// @return whether this Path leads anywhere
    bool IsValid() const { return !m_Cells.empty(); }

    /// @brief  how many steps the route takes
    /// @return the number of moves, which is one less than the number of cells
    int GetStepCount() const
    {
        return m_Cells.empty() ? 0 : static_cast< int >( m_Cells.size() ) - 1;
    }

    /// @brief  the first cell to move to
    /// @return the cell after the start, or the start itself when already there
    Vec2i GetNextStep() const
    {
        if ( m_Cells.empty() )
            return Vec2i{ -1, -1 };

        return ( m_Cells.size() > 1 ) ? m_Cells[ 1 ] : m_Cells[ 0 ];
    }
};

//-----------------------------------------------------------------------------
// Pathfinding
//-----------------------------------------------------------------------------

namespace Pathfinding
{
    /// @brief  which algorithm to run
    enum class Algorithm
    {
        BreadthFirst,
        Dijkstra,
        AStar
    };

    /// @brief  how many algorithms there are, for cycling through them
    constexpr int ALGORITHM_COUNT = 3;

    /// @brief  fewest steps, ignoring what each cell costs
    /// @param  grid    the world to search
    /// @param  start   where to search from
    /// @param  goal    where to search to
    /// @return the route, invalid when the goal cannot be reached
    Path FindPathBreadthFirst( NavGrid const& grid, Vec2i const& start, Vec2i const& goal );

    /// @brief  cheapest route, respecting what each cell costs
    /// @param  grid    the world to search
    /// @param  start   where to search from
    /// @param  goal    where to search to
    /// @return the route, invalid when the goal cannot be reached
    Path FindPathDijkstra( NavGrid const& grid, Vec2i const& start, Vec2i const& goal );

    /// @brief  cheapest route, steered towards the goal so far fewer cells are opened
    /// @param  grid    the world to search
    /// @param  start   where to search from
    /// @param  goal    where to search to
    /// @return the route, invalid when the goal cannot be reached
    Path FindPathAStar( NavGrid const& grid, Vec2i const& start, Vec2i const& goal );

    /// @brief  runs whichever algorithm is asked for
    /// @param  grid        the world to search
    /// @param  start       where to search from
    /// @param  goal        where to search to
    /// @param  algorithm   which algorithm to run
    /// @return the route, invalid when the goal cannot be reached
    Path FindPath( NavGrid const& grid, Vec2i const& start, Vec2i const& goal, Algorithm algorithm );

    /// @brief  the readable name of an algorithm
    /// @param  algorithm   the algorithm to name
    /// @return its name
    std::string GetAlgorithmName( Algorithm algorithm );

    /// @brief  the algorithm after this one, wrapping round at the end
    /// @param  algorithm   the algorithm to step past
    /// @return the next algorithm
    Algorithm GetNextAlgorithm( Algorithm algorithm );

    /// @brief  walking distance between two cells with no diagonals
    /// @param  from    the first cell
    /// @param  to      the second cell
    /// @return the number of steps ignoring walls
    int ManhattanDistance( Vec2i const& from, Vec2i const& to );

    /// @brief  whether one cell can see another in a straight line
    /// @param  grid    the world to look through
    /// @param  from    the cell doing the looking
    /// @param  to      the cell being looked at
    /// @return false when a wall sits anywhere between them
    /// @note   the two ends are not tested, you can always see the floor you stand on and
    ///         you can see a wall you are looking straight at
    bool HasLineOfSight( NavGrid const& grid, Vec2i const& from, Vec2i const& to );
}

#endif //PATHFINDING_H
