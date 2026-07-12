/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Transform
* Description:
*     Position, rotation and scale for an Entity. The first real Component in the engine
*     and the reference for how a Component should be written.
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

class Transform final : public ComponentOf< Transform >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Transform();

    /// @brief  constructor
    /// @param  translation the starting position
    /// @param  rotation    the starting rotation in degrees
    /// @param  scale       the starting scale
    explicit Transform( Vec2f const& translation, float rotation = 0.0f, Vec2f const& scale = Vec2f( 1.0f ) );

    /// @brief destructor
    ~Transform() override = default;

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  moves this Transform by an offset
    /// @param  offset  how far to move
    void Translate( Vec2f const& offset );

    /// @brief  turns this Transform by an amount
    /// @param  degrees how far to turn in degrees
    void Rotate( float degrees );

    /// @brief  Used by the Inspection System to display information about this Component
    void Inspect() override;

    /// @brief  writes this Component's state so it can be saved
    /// @param  data    the object to write into
    void Write( nlohmann::json& data ) const override;

    /// @brief  reads this Component's state back out of a save
    /// @param  data    the object to read from
    void Read( nlohmann::json const& data ) override;


    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets the position of this Transform
    /// @return the position of this Transform
    Vec2f const& GetTranslation() const { return m_Translation; }

    /// @brief  sets the position of this Transform
    /// @param  translation the new position
    void SetTranslation( Vec2f const& translation );

    /// @brief  gets the rotation of this Transform in degrees
    /// @return the rotation of this Transform in degrees
    float GetRotation() const { return m_Rotation; }

    /// @brief  sets the rotation of this Transform
    /// @param  rotation    the new rotation in degrees
    void SetRotation( float rotation );

    /// @brief  gets the scale of this Transform
    /// @return the scale of this Transform
    Vec2f const& GetScale() const { return m_Scale; }

    /// @brief  sets the scale of this Transform
    /// @param  scale   the new scale
    void SetScale( Vec2f const& scale );

    /// @brief  gets whether this Transform changed since the last time it was cleared
    /// @return whether this Transform needs its matrix regenerated
    bool IsDirty() const { return m_IsDirty; }

    /// @brief  marks this Transform as up to date
    void ClearDirty() { m_IsDirty = false; }

private:
    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief The position of this Transform
    Vec2f m_Translation = Vec2f::Zero();

    /// @brief the rotation of this Transform in degrees
    float m_Rotation = 0.0f;

    /// @brief the scale of this Transform
    Vec2f m_Scale = Vec2f( 1.0f );

    /// @brief  flag for when the matrix needs to be regenerated
    bool m_IsDirty = true;

};

#endif //TRANSFORM_H
