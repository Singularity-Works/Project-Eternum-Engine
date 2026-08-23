/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Attacker
* Description:
*     The ability to hurt something.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Attacker.h"

#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Entity/Entity.h>
#include <Core/Random/Random.h>
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Attacker::Attacker() = default;

Attacker::Attacker( const int damage, const double interval )
    : m_Damage( std::max( damage, 0 ) ),
      m_AttackInterval( std::max( interval, 0.0 ) )
{}

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void Attacker::OnUpdate( const double deltaTime )
{
    if ( m_Cooldown > 0.0 )
        m_Cooldown -= deltaTime;
}

void Attacker::Inspect()
{
    std::cout << GetName()
              << " damage " << m_Damage
              << " every " << m_AttackInterval << "s"
              << " total dealt " << m_TotalDamageDealt
              << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

int Attacker::Attack( Entity* target )
{
    if ( target == nullptr || !CanAttack() )
        return 0;

    Health* health = target->GetComponent< Health >();
    if ( health == nullptr || !health->IsAlive() )
        return 0;

    const int dealt = health->TakeDamage( m_Damage );

    // the swing happened either way, a target still recovering just shrugs it off
    ResetCooldown();

    if ( dealt <= 0 )
        return 0;

    m_TotalDamageDealt += dealt;

    if ( m_OnHit.m_Type != EffectType::None && Rng().Chance( m_OnHit.m_Chance ) )
    {
        if ( StatusEffects* effects = target->GetComponent< StatusEffects >() )
            effects->Apply( m_OnHit.m_Type, m_OnHit.m_Duration, m_OnHit.m_Magnitude );
    }

    return dealt;
}

void Attacker::ResetCooldown()
{
    m_Cooldown = m_AttackInterval;
}

void Attacker::Reset()
{
    m_Cooldown = 0.0;
    m_TotalDamageDealt = 0;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Attacker::Write( nlohmann::json& data ) const
{
    data[ "damage" ] = m_Damage;
    data[ "attackInterval" ] = m_AttackInterval;
    data[ "totalDamageDealt" ] = m_TotalDamageDealt;

    data[ "onHit" ] = {
        { "type", StatusEffects::GetEffectName( m_OnHit.m_Type ) },
        { "duration", m_OnHit.m_Duration },
        { "magnitude", m_OnHit.m_Magnitude },
        { "chance", m_OnHit.m_Chance }
    };
}

void Attacker::Read( nlohmann::json const& data )
{
    if ( data.contains( "damage" ) )
        SetDamage( data[ "damage" ].get< int >() );

    if ( data.contains( "attackInterval" ) )
        SetAttackInterval( data[ "attackInterval" ].get< double >() );

    if ( data.contains( "totalDamageDealt" ) )
        m_TotalDamageDealt = std::max( 0, data[ "totalDamageDealt" ].get< int >() );

    if ( data.contains( "onHit" ) )
    {
        nlohmann::json const& onHit = data[ "onHit" ];

        OnHit loaded;
        loaded.m_Type = StatusEffects::GetEffectFromName( onHit.value( "type", std::string() ) );
        loaded.m_Duration = onHit.value( "duration", 0.0 );
        loaded.m_Magnitude = onHit.value( "magnitude", 0 );
        loaded.m_Chance = onHit.value( "chance", 100 );

        m_OnHit = loaded;
    }

    // a load is not a swing, so the next one is ready
    m_Cooldown = 0.0;
}

REGISTER_COMPONENT(Attacker)
