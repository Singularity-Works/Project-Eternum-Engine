/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Pickup
* Description:
*     Something lying on the floor that heals whatever walks over it.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Pickup.h"

#include <nlohmann/json.hpp>

#include <Core/ECS/Component/ComponentFactory.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Event/GameEvents.h>

namespace
{
    /// @brief  the cell that means nowhere
    const Vec2i NO_CELL{ -1, -1 };
}

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Pickup::Pickup() = default;

Pickup::Pickup( const int restores )
    : m_Restores( std::max( restores, 0 ) )
{}

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void Pickup::OnUpdate( double deltaTime )
{
    if ( m_Collected )
        return;

    const Vec2i here = currentCell();
    if ( here == NO_CELL )
        return;

    Entity* standing = Entities()->FindEntityAt( here, GetEntity() );
    if ( standing == nullptr )
        return;

    Collect( standing );
}

void Pickup::Inspect()
{
    std::cout << GetName()
              << ( m_Collected ? " taken" : " waiting" )
              << " restores " << m_Restores
              << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

int Pickup::Collect( Entity* collector )
{
    if ( m_Collected || collector == nullptr )
        return 0;

    Health* health = collector->GetComponent< Health >();
    if ( health == nullptr || !health->IsAlive() )
        return 0;

    const int restored = health->Heal( m_Restores );

    // on full health it stays on the floor rather than being thrown away
    if ( restored <= 0 )
        return 0;

    m_Collected = true;

    PickupCollectedEvent event;
    event.m_Pickup = GetEntity();
    event.m_Collector = collector;
    event.m_Restored = restored;
    event.m_Message = collector->GetName() + " recovered " + std::to_string( restored );

    Events()->Post( event );

    // posted first, so the log line survives this Entity being swept up
    if ( Entity* owner = GetEntity() )
        owner->Destroy();

    return restored;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Pickup::Write( nlohmann::json& data ) const
{
    data[ "restores" ] = m_Restores;
    data[ "collected" ] = m_Collected;
}

void Pickup::Read( nlohmann::json const& data )
{
    m_Restores = std::max( 0, data.value( "restores", DEFAULT_RESTORES ) );
    m_Collected = data.value( "collected", false );
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

Vec2i Pickup::currentCell() const
{
    Entity* owner = GetEntity();
    if ( owner == nullptr )
        return NO_CELL;

    const Transform* transform = owner->GetComponent< Transform >();
    if ( transform == nullptr )
        return NO_CELL;

    Vec2f const& position = transform->GetTranslation();
    return Vec2i{ static_cast< int >( position.x() ), static_cast< int >( position.y() ) };
}

REGISTER_COMPONENT(Pickup)
