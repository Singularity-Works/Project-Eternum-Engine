/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: BspGenerator
* Description:
*     Binary space partitioning. Cuts the map in half over and over until the pieces are
*     small enough, drops one room into each piece, then joins the rooms with a corridor
*     graph. Rooms cannot overlap because each one owns its own slice of the map.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef BSPGENERATOR_H
#define BSPGENERATOR_H

#include <pch.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>

class BspGenerator final : public DungeonGenerator
{

public:

    DungeonLayout Generate( int width, int height, Random& rng ) const override;

    std::string GetName() const override { return "BSP rooms"; }

    std::string GetKey() const override { return "bsp"; }

    /// @brief  a piece of the map is never cut smaller than this on either side
    static constexpr int MIN_PARTITION_SIZE = 7;

    /// @brief  the smallest room that will be placed
    static constexpr int MIN_ROOM_SIZE = 3;

    /// @brief  how many times the map can be cut in half
    static constexpr int MAX_DEPTH = 5;

    /// @brief  corridors added on top of the spanning tree, these are what make loops
    static constexpr int EXTRA_CORRIDORS = 2;

};

#endif //BSPGENERATOR_H
