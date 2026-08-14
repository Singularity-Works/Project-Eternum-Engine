/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: NavGrid
* Description:
*     What pathfinding needs to know about the world, and nothing else.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "NavGrid.h"

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NavGrid::NavGrid( const int width, const int height, const int fillCost )
{
    if ( width <= 0 || height <= 0 )
        return;

    m_Width = width;
    m_Height = height;
    m_Costs.assign( static_cast< std::size_t >( width ) * height, std::max( fillCost, BLOCKED ) );

    refreshCheapestStep();
}

NavGrid NavGrid::FromGrid( GridSystem::Grid const& grid, std::string const& blockingTiles )
{
    NavGrid navigation( grid.m_Dimension.m_Width, grid.m_Dimension.m_Height );

    for ( int y = 0; y < navigation.m_Height; ++y )
    {
        for ( int x = 0; x < navigation.m_Width; ++x )
        {
            if ( blockingTiles.find( grid.GetCell( x, y ) ) != std::string::npos )
                navigation.m_Costs[ navigation.ToIndex( x, y ) ] = BLOCKED;
        }
    }

    navigation.refreshCheapestStep();
    return navigation;
}

NavGrid NavGrid::FromActiveMap()
{
    GridSystem* grid = GridSystem::GetInstance().get();

    const int width = grid->GetWidth();
    const int height = grid->GetHeight();

    NavGrid navigation( width, height );

    for ( int y = 0; y < height; ++y )
        for ( int x = 0; x < width; ++x )
            if ( !grid->IsWalkable( x, y ) )
                navigation.m_Costs[ navigation.ToIndex( x, y ) ] = BLOCKED;

    navigation.refreshCheapestStep();
    return navigation;
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

bool NavGrid::Contains( const int x, const int y ) const
{
    return x >= 0 && x < m_Width && y >= 0 && y < m_Height;
}

int NavGrid::GetCost( const int x, const int y ) const
{
    // off the map is the same as a wall, callers never have to bounds check
    if ( !Contains( x, y ) )
        return BLOCKED;

    return m_Costs[ ToIndex( x, y ) ];
}

void NavGrid::SetCost( const int x, const int y, const int cost )
{
    if ( !Contains( x, y ) )
        return;

    m_Costs[ ToIndex( x, y ) ] = std::max( cost, BLOCKED );
    refreshCheapestStep();
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

void NavGrid::refreshCheapestStep()
{
    int cheapest = 0;

    for ( const int cost : m_Costs )
    {
        if ( cost <= BLOCKED )
            continue;

        if ( cheapest == 0 || cost < cheapest )
            cheapest = cost;
    }

    // with nothing walkable the value does not matter, keep it sane rather than zero
    m_CheapestStep = ( cheapest > 0 ) ? cheapest : NORMAL_COST;
}
