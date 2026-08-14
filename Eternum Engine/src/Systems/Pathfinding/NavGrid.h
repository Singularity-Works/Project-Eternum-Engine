/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: NavGrid
* Description:
*     What pathfinding needs to know about the world, and nothing else. A cost per cell,
*     where zero means the cell cannot be entered at all. Keeping this separate from the
*     GridSystem means the algorithms can be tested against a hand built map with no
*     engine running.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef NAVGRID_H
#define NAVGRID_H

#include <pch.h>
#include <Systems/Grid System/GridSystem.h>

class NavGrid
{

public:

    /// @brief  a cell that cannot be entered
    static constexpr int BLOCKED = 0;

    /// @brief  what an ordinary cell costs to walk into
    static constexpr int NORMAL_COST = 1;

    //-----------------------------------------------------------------------------
    // Constructors
    //-----------------------------------------------------------------------------

    NavGrid() = default;

    /// @brief  constructor
    /// @param  width       how many cells across
    /// @param  height      how many cells down
    /// @param  fillCost    what every cell costs to begin with
    NavGrid( int width, int height, int fillCost = NORMAL_COST );

    /// @brief  builds a NavGrid from a map
    /// @param  grid            the map to read
    /// @param  blockingTiles   the characters that cannot be walked through
    /// @return a NavGrid matching the map
    static NavGrid FromGrid( GridSystem::Grid const& grid, std::string const& blockingTiles );

    /// @brief  builds a NavGrid from whatever map the GridSystem currently has loaded
    /// @return a NavGrid matching the active map, empty when there is no active map
    static NavGrid FromActiveMap();

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  whether a cell is on the map at all
    /// @param  x   the column to test
    /// @param  y   the row to test
    /// @return whether the cell exists
    bool Contains( int x, int y ) const;

    /// @brief  what it costs to walk into a cell
    /// @param  x   the column to read
    /// @param  y   the row to read
    /// @return the cost, or BLOCKED for a wall or anything off the map
    int GetCost( int x, int y ) const;

    /// @brief  sets what it costs to walk into a cell
    /// @param  x       the column to write
    /// @param  y       the row to write
    /// @param  cost    the new cost, BLOCKED to wall it off
    void SetCost( int x, int y, int cost );

    /// @brief  whether a cell can be walked into
    /// @param  x   the column to test
    /// @param  y   the row to test
    /// @return whether the cell can be entered
    bool IsWalkable( int x, int y ) const { return GetCost( x, y ) > BLOCKED; }

    /// @brief  whether a cell can be walked into
    /// @param  cell    the cell to test
    /// @return whether the cell can be entered
    bool IsWalkable( Vec2i const& cell ) const { return IsWalkable( cell.x(), cell.y() ); }

    /// @brief  the cheapest any single step can be, used to keep the A star guess honest
    /// @return the lowest cost of any walkable cell, or NORMAL_COST when there are none
    int GetCheapestStep() const { return m_CheapestStep; }

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    /// @brief  how many cells this NavGrid holds
    /// @return width times height
    int GetCellCount() const { return m_Width * m_Height; }

    /// @brief  turns a cell into an index into the cost array
    /// @param  x   the column
    /// @param  y   the row
    /// @return the index, only valid when Contains is true
    int ToIndex( const int x, const int y ) const { return y * m_Width + x; }

    /// @brief  turns an index back into a cell
    /// @param  index   the index to convert
    /// @return the cell that index refers to
    Vec2i ToCell( const int index ) const { return Vec2i{ index % m_Width, index / m_Width }; }

private:

    /// @brief  works out the cheapest walkable cell, called after the costs change
    void refreshCheapestStep();

    int m_Width = 0;
    int m_Height = 0;

    /// @brief  what each cell costs to enter, BLOCKED for walls
    std::vector< int > m_Costs;

    /// @brief  the lowest cost of any walkable cell
    int m_CheapestStep = NORMAL_COST;

};

#endif //NAVGRID_H
