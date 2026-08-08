/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Solid
* Description:
*     Marks an Entity as something you cannot walk through.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Solid.h"

#include <Core/ECS/Component/ComponentFactory.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>

void Solid::Inspect()
{
    std::cout << GetName() << " blocks movement" << std::endl;
}

bool Solid::IsCellBlocked( Vec2i const& cell, Entity const* ignore )
{
    Entity* occupant = Entities()->FindEntityAt( cell, ignore );
    if ( occupant == nullptr )
        return false;

    return occupant->GetComponent< Solid >() != nullptr;
}

REGISTER_COMPONENT(Solid)
