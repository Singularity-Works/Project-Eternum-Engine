/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: RoomGenerator
* Description:
*     Scattered rooms. Throws rooms at the map at random and keeps the ones that do not
*     land on top of anything, then joins what stuck with a corridor graph. Looser and
*     less even than BSP, which is the point of having both.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef ROOMGENERATOR_H
#define ROOMGENERATOR_H

#include <pch.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>

class RoomGenerator final : public DungeonGenerator
{

public:

    DungeonLayout Generate( int width, int height, Random& rng ) const override;

    std::string GetName() const override { return "Scattered rooms"; }

    std::string GetKey() const override { return "rooms"; }

    /// @brief  the smallest room that will be placed
    static constexpr int MIN_ROOM_SIZE = 4;

    /// @brief  the largest room that will be placed
    static constexpr int MAX_ROOM_SIZE = 9;

    /// @brief  how many rooms to aim for
    static constexpr int TARGET_ROOMS = 10;

    /// @brief  how many placements to try before giving up on reaching the target
    static constexpr int MAX_ATTEMPTS = 300;

    /// @brief  corridors added on top of the spanning tree, these are what make loops
    static constexpr int EXTRA_CORRIDORS = 3;

};

#endif //ROOMGENERATOR_H
