/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CorridorGraph
* Description:
*     Decides which rooms get joined to which. Builds a minimum spanning tree over the room
*     centres, which is the cheapest set of corridors that still reaches every room, then
*     adds a few extra links so the dungeon has loops instead of being a dead end tree.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef CORRIDORGRAPH_H
#define CORRIDORGRAPH_H

#include <pch.h>
#include <Core/Random/Random.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>

namespace CorridorGraph
{
    /// @brief  A corridor between two rooms, named by their index.
    struct Edge
    {
        int m_A = 0;
        int m_B = 0;
        int m_Cost = 0;

        Edge() = default;
        Edge( const int a, const int b, const int cost ) : m_A( a ), m_B( b ), m_Cost( cost ) {}

        /// @brief  whether this Edge joins the same pair as another, in either order
        /// @param  other   the Edge to compare against
        /// @return whether the two Edges join the same rooms
        bool SamePair( Edge const& other ) const
        {
            return ( m_A == other.m_A && m_B == other.m_B )
                || ( m_A == other.m_B && m_B == other.m_A );
        }
    };

    /// @brief  the cheapest set of corridors that still reaches every room
    /// @param  rooms   the rooms to join up
    /// @return one Edge short of the room count, or empty for fewer than two rooms
    /// @note   Prim's algorithm over the complete graph, cost is the walking distance
    ///         between room centres
    std::vector< Edge > BuildMinimumSpanningTree( std::vector< Room > const& rooms );

    /// @brief  adds a few corridors the tree did not need, so the map has loops
    /// @param  rooms       the rooms to join up
    /// @param  tree        the corridors already chosen
    /// @param  extraCount  how many extra corridors to try to add
    /// @param  rng         the random source to draw from
    /// @return the extra corridors, none of which repeat a pair already in the tree
    std::vector< Edge > PickExtraEdges( std::vector< Room > const& rooms,
                                        std::vector< Edge > const& tree,
                                        int extraCount,
                                        Random& rng );

    /// @brief  cuts an L shaped corridor between two cells
    /// @param  grid    the map to cut into
    /// @param  from    the cell to start at
    /// @param  to      the cell to finish at
    /// @param  rng     the random source, decides whether the corner turns early or late
    /// @param  floor   the character to cut with
    void CarveCorridor( GridSystem::Grid& grid, Vec2i const& from, Vec2i const& to, Random& rng,
                        char floor = Tiles::FLOOR );

    /// @brief  cuts every corridor in a set of edges
    /// @param  grid    the map to cut into
    /// @param  rooms   the rooms the edges refer to
    /// @param  edges   the corridors to cut
    /// @param  rng     the random source to draw from
    /// @param  floor   the character to cut with
    void CarveAll( GridSystem::Grid& grid, std::vector< Room > const& rooms,
                   std::vector< Edge > const& edges, Random& rng, char floor = Tiles::FLOOR );
}

#endif //CORRIDORGRAPH_H
