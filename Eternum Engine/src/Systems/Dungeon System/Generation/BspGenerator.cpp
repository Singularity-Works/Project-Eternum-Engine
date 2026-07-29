/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: BspGenerator
* Description:
*     Binary space partitioning.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "BspGenerator.h"
#include "CorridorGraph.h"

namespace
{
    /// @brief  A rectangular piece of the map that a room can be placed inside.
    struct Partition
    {
        int m_X = 0;
        int m_Y = 0;
        int m_Width = 0;
        int m_Height = 0;
    };

    /// @brief  cuts a piece of the map in half until it is small enough to hold one room
    /// @param  area    the piece to cut
    /// @param  depth   how many cuts have already been made above this one
    /// @param  rng     the random source to draw from
    /// @param  leaves  where finished pieces are collected
    void SplitPartition( Partition const& area, const int depth, Random& rng, std::vector< Partition >& leaves )
    {
        const int minimum = BspGenerator::MIN_PARTITION_SIZE;

        const bool canSplitVertically   = area.m_Width  >= minimum * 2;
        const bool canSplitHorizontally = area.m_Height >= minimum * 2;

        // out of room or out of depth, this piece gets a room of its own
        if ( depth >= BspGenerator::MAX_DEPTH || ( !canSplitVertically && !canSplitHorizontally ) )
        {
            leaves.push_back( area );
            return;
        }

        // cut across the longer side so the pieces stay roughly square
        bool splitVertically;
        if ( canSplitVertically && canSplitHorizontally )
            splitVertically = ( area.m_Width >= area.m_Height );
        else
            splitVertically = canSplitVertically;

        const int extent = splitVertically ? area.m_Width : area.m_Height;
        const int cut = rng.Range( minimum, extent - minimum );

        if ( splitVertically )
        {
            SplitPartition( { area.m_X,       area.m_Y, cut,                 area.m_Height }, depth + 1, rng, leaves );
            SplitPartition( { area.m_X + cut, area.m_Y, area.m_Width - cut,  area.m_Height }, depth + 1, rng, leaves );
        }
        else
        {
            SplitPartition( { area.m_X, area.m_Y,       area.m_Width, cut                  }, depth + 1, rng, leaves );
            SplitPartition( { area.m_X, area.m_Y + cut, area.m_Width, area.m_Height - cut  }, depth + 1, rng, leaves );
        }
    }

    /// @brief  places one room inside a piece of the map, leaving a wall margin
    /// @param  area    the piece to place inside
    /// @param  rng     the random source to draw from
    /// @param  room    the room that was placed
    /// @return whether the piece was big enough to hold a room
    bool PlaceRoomIn( Partition const& area, Random& rng, Room& room )
    {
        // one cell of wall on every side so neighbouring rooms never touch
        const int maxWidth  = area.m_Width  - 2;
        const int maxHeight = area.m_Height - 2;

        if ( maxWidth < BspGenerator::MIN_ROOM_SIZE || maxHeight < BspGenerator::MIN_ROOM_SIZE )
            return false;

        const int roomWidth  = rng.Range( BspGenerator::MIN_ROOM_SIZE, maxWidth );
        const int roomHeight = rng.Range( BspGenerator::MIN_ROOM_SIZE, maxHeight );

        const int x = area.m_X + rng.Range( 1, area.m_Width  - roomWidth  - 1 );
        const int y = area.m_Y + rng.Range( 1, area.m_Height - roomHeight - 1 );

        room = Room( x, y, roomWidth, roomHeight );
        return true;
    }
}

DungeonLayout BspGenerator::Generate( const int width, const int height, Random& rng ) const
{
    DungeonLayout layout;
    layout.m_Grid = GridSystem::Grid( width, height, Tiles::WALL );

    if ( width < MIN_PARTITION_SIZE || height < MIN_PARTITION_SIZE )
        return layout;

    // the whole map is the first piece, minus a one cell border that stays solid
    std::vector< Partition > leaves;
    SplitPartition( { 1, 1, width - 2, height - 2 }, 0, rng, leaves );

    for ( Partition const& leaf : leaves )
    {
        Room room;
        if ( PlaceRoomIn( leaf, rng, room ) )
            layout.m_Rooms.push_back( room );
    }

    for ( Room const& room : layout.m_Rooms )
        for ( int y = room.Top(); y <= room.Bottom(); ++y )
            for ( int x = room.Left(); x <= room.Right(); ++x )
                layout.m_Grid.SetCell( x, y, Tiles::FLOOR );

    // the spanning tree reaches every room, the extras turn dead ends into loops
    const std::vector< CorridorGraph::Edge > tree =
        CorridorGraph::BuildMinimumSpanningTree( layout.m_Rooms );

    const std::vector< CorridorGraph::Edge > extras =
        CorridorGraph::PickExtraEdges( layout.m_Rooms, tree, EXTRA_CORRIDORS, rng );

    CorridorGraph::CarveAll( layout.m_Grid, layout.m_Rooms, tree, rng );
    CorridorGraph::CarveAll( layout.m_Grid, layout.m_Rooms, extras, rng );

    return layout;
}
