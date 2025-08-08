/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Transform
* Description:
*     TODO: Describe the purpose of this file.
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Transform final : public Component{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Transform();

    /// @brief destructor
    ~Transform() override = default;




};

REGISTER_COMPONENT_SYSTEM(Transform)

#endif //TRANSFORM_H
