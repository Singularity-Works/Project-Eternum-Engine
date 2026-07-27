/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: RoomGenerator
* Description:
*     Scattered rooms.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "RoomGenerator.h"
#include "CorridorGraph.h"

DungeonLayout RoomGenerator::Generate( const int width, const int height, Random& rng ) const
{
    DungeonLayout layout;
    layout.m_Grid = GridSystem::Grid( width, height, Tiles::WALL );

    // every room needs its own space plus the one cell border around the map
    if ( width < MIN_ROOM_SIZE + 2 || height < MIN_ROOM_SIZE + 2 )
        return layout;

    for ( int attempt = 0;
          attempt < MAX_ATTEMPTS && static_cast< int >( layout.m_Rooms.size() ) < TARGET_ROOMS;
          ++attempt )
    {
        const int roomWidth  = rng.Range( MIN_ROOM_SIZE, std::min( MAX_ROOM_SIZE, width  - 2 ) );
        const int roomHeight = rng.Range( MIN_ROOM_SIZE, std::min( MAX_ROOM_SIZE, height - 2 ) );

        const int x = rng.Range( 1, width  - roomWidth  - 1 );
        const int y = rng.Range( 1, height - roomHeight - 1 );

        const Room candidate( x, y, roomWidth, roomHeight );

        // a padding of one keeps a wall between rooms instead of merging them
        const bool collides = std::any_of( layout.m_Rooms.begin(), layout.m_Rooms.end(),
            [ &candidate ]( Room const& room ) { return room.Intersects( candidate, 1 ); } );

        if ( !collides )
            layout.m_Rooms.push_back( candidate );
    }

    for ( Room const& room : layout.m_Rooms )
        for ( int y = room.Top(); y <= room.Bottom(); ++y )
            for ( int x = room.Left(); x <= room.Right(); ++x )
                layout.m_Grid.SetCell( x, y, Tiles::FLOOR );

    const std::vector< CorridorGraph::Edge > tree =
        CorridorGraph::BuildMinimumSpanningTree( layout.m_Rooms );

    const std::vector< CorridorGraph::Edge > extras =
        CorridorGraph::PickExtraEdges( layout.m_Rooms, tree, EXTRA_CORRIDORS, rng );

    CorridorGraph::CarveAll( layout.m_Grid, layout.m_Rooms, tree, rng );
    CorridorGraph::CarveAll( layout.m_Grid, layout.m_Rooms, extras, rng );

    return layout;
}
