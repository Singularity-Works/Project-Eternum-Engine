/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GridRegions
* Description:
*     Flood fill over a map.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "GridRegions.h"

#include <queue>

namespace GridRegions
{
    std::vector< std::vector< Vec2i > > FindRegions( GridSystem::Grid const& grid, const char floor )
    {
        const int width  = grid.m_Dimension.m_Width;
        const int height = grid.m_Dimension.m_Height;

        std::vector< std::vector< Vec2i > > regions;
        if ( width <= 0 || height <= 0 )
            return regions;

        std::vector< bool > visited( static_cast< std::size_t >( width ) * height, false );

        for ( int startY = 0; startY < height; ++startY )
        {
            for ( int startX = 0; startX < width; ++startX )
            {
                const std::size_t startIndex = static_cast< std::size_t >( startY ) * width + startX;

                if ( visited[ startIndex ] || grid.GetCell( startX, startY ) != floor )
                    continue;

                // breadth first from this cell until the region runs out
                std::vector< Vec2i > region;
                std::queue< Vec2i > frontier;

                visited[ startIndex ] = true;
                frontier.push( Vec2i{ startX, startY } );

                while ( !frontier.empty() )
                {
                    const Vec2i cell = frontier.front();
                    frontier.pop();
                    region.push_back( cell );

                    constexpr int offsetsX[ 4 ] = {  0,  0, -1, 1 };
                    constexpr int offsetsY[ 4 ] = { -1,  1,  0, 0 };

                    for ( int i = 0; i < 4; ++i )
                    {
                        const int x = cell.x() + offsetsX[ i ];
                        const int y = cell.y() + offsetsY[ i ];

                        if ( x < 0 || x >= width || y < 0 || y >= height )
                            continue;

                        const std::size_t index = static_cast< std::size_t >( y ) * width + x;

                        if ( visited[ index ] || grid.GetCell( x, y ) != floor )
                            continue;

                        visited[ index ] = true;
                        frontier.push( Vec2i{ x, y } );
                    }
                }

                regions.push_back( std::move( region ) );
            }
        }

        // largest first so callers can just take the front
        std::sort( regions.begin(), regions.end(),
            []( std::vector< Vec2i > const& a, std::vector< Vec2i > const& b ) { return a.size() > b.size(); } );

        return regions;
    }

    bool IsFullyConnected( GridSystem::Grid const& grid, const char floor )
    {
        return FindRegions( grid, floor ).size() <= 1;
    }

    int CountFloor( GridSystem::Grid const& grid, const char floor )
    {
        int count = 0;
        for ( int y = 0; y < grid.m_Dimension.m_Height; ++y )
            for ( int x = 0; x < grid.m_Dimension.m_Width; ++x )
                if ( grid.GetCell( x, y ) == floor )
                    ++count;

        return count;
    }

    int KeepLargestRegion( GridSystem::Grid& grid, const char floor, const char wall )
    {
        const std::vector< std::vector< Vec2i > > regions = FindRegions( grid, floor );
        if ( regions.size() <= 1 )
            return 0;

        int filled = 0;

        // regions are sorted largest first, so everything after the front goes
        for ( std::size_t i = 1; i < regions.size(); ++i )
        {
            for ( Vec2i const& cell : regions[ i ] )
            {
                grid.SetCell( cell.x(), cell.y(), wall );
                ++filled;
            }
        }

        return filled;
    }
}
