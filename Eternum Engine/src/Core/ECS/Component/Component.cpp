/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Component
* Description:
*     Out of line parts of the base Component class, the ones that need a complete Entity.
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Component.h"
#include <Core/ECS/Entity/Entity.h>

std::string Component::GetName() const
{
    // an unparented Component still deserves a readable name
    if ( m_Parent == nullptr )
        return PrefixlessName( m_Type );

    return m_Parent->GetName() + "->" + PrefixlessName( m_Type );
}
