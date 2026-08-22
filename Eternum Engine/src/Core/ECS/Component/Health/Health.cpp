/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Health
* Description:
*     How much damage an Entity can take before it dies.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Health.h"

#include <Core/ECS/Entity/Entity.h>
#include <Systems/Event/GameEvents.h>
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Health::Health() = default;

Health::Health( const int maxHealth )
    : m_MaxHealth( std::max( maxHealth, 1 ) ),
      m_Health( std::max( maxHealth, 1 ) )
{}

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void Health::OnUpdate( const double deltaTime )
{
    if ( m_MercyRemaining > 0.0 )
        m_MercyRemaining -= deltaTime;
}

void Health::Inspect()
{
    std::cout << GetName() << " " << m_Health << "/" << m_MaxHealth << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

int Health::TakeDamage( const int amount )
{
    // a hit that lands during the mercy window is thrown away entirely
    if ( IsRecovering() )
        return 0;

    const int dealt = TakeDamageIgnoringMercy( amount );

    if ( dealt > 0 )
        m_MercyRemaining = m_MercyWindow;

    return dealt;
}

int Health::TakeDamageIgnoringMercy( const int amount )
{
    if ( amount <= 0 || !IsAlive() )
        return 0;

    const int dealt = std::min( amount, m_Health );
    m_Health -= dealt;

    announceDamage( dealt );

    if ( m_Health <= 0 )
        die();

    return dealt;
}

void Health::announceDamage( const int dealt ) const
{
    Entity* owner = GetEntity();
    if ( owner == nullptr )
        return;

    DamageTakenEvent event;
    event.m_Target = owner;
    event.m_Amount = dealt;
    event.m_Fraction = static_cast< double >( dealt ) / std::max( 1, m_MaxHealth );
    event.m_Message = owner->GetName() + " took " + std::to_string( dealt );

    // sent rather than posted, the camera shake wants to react on the same frame
    Events()->Send( event );
}

int Health::Heal( const int amount )
{
    if ( amount <= 0 || !IsAlive() )
        return 0;

    const int restored = std::min( amount, m_MaxHealth - m_Health );
    m_Health += restored;

    return restored;
}

void Health::Kill()
{
    if ( !IsAlive() )
        return;

    m_Health = 0;
    die();
}

void Health::Reset()
{
    m_Health = m_MaxHealth;
    m_MercyRemaining = 0.0;
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

void Health::SetMaxHealth( const int maxHealth )
{
    m_MaxHealth = std::max( maxHealth, 1 );
    m_Health = std::min( m_Health, m_MaxHealth );
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

void Health::die()
{
    m_MercyRemaining = 0.0;

    Entity* owner = GetEntity();
    if ( owner == nullptr )
        return;

    EntityDiedEvent event;
    event.m_Entity = owner;
    event.m_Message = owner->GetName() + " died";

    // posted, because whatever listens is very likely to be looking at an Entity that is
    // about to be swept up
    Events()->Post( event );

    if ( !m_DestroyOnDeath )
        return;

    // the Scene sweeps this up on its next update, nothing dies mid frame
    owner->Destroy();
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Health::Write( nlohmann::json& data ) const
{
    data[ "health" ] = m_Health;
    data[ "maxHealth" ] = m_MaxHealth;
    data[ "mercyWindow" ] = m_MercyWindow;
    data[ "destroyOnDeath" ] = m_DestroyOnDeath;
}

void Health::Read( nlohmann::json const& data )
{
    if ( data.contains( "maxHealth" ) )
        m_MaxHealth = std::max( 1, data[ "maxHealth" ].get< int >() );

    if ( data.contains( "health" ) )
        m_Health = std::clamp( data[ "health" ].get< int >(), 0, m_MaxHealth );

    if ( data.contains( "mercyWindow" ) )
        SetMercyWindow( data[ "mercyWindow" ].get< double >() );

    if ( data.contains( "destroyOnDeath" ) )
        m_DestroyOnDeath = data[ "destroyOnDeath" ].get< bool >();

    // a load is not a hit, so nothing is recovering from anything
    m_MercyRemaining = 0.0;
}

REGISTER_COMPONENT(Health)
