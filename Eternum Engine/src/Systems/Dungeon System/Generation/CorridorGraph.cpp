/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CorridorGraph
* Description:
*     Decides which rooms get joined to which.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "CorridorGraph.h"

#include <limits>

namespace
{
    /// @brief  walking distance between two cells, no diagonals
    int ManhattanDistance( Vec2i const& a, Vec2i const& b )
    {
        return std::abs( a.x() - b.x() ) + std::abs( a.y() - b.y() );
    }

    /// @brief  cuts a straight run along a row, both ends included
    void CarveHorizontal( GridSystem::Grid& grid, const int y, const int fromX, const int toX, const char floor )
    {
        const int step = ( toX >= fromX ) ? 1 : -1;
        for ( int x = fromX; ; x += step )
        {
            grid.SetCell( x, y, floor );
            if ( x == toX )
                break;
        }
    }

    /// @brief  cuts a straight run down a column, both ends included
    void CarveVertical( GridSystem::Grid& grid, const int x, const int fromY, const int toY, const char floor )
    {
        const int step = ( toY >= fromY ) ? 1 : -1;
        for ( int y = fromY; ; y += step )
        {
            grid.SetCell( x, y, floor );
            if ( y == toY )
                break;
        }
    }
}

namespace CorridorGraph
{
    std::vector< Edge > BuildMinimumSpanningTree( std::vector< Room > const& rooms )
    {
        std::vector< Edge > tree;

        const int roomCount = static_cast< int >( rooms.size() );
        if ( roomCount < 2 )
            return tree;

        // Prim's algorithm, grow one connected blob until it swallows every room
        std::vector< bool > inTree( roomCount, false );
        std::vector< int > cheapestCost( roomCount, std::numeric_limits< int >::max() );
        std::vector< int > cheapestFrom( roomCount, -1 );

        inTree[ 0 ] = true;
        for ( int i = 1; i < roomCount; ++i )
        {
            cheapestCost[ i ] = ManhattanDistance( rooms[ 0 ].Center(), rooms[ i ].Center() );
            cheapestFrom[ i ] = 0;
        }

        for ( int added = 1; added < roomCount; ++added )
        {
            // find the room outside the tree that is cheapest to reach
            int next = -1;
            for ( int i = 0; i < roomCount; ++i )
            {
                if ( inTree[ i ] )
                    continue;

                if ( next == -1 || cheapestCost[ i ] < cheapestCost[ next ] )
                    next = i;
            }

            if ( next == -1 )
                break;

            inTree[ next ] = true;
            tree.emplace_back( cheapestFrom[ next ], next, cheapestCost[ next ] );

            // pulling a room in may make its neighbours cheaper to reach
            for ( int i = 0; i < roomCount; ++i )
            {
                if ( inTree[ i ] )
                    continue;

                const int cost = ManhattanDistance( rooms[ next ].Center(), rooms[ i ].Center() );
                if ( cost < cheapestCost[ i ] )
                {
                    cheapestCost[ i ] = cost;
                    cheapestFrom[ i ] = next;
                }
            }
        }

        return tree;
    }

    std::vector< Edge > PickExtraEdges( std::vector< Room > const& rooms,
                                        std::vector< Edge > const& tree,
                                        const int extraCount,
                                        Random& rng )
    {
        std::vector< Edge > extras;

        const int roomCount = static_cast< int >( rooms.size() );
        if ( roomCount < 3 || extraCount <= 0 )
            return extras;

        // every pair that is not already a corridor, cheapest first
        std::vector< Edge > candidates;
        for ( int a = 0; a < roomCount; ++a )
        {
            for ( int b = a + 1; b < roomCount; ++b )
            {
                const Edge candidate( a, b, ManhattanDistance( rooms[ a ].Center(), rooms[ b ].Center() ) );

                const bool alreadyJoined = std::any_of( tree.begin(), tree.end(),
                    [ &candidate ]( Edge const& edge ) { return edge.SamePair( candidate ); } );

                if ( !alreadyJoined )
                    candidates.push_back( candidate );
            }
        }

        if ( candidates.empty() )
            return extras;

        std::sort( candidates.begin(), candidates.end(),
            []( Edge const& a, Edge const& b ) { return a.m_Cost < b.m_Cost; } );

        // prefer short links so the loops feel local, but keep it from being identical every run
        const std::size_t pool = std::min( candidates.size(), static_cast< std::size_t >( extraCount ) * 3 );

        for ( int i = 0; i < extraCount && !candidates.empty(); ++i )
        {
            const std::size_t limit = std::min( pool, candidates.size() );
            const std::size_t choice = rng.Index( limit );

            extras.push_back( candidates[ choice ] );
            candidates.erase( candidates.begin() + static_cast< std::ptrdiff_t >( choice ) );
        }

        return extras;
    }

    void CarveCorridor( GridSystem::Grid& grid, Vec2i const& from, Vec2i const& to, Random& rng, const char floor )
    {
        // turning the corner at a random end stops every corridor looking the same
        if ( rng.Chance( 50 ) )
        {
            CarveHorizontal( grid, from.y(), from.x(), to.x(), floor );
            CarveVertical( grid, to.x(), from.y(), to.y(), floor );
        }
        else
        {
            CarveVertical( grid, from.x(), from.y(), to.y(), floor );
            CarveHorizontal( grid, to.y(), from.x(), to.x(), floor );
        }
    }

    void CarveAll( GridSystem::Grid& grid, std::vector< Room > const& rooms,
                   std::vector< Edge > const& edges, Random& rng, const char floor )
    {
        for ( Edge const& edge : edges )
        {
            if ( edge.m_A < 0 || edge.m_B < 0 )
                continue;

            if ( edge.m_A >= static_cast< int >( rooms.size() ) || edge.m_B >= static_cast< int >( rooms.size() ) )
                continue;

            CarveCorridor( grid, rooms[ edge.m_A ].Center(), rooms[ edge.m_B ].Center(), rng, floor );
        }
    }
}
