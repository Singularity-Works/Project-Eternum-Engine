/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GridRegions
* Description:
*     Flood fill over a map. Used to prove a generated dungeon is one connected space and
*     to throw away pockets of floor that nothing can reach.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef GRIDREGIONS_H
#define GRIDREGIONS_H

#include <pch.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>

namespace GridRegions
{
    /// @brief  groups every walkable cell into connected regions, four way
    /// @param  grid    the map to search
    /// @param  floor   the character that counts as walkable
    /// @return one list of cells per region, largest first
    std::vector< std::vector< Vec2i > > FindRegions( GridSystem::Grid const& grid, char floor = Tiles::FLOOR );

    /// @brief  whether every walkable cell can reach every other one
    /// @param  grid    the map to test
    /// @param  floor   the character that counts as walkable
    /// @return true when there is exactly one region, or none at all
    bool IsFullyConnected( GridSystem::Grid const& grid, char floor = Tiles::FLOOR );

    /// @brief  counts the walkable cells on a map
    /// @param  grid    the map to count
    /// @param  floor   the character that counts as walkable
    /// @return how many cells are walkable
    int CountFloor( GridSystem::Grid const& grid, char floor = Tiles::FLOOR );

    /// @brief  walls off every region except the biggest one
    /// @param  grid    the map to clean up
    /// @param  floor   the character that counts as walkable
    /// @param  wall    the character to fill the discarded regions with
    /// @return how many cells were walled off
    int KeepLargestRegion( GridSystem::Grid& grid, char floor = Tiles::FLOOR, char wall = Tiles::WALL );
}

#endif //GRIDREGIONS_H
