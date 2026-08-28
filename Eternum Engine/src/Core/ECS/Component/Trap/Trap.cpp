/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Trap
* Description:
*     Sits on a cell looking like floor until something walks onto it.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Trap.h"

#include <nlohmann/json.hpp>

#include <Core/ECS/Component/ComponentFactory.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Event/GameEvents.h>
#include <Systems/Grid System/GridSystem.h>

namespace
{
    /// @brief  the cell that means nowhere
    const Vec2i NO_CELL{ -1, -1 };
}

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Trap::Trap() = default;

Trap::Trap( const int damage )
    : m_Damage( std::max( damage, 0 ) )
{}

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void Trap::OnUpdate( double deltaTime )
{
    if ( !m_Armed )
        return;

    const Vec2i here = currentCell();
    if ( here == NO_CELL )
        return;

    Entity* standing = Entities()->FindEntityAt( here, GetEntity() );
    if ( standing == nullptr )
        return;

    Trigger( standing );
}

void Trap::Inspect()
{
    std::cout << GetName()
              << ( m_Armed ? " armed" : " spent" )
              << ( m_Hidden ? " hidden" : " found" )
              << " damage " << m_Damage
              << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

int Trap::Trigger( Entity* victim )
{
    if ( !m_Armed || victim == nullptr )
        return 0;

    Health* health = victim->GetComponent< Health >();
    if ( health == nullptr || !health->IsAlive() )
        return 0;

    // it goes off whether or not the damage lands, so standing on a spring trap during a
    // mercy window still springs it
    m_Armed = false;
    m_Hidden = false;
    publishSymbol();

    const int dealt = health->TakeDamage( m_Damage );

    if ( m_Effect != EffectType::None )
    {
        if ( StatusEffects* effects = victim->GetComponent< StatusEffects >() )
            effects->Apply( m_Effect, m_EffectDuration, m_EffectMagnitude );
    }

    TrapTriggeredEvent event;
    event.m_Trap = GetEntity();
    event.m_Victim = victim;
    event.m_Damage = dealt;
    event.m_Message = victim->GetName() + " hit a trap for " + std::to_string( dealt );

    // posted rather than sent, a handler may well destroy whatever just stepped on it
    Events()->Post( event );

    return dealt;
}

void Trap::Reset()
{
    m_Armed = true;
    m_Hidden = true;
    publishSymbol();
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

void Trap::SetEffect( const EffectType effect, const double duration, const int magnitude )
{
    m_Effect = effect;
    m_EffectDuration = std::max( duration, 0.0 );
    m_EffectMagnitude = std::max( magnitude, 0 );
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Trap::Write( nlohmann::json& data ) const
{
    data[ "damage" ] = m_Damage;
    data[ "armed" ] = m_Armed;
    data[ "hidden" ] = m_Hidden;
    data[ "effect" ] = StatusEffects::GetEffectName( m_Effect );
    data[ "effectDuration" ] = m_EffectDuration;
    data[ "effectMagnitude" ] = m_EffectMagnitude;
}

void Trap::Read( nlohmann::json const& data )
{
    m_Damage = std::max( 0, data.value( "damage", DEFAULT_DAMAGE ) );
    m_Armed = data.value( "armed", true );
    m_Hidden = data.value( "hidden", true );

    m_Effect = StatusEffects::GetEffectFromName( data.value( "effect", std::string() ) );
    m_EffectDuration = data.value( "effectDuration", DEFAULT_EFFECT_DURATION );
    m_EffectMagnitude = data.value( "effectMagnitude", DEFAULT_EFFECT_MAGNITUDE );
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

Vec2i Trap::currentCell() const
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

void Trap::publishSymbol() const
{
    Entity* owner = GetEntity();
    if ( owner == nullptr )
        return;

    if ( Glyph* glyph = owner->GetComponent< Glyph >() )
    {
        glyph->SetSymbol( m_Hidden ? HIDDEN_SYMBOL : REVEALED_SYMBOL );
        GridSystem::GetInstance()->MarkDirty();
    }
}

REGISTER_COMPONENT(Trap)
