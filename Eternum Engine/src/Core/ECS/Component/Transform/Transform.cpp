/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Transform
* Description:
*     Position, rotation and scale for an Entity.
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Transform.h"
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Transform::Transform() = default;

Transform::Transform( Vec2f const& translation, const float rotation, Vec2f const& scale ) :
    m_Translation( translation ),
    m_Rotation( rotation ),
    m_Scale( scale )
{}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void Transform::Translate( Vec2f const& offset )
{
    m_Translation += offset;
    m_IsDirty = true;
}

void Transform::Rotate( const float degrees )
{
    SetRotation( m_Rotation + degrees );
}

void Transform::Inspect()
{
    std::cout << GetName()
              << " translation " << m_Translation
              << " rotation " << m_Rotation
              << " scale " << m_Scale
              << std::endl;
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

void Transform::SetTranslation( Vec2f const& translation )
{
    m_Translation = translation;
    m_IsDirty = true;
}

void Transform::SetRotation( const float rotation )
{
    // keep rotation readable instead of letting it run away
    m_Rotation = std::fmod( rotation, 360.0f );
    if ( m_Rotation < 0.0f )
        m_Rotation += 360.0f;

    m_IsDirty = true;
}

void Transform::SetScale( Vec2f const& scale )
{
    m_Scale = scale;
    m_IsDirty = true;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Transform::Write( nlohmann::json& data ) const
{
    data[ "translation" ] = { m_Translation.x(), m_Translation.y() };
    data[ "rotation" ] = m_Rotation;
    data[ "scale" ] = { m_Scale.x(), m_Scale.y() };
}

void Transform::Read( nlohmann::json const& data )
{
    if ( data.contains( "translation" ) && data[ "translation" ].size() >= 2 )
        SetTranslation( Vec2f{ data[ "translation" ][ 0 ].get< float >(),
                               data[ "translation" ][ 1 ].get< float >() } );

    if ( data.contains( "rotation" ) )
        SetRotation( data[ "rotation" ].get< float >() );

    if ( data.contains( "scale" ) && data[ "scale" ].size() >= 2 )
        SetScale( Vec2f{ data[ "scale" ][ 0 ].get< float >(),
                         data[ "scale" ][ 1 ].get< float >() } );
}

REGISTER_COMPONENT(Transform)
