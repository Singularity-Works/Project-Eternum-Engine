/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Pathfinding
* Description:
*     Route finding over a NavGrid, four way, no diagonals.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Pathfinding.h"

#include <queue>

namespace
{
    /// @brief  the four directions a step can go
    constexpr int OFFSETS_X[ 4 ] = {  0,  0, -1, 1 };
    constexpr int OFFSETS_Y[ 4 ] = { -1,  1,  0, 0 };

    /// @brief  marks a cell that nothing has reached yet
    constexpr int NO_PARENT = -1;

    /// @brief  a cell waiting on the frontier, cheapest first
    struct Candidate
    {
        int m_Index = 0;
        int m_Priority = 0;
    };

    /// @brief  orders the queue so the cheapest candidate comes off first
    struct CheapestFirst
    {
        bool operator()( Candidate const& a, Candidate const& b ) const
        {
            return a.m_Priority > b.m_Priority;
        }
    };

    /// @brief  whether a search can even start
    bool EndpointsAreUsable( NavGrid const& grid, Vec2i const& start, Vec2i const& goal )
    {
        return grid.GetCellCount() > 0
            && grid.IsWalkable( start )
            && grid.IsWalkable( goal );
    }

    /// @brief  walks the parent chain back from the goal to build the route
    /// @param  grid    the world that was searched
    /// @param  parents where each cell was reached from
    /// @param  start   where the search began
    /// @param  goal    where the search ended
    /// @return the route, start first
    std::vector< Vec2i > Reconstruct( NavGrid const& grid, std::vector< int > const& parents,
                                      Vec2i const& start, Vec2i const& goal )
    {
        std::vector< Vec2i > cells;

        const int startIndex = grid.ToIndex( start.x(), start.y() );
        int current = grid.ToIndex( goal.x(), goal.y() );

        while ( current != startIndex )
        {
            cells.push_back( grid.ToCell( current ) );

            current = parents[ current ];

            // the goal was never reached, so there is no route to hand back
            if ( current == NO_PARENT )
                return {};
        }

        cells.push_back( start );
        std::reverse( cells.begin(), cells.end() );

        return cells;
    }

    /// @brief  Dijkstra, and A star when a heuristic is allowed
    /// @param  grid            the world to search
    /// @param  start           where to search from
    /// @param  goal            where to search to
    /// @param  useHeuristic    true to steer towards the goal, which makes this A star
    /// @return the route, invalid when the goal cannot be reached
    /// @note   A star really is just this with a guess added to the priority, so both go
    ///         through here rather than being copied twice
    Path SearchWeighted( NavGrid const& grid, Vec2i const& start, Vec2i const& goal,
                         const bool useHeuristic )
    {
        Path path;

        if ( !EndpointsAreUsable( grid, start, goal ) )
            return path;

        const int cellCount = grid.GetCellCount();
        const int startIndex = grid.ToIndex( start.x(), start.y() );
        const int goalIndex  = grid.ToIndex( goal.x(), goal.y() );

        std::vector< int > costSoFar( cellCount, std::numeric_limits< int >::max() );
        std::vector< int > parents( cellCount, NO_PARENT );
        std::vector< char > settled( cellCount, 0 );

        // the guess has to stay under the real remaining cost or the answer stops being optimal
        const int cheapestStep = grid.GetCheapestStep();

        std::priority_queue< Candidate, std::vector< Candidate >, CheapestFirst > frontier;

        costSoFar[ startIndex ] = 0;
        frontier.push( { startIndex, 0 } );

        while ( !frontier.empty() )
        {
            const Candidate current = frontier.top();
            frontier.pop();

            // a cheaper way here was already dealt with, this entry is stale
            if ( settled[ current.m_Index ] )
                continue;

            settled[ current.m_Index ] = 1;
            ++path.m_Expanded;

            if ( current.m_Index == goalIndex )
                break;

            const Vec2i cell = grid.ToCell( current.m_Index );

            for ( int direction = 0; direction < 4; ++direction )
            {
                const int x = cell.x() + OFFSETS_X[ direction ];
                const int y = cell.y() + OFFSETS_Y[ direction ];

                const int stepCost = grid.GetCost( x, y );
                if ( stepCost <= NavGrid::BLOCKED )
                    continue;

                const int neighbour = grid.ToIndex( x, y );
                if ( settled[ neighbour ] )
                    continue;

                const int candidateCost = costSoFar[ current.m_Index ] + stepCost;
                if ( candidateCost >= costSoFar[ neighbour ] )
                    continue;

                costSoFar[ neighbour ] = candidateCost;
                parents[ neighbour ] = current.m_Index;

                const int guess = useHeuristic
                    ? Pathfinding::ManhattanDistance( Vec2i{ x, y }, goal ) * cheapestStep
                    : 0;

                frontier.push( { neighbour, candidateCost + guess } );
            }
        }

        if ( costSoFar[ goalIndex ] == std::numeric_limits< int >::max() )
            return path;

        path.m_Cells = Reconstruct( grid, parents, start, goal );
        path.m_Cost = costSoFar[ goalIndex ];

        return path;
    }
}

