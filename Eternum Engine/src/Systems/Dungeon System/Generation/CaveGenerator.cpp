/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CaveGenerator
* Description:
*     Cellular automata.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "CaveGenerator.h"
#include "GridRegions.h"

namespace
{
    /// @brief  counts the wall cells touching a cell, including diagonals
    /// @param  grid    the map to look at
    /// @param  cellX   the column to look around
    /// @param  cellY   the row to look around
    /// @return how many of the eight neighbours are wall, counting off map as wall
    int CountWallNeighbours( GridSystem::Grid const& grid, const int cellX, const int cellY )
    {
        int count = 0;

        for ( int offsetY = -1; offsetY <= 1; ++offsetY )
        {
            for ( int offsetX = -1; offsetX <= 1; ++offsetX )
            {
                if ( offsetX == 0 && offsetY == 0 )
                    continue;

                const int x = cellX + offsetX;
                const int y = cellY + offsetY;

                // off the map counts as wall, which keeps the edges solid
                if ( x < 0 || x >= grid.m_Dimension.m_Width || y < 0 || y >= grid.m_Dimension.m_Height )
                {
                    ++count;
                    continue;
                }

                if ( grid.GetCell( x, y ) == Tiles::WALL )
                    ++count;
            }
        }

        return count;
    }
}

DungeonLayout CaveGenerator::Generate( const int width, const int height, Random& rng ) const
{
    DungeonLayout layout;
    layout.m_Grid = GridSystem::Grid( width, height, Tiles::WALL );

    if ( width < 3 || height < 3 )
        return layout;

    // start from noise, the border is left solid
    for ( int y = 1; y < height - 1; ++y )
        for ( int x = 1; x < width - 1; ++x )
            layout.m_Grid.SetCell( x, y, rng.Chance( FILL_PERCENT ) ? Tiles::WALL : Tiles::FLOOR );

    // each pass makes a cell agree with its neighbours, which grows blobs and eats specks
    for ( int step = 0; step < SMOOTH_STEPS; ++step )
    {
        GridSystem::Grid next = layout.m_Grid;

        for ( int y = 1; y < height - 1; ++y )
        {
            for ( int x = 1; x < width - 1; ++x )
            {
                const int walls = CountWallNeighbours( layout.m_Grid, x, y );

                if ( walls > WALL_THRESHOLD )
                    next.SetCell( x, y, Tiles::WALL );
                else if ( walls < WALL_THRESHOLD )
                    next.SetCell( x, y, Tiles::FLOOR );
            }
        }

        layout.m_Grid = next;
    }

    // smoothing leaves sealed off pockets, wall them in so every floor cell is reachable
    GridRegions::KeepLargestRegion( layout.m_Grid );

    // a cave has no rooms, callers fall back to picking any open cell
    return layout;
}
