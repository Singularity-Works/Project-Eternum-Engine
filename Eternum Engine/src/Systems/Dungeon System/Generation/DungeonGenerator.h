/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: DungeonGenerator
* Description:
*     The shape every dungeon generator has in common. A generator is handed a size and a
*     random source and hands back a finished map plus the rooms it placed, so the
*     DungeonSystem can swap one algorithm for another without knowing how it works.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef DUNGEONGENERATOR_H
#define DUNGEONGENERATOR_H

#include <pch.h>
#include <Core/Random/Random.h>
#include <Systems/Grid System/GridSystem.h>

//-----------------------------------------------------------------------------
// Tiles
//-----------------------------------------------------------------------------

namespace Tiles
{
    constexpr char FLOOR = '.';
    constexpr char WALL  = '#';
}

//-----------------------------------------------------------------------------
// Room
//-----------------------------------------------------------------------------

/// @brief  A rectangle of floor on the map.
struct Room
{
    int m_X = 0;
    int m_Y = 0;
    int m_Width = 0;
    int m_Height = 0;

    Room() = default;

    Room( const int x, const int y, const int width, const int height )
        : m_X( x ), m_Y( y ), m_Width( width ), m_Height( height ) {}

    int Left()   const { return m_X; }
    int Right()  const { return m_X + m_Width - 1; }
    int Top()    const { return m_Y; }
    int Bottom() const { return m_Y + m_Height - 1; }

    /// @brief  gets the middle cell of this Room
    /// @return the middle cell of this Room
    Vec2i Center() const
    {
        return Vec2i{ m_X + m_Width / 2, m_Y + m_Height / 2 };
    }

    /// @brief  gets the number of cells this Room covers
    /// @return the area of this Room
    int Area() const { return m_Width * m_Height; }

    /// @brief  whether a cell sits inside this Room
    /// @param  x   the column to test
    /// @param  y   the row to test
    /// @return whether the cell is inside this Room
    bool Contains( const int x, const int y ) const
    {
        return x >= Left() && x <= Right() && y >= Top() && y <= Bottom();
    }

    /// @brief  whether this Room overlaps another
    /// @param  other   the Room to test against
    /// @param  padding how many cells of wall to insist on between them
    /// @return whether the two Rooms are too close together
    bool Intersects( Room const& other, const int padding = 0 ) const
    {
        return Left()   - padding <= other.Right()
            && Right()  + padding >= other.Left()
            && Top()    - padding <= other.Bottom()
            && Bottom() + padding >= other.Top();
    }
};

//-----------------------------------------------------------------------------
// DungeonLayout
//-----------------------------------------------------------------------------

/// @brief  What a generator hands back, a finished map and the rooms inside it.
struct DungeonLayout
{
    GridSystem::Grid m_Grid;
    std::vector< Room > m_Rooms;
};

//-----------------------------------------------------------------------------
// DungeonGenerator
//-----------------------------------------------------------------------------

/// @brief  Base class for every dungeon generation algorithm.
class DungeonGenerator
{

public:

    virtual ~DungeonGenerator() = default;

    /// @brief  builds a dungeon
    /// @param  width   how many cells across
    /// @param  height  how many cells down
    /// @param  rng     the random source to draw from
    /// @return the finished map and the rooms inside it
    virtual DungeonLayout Generate( int width, int height, Random& rng ) const = 0;

    /// @brief  gets the name of this algorithm, shown under the map
    /// @return the name of this algorithm
    virtual std::string GetName() const = 0;

    /// @brief  gets the short name this algorithm is selected by on the command line
    /// @return a single lowercase word, unique across the generators
    virtual std::string GetKey() const = 0;

};

#endif //DUNGEONGENERATOR_H
