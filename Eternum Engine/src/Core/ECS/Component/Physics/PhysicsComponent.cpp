/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PhysicsComponent
* Description:
*     TODO: Describe the purpose of this file.
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "PhysicsComponent.h"

Component* PhysicsComponent::Clone() const {
    return new PhysicsComponent(*this);
}

