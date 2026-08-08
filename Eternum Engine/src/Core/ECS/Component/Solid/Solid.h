/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Solid
* Description:
*     Marks an Entity as something you cannot walk through.
*
*     This exists because the alternative rules are all quietly wrong. Blocking on any
*     Entity turns loot and traps into invisible walls. Blocking on anything with Health
*     works today only by accident, and stops working the moment something breakable is
*     added that you are meant to walk past. So solidity says so out loud, and a thing
*     lying on the floor simply does not have one.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef SOLID_H
#define SOLID_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Solid final : public ComponentOf< Solid >
{

public:

    /// @brief  constructor
    Solid() = default;

    /// @brief  destructor
    ~Solid() override = default;

    /// @brief  Used by the Inspection System to display information about this Component
    void Inspect() override;

    /// @brief  whether anything standing on a cell blocks movement through it
    /// @param  cell    the cell to test
    /// @param  ignore  an Entity to skip, normally the one trying to move
    /// @return whether something solid is in the way
    static bool IsCellBlocked( Vec2i const& cell, Entity const* ignore );

};

#endif //SOLID_H
