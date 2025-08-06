/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Entity
* Description:
*     TODO: Describe the purpose of this file.
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Entity.h"
#include <Core/ECS/Component/Component.h>

/// @brief  default constrcutor
Entity::Entity() :
    m_Id( GetUniqueId() )
{}

/// @brief destructor
Entity::~Entity()
{
    for ( auto& [ type, component ] : m_Components )
    {
        delete component;
    }

    for (const Entity* child : m_Children)
    {
        delete child;
    }
}

