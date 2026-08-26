/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: StatusEffects
* Description:
*     Timed effects hanging off an Entity.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "StatusEffects.h"

#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Entity/Entity.h>
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

StatusEffects::StatusEffects() = default;

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void StatusEffects::OnUpdate( const double deltaTime )
{
    for ( StatusEffect& effect : m_Effects )
    {
        effect.m_Remaining -= deltaTime;
        effect.m_TickRemaining -= deltaTime;

        if ( effect.m_TickRemaining <= 0.0 )
        {
            applyTick( effect );
            effect.m_TickRemaining = POISON_TICK;
        }
    }

    // anything that ran out this frame goes
    m_Effects.erase(
        std::remove_if( m_Effects.begin(), m_Effects.end(),
            []( StatusEffect const& effect ) { return effect.m_Remaining <= 0.0; } ),
        m_Effects.end() );
}

void StatusEffects::Inspect()
{
    const std::string summary = GetSummary();
    std::cout << GetName() << " " << ( summary.empty() ? "clear" : summary ) << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void StatusEffects::Apply( const EffectType type, const double duration, const int magnitude )
{
    if ( type == EffectType::None || duration <= 0.0 )
        return;

    if ( StatusEffect* existing = find( type ) )
    {
        // refresh rather than stack, so repeated hits cannot pile up forever
        existing->m_Remaining = std::max( existing->m_Remaining, duration );
        existing->m_Magnitude = std::max( existing->m_Magnitude, magnitude );
        return;
    }

    StatusEffect effect;
    effect.m_Type = type;
    effect.m_Remaining = duration;
    effect.m_Magnitude = magnitude;
    effect.m_TickRemaining = POISON_TICK;

    m_Effects.push_back( effect );
}

void StatusEffects::Clear( const EffectType type )
{
    m_Effects.erase(
        std::remove_if( m_Effects.begin(), m_Effects.end(),
            [ type ]( StatusEffect const& effect ) { return effect.m_Type == type; } ),
        m_Effects.end() );
}

void StatusEffects::ClearAll()
{
    m_Effects.clear();
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

bool StatusEffects::Has( const EffectType type ) const
{
    return find( type ) != nullptr;
}

double StatusEffects::GetRemaining( const EffectType type ) const
{
    StatusEffect const* effect = find( type );
    return ( effect != nullptr ) ? effect->m_Remaining : 0.0;
}

double StatusEffects::GetCooldownMultiplier() const
{
    return Has( EffectType::Slow ) ? SLOW_MULTIPLIER : 1.0;
}

std::string StatusEffects::GetSummary() const
{
    std::string summary;

    for ( StatusEffect const& effect : m_Effects )
    {
        if ( !summary.empty() )
            summary += ", ";

        summary += GetEffectName( effect.m_Type );
    }

    return summary;
}

std::string StatusEffects::GetEffectName( const EffectType type )
{
    switch ( type )
    {
        case EffectType::Poison: return "poisoned";
        case EffectType::Slow:   return "slowed";
        case EffectType::Stun:   return "stunned";
        case EffectType::None:   break;
    }

    return "none";
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

StatusEffect* StatusEffects::find( const EffectType type )
{
    const auto it = std::find_if( m_Effects.begin(), m_Effects.end(),
        [ type ]( StatusEffect const& effect ) { return effect.m_Type == type; } );

    return ( it != m_Effects.end() ) ? &( *it ) : nullptr;
}

StatusEffect const* StatusEffects::find( const EffectType type ) const
{
    const auto it = std::find_if( m_Effects.begin(), m_Effects.end(),
        [ type ]( StatusEffect const& effect ) { return effect.m_Type == type; } );

    return ( it != m_Effects.end() ) ? &( *it ) : nullptr;
}

void StatusEffects::applyTick( StatusEffect const& effect ) const
{
    if ( effect.m_Type != EffectType::Poison || effect.m_Magnitude <= 0 )
        return;

    Entity* owner = GetEntity();
    if ( owner == nullptr )
        return;

    if ( Health* health = owner->GetComponent< Health >() )
    {
        // poison ignores the mercy window, otherwise it would never land during a fight
        health->TakeDamageIgnoringMercy( effect.m_Magnitude );
    }
}

EffectType StatusEffects::GetEffectFromName( std::string const& name )
{
    if ( name == "poisoned" ) return EffectType::Poison;
    if ( name == "slowed" )   return EffectType::Slow;
    if ( name == "stunned" )  return EffectType::Stun;

    return EffectType::None;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void StatusEffects::Write( nlohmann::json& data ) const
{
    nlohmann::json effects = nlohmann::json::array();

    for ( StatusEffect const& effect : m_Effects )
    {
        effects.push_back( {
            { "type", GetEffectName( effect.m_Type ) },
            { "remaining", effect.m_Remaining },
            { "magnitude", effect.m_Magnitude },
            { "tickRemaining", effect.m_TickRemaining }
        } );
    }

    data[ "effects" ] = effects;
}

void StatusEffects::Read( nlohmann::json const& data )
{
    m_Effects.clear();

    if ( !data.contains( "effects" ) || !data[ "effects" ].is_array() )
        return;

    for ( nlohmann::json const& entry : data[ "effects" ] )
    {
        StatusEffect effect;
        effect.m_Type = GetEffectFromName( entry.value( "type", std::string() ) );

        // an unknown effect name is dropped rather than loaded as a broken one
        if ( effect.m_Type == EffectType::None )
            continue;

        effect.m_Remaining = entry.value( "remaining", 0.0 );
        effect.m_Magnitude = entry.value( "magnitude", 0 );
        effect.m_TickRemaining = entry.value( "tickRemaining", POISON_TICK );

        if ( effect.m_Remaining > 0.0 )
            m_Effects.push_back( effect );
    }
}

REGISTER_COMPONENT(StatusEffects)