namespace Pathfinding
{
    Path FindPathBreadthFirst( NavGrid const& grid, Vec2i const& start, Vec2i const& goal )
    {
        Path path;

        if ( !EndpointsAreUsable( grid, start, goal ) )
            return path;

        const int cellCount = grid.GetCellCount();
        const int startIndex = grid.ToIndex( start.x(), start.y() );
        const int goalIndex  = grid.ToIndex( goal.x(), goal.y() );

        std::vector< int > parents( cellCount, NO_PARENT );
        std::vector< char > seen( cellCount, 0 );

        std::queue< int > frontier;
        seen[ startIndex ] = 1;
        frontier.push( startIndex );

        bool reached = ( startIndex == goalIndex );

        while ( !frontier.empty() && !reached )
        {
            const int current = frontier.front();
            frontier.pop();
            ++path.m_Expanded;

            const Vec2i cell = grid.ToCell( current );

            for ( int direction = 0; direction < 4 && !reached; ++direction )
            {
                const int x = cell.x() + OFFSETS_X[ direction ];
                const int y = cell.y() + OFFSETS_Y[ direction ];

                if ( !grid.IsWalkable( x, y ) )
                    continue;

                const int neighbour = grid.ToIndex( x, y );
                if ( seen[ neighbour ] )
                    continue;

                seen[ neighbour ] = 1;
                parents[ neighbour ] = current;

                // every step weighs the same here, so the first arrival is the shortest
                if ( neighbour == goalIndex )
                    reached = true;
                else
                    frontier.push( neighbour );
            }
        }

        if ( !seen[ goalIndex ] )
            return path;

        path.m_Cells = Reconstruct( grid, parents, start, goal );

        // cost still reports what the route would charge, even though the search ignored it
        for ( std::size_t i = 1; i < path.m_Cells.size(); ++i )
            path.m_Cost += grid.GetCost( path.m_Cells[ i ].x(), path.m_Cells[ i ].y() );

        return path;
    }

    Path FindPathDijkstra( NavGrid const& grid, Vec2i const& start, Vec2i const& goal )
    {
        return SearchWeighted( grid, start, goal, false );
    }

    Path FindPathAStar( NavGrid const& grid, Vec2i const& start, Vec2i const& goal )
    {
        return SearchWeighted( grid, start, goal, true );
    }

    Path FindPath( NavGrid const& grid, Vec2i const& start, Vec2i const& goal, const Algorithm algorithm )
    {
        switch ( algorithm )
        {
            case Algorithm::BreadthFirst: return FindPathBreadthFirst( grid, start, goal );
            case Algorithm::Dijkstra:     return FindPathDijkstra( grid, start, goal );
            case Algorithm::AStar:        return FindPathAStar( grid, start, goal );
        }

        return {};
    }

    std::string GetAlgorithmName( const Algorithm algorithm )
    {
        switch ( algorithm )
        {
            case Algorithm::BreadthFirst: return "BFS";
            case Algorithm::Dijkstra:     return "Dijkstra";
            case Algorithm::AStar:        return "A*";
        }

        return "Unknown";
    }

    Algorithm GetNextAlgorithm( const Algorithm algorithm )
    {
        switch ( algorithm )
        {
            case Algorithm::BreadthFirst: return Algorithm::Dijkstra;
            case Algorithm::Dijkstra:     return Algorithm::AStar;
            case Algorithm::AStar:        return Algorithm::BreadthFirst;
        }

        return Algorithm::AStar;
    }

    int ManhattanDistance( Vec2i const& from, Vec2i const& to )
    {
        return std::abs( from.x() - to.x() ) + std::abs( from.y() - to.y() );
    }

    bool HasLineOfSight( NavGrid const& grid, Vec2i const& from, Vec2i const& to )
    {
        // Bresenham, walking the straight line one cell at a time
        int x = from.x();
        int y = from.y();

        const int targetX = to.x();
        const int targetY = to.y();

        const int deltaX = std::abs( targetX - x );
        const int deltaY = -std::abs( targetY - y );

        const int stepX = ( x < targetX ) ? 1 : -1;
        const int stepY = ( y < targetY ) ? 1 : -1;

        int error = deltaX + deltaY;

        while ( x != targetX || y != targetY )
        {
            const int doubled = error * 2;

            if ( doubled >= deltaY )
            {
                error += deltaY;
                x += stepX;
            }

            if ( doubled <= deltaX )
            {
                error += deltaX;
                y += stepY;
            }

            // arriving at the far end means nothing blocked the way
            if ( x == targetX && y == targetY )
                return true;

            if ( !grid.IsWalkable( x, y ) )
                return false;
        }

        return true;
    }
}
