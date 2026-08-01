/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CaveGenerator
* Description:
*     Cellular automata. Fills the map with random noise, then repeatedly turns each cell
*     into whatever most of its neighbours are, which pulls the noise into cave shapes.
*     Anything left stranded afterwards is walled off so the result is one connected space.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef CAVEGENERATOR_H
#define CAVEGENERATOR_H

#include <pch.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>

class CaveGenerator final : public DungeonGenerator
{

public:

    DungeonLayout Generate( int width, int height, Random& rng ) const override;

    std::string GetName() const override { return "Cave"; }

    std::string GetKey() const override { return "cave"; }

    /// @brief  how much of the starting noise is wall, as a percentage
    static constexpr int FILL_PERCENT = 45;

    /// @brief  how many times the smoothing pass runs
    static constexpr int SMOOTH_STEPS = 5;

    /// @brief  a cell with more wall neighbours than this becomes wall, fewer becomes floor
    static constexpr int WALL_THRESHOLD = 4;

};

#endif //CAVEGENERATOR_H
