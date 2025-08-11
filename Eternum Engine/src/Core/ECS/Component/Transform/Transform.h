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

private:
    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief The position of this Transform
    Vec2f m_Translation = Vec2f::Zero();

    /// @brief the rotation of this Transform in degrees
    float m_Rotation = 0.0f;

    /// @brief the scale of this Transform
    Vec2f m_Scale = Vec2f::Zero();

    /// @brief  flag for when the matrix needs to be regenerated
    bool m_IsDirty = true;

};

REGISTER_COMPONENT_SYSTEM(Transform)
#endif //TRANSFORM_H
