/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PlayerController
* Description:
*     Turns WASD into movement on the grid.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "PlayerController.h"

#include <Core/ECS/Entity/Entity.h>
#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Solid/Solid.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Event/GameEvents.h>
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Input/InputSystem.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

PlayerController::PlayerController() = default;

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void PlayerController::OnInit()
{
    // a scratch nudges the view, losing a whole bar throws it about
    m_DamageListener = Events()->Subscribe< DamageTakenEvent >(
        [ this ]( DamageTakenEvent const& event )
        {
            if ( event.m_Target != GetEntity() )
                return;

            GridSystem::GetInstance()->AddShake(
                SHAKE_ON_HURT + SHAKE_PER_HEALTH_LOST * event.m_Fraction );
        } );
}

void PlayerController::OnExit()
{
    Events()->Unsubscribe( m_DamageListener );
    m_DamageListener = EventSystem::NO_TOKEN;
}

void PlayerController::OnUpdate( const double deltaTime )
{
    if ( m_MoveCooldown > 0.0 )
        m_MoveCooldown -= deltaTime;

    // dead or stunned means no input at all, but the cooldown still runs down
    if ( !CanAct() )
        return;

    const Vec2i direction = GetHeldDirection();

    // nothing held, so be ready to move the instant a key goes down
    if ( direction == Vec2i{ 0, 0 } )
    {
        m_MoveCooldown = 0.0;
        return;
    }

    if ( m_MoveCooldown > 0.0 )
        return;

    if ( TryMove( direction ) )
    {
        double interval = m_MoveInterval;

        if ( Entity* self = GetEntity() )
            if ( const StatusEffects* effects = self->GetComponent< StatusEffects >() )
                interval *= effects->GetCooldownMultiplier();

        m_MoveCooldown = interval;
    }
}

bool PlayerController::CanAct() const
{
    Entity* self = GetEntity();
    if ( self == nullptr )
        return false;

    if ( const Health* health = self->GetComponent< Health >() )
        if ( !health->IsAlive() )
            return false;

    if ( const StatusEffects* effects = self->GetComponent< StatusEffects >() )
        if ( effects->IsStunned() )
            return false;

    return true;
}

Vec2i PlayerController::GetHeldDirection() const
{
    // one direction at a time, checked in a fixed order so holding two keys is not random
    if ( Input()->IsKeyDown( Key::W ) || Input()->IsKeyDown( Key::ARROW_UP ) )
        return Vec2i{ 0, -1 };

    if ( Input()->IsKeyDown( Key::S ) || Input()->IsKeyDown( Key::ARROW_DOWN ) )
        return Vec2i{ 0, 1 };

    if ( Input()->IsKeyDown( Key::A ) || Input()->IsKeyDown( Key::ARROW_LEFT ) )
        return Vec2i{ -1, 0 };

    if ( Input()->IsKeyDown( Key::D ) || Input()->IsKeyDown( Key::ARROW_RIGHT ) )
        return Vec2i{ 1, 0 };

    return Vec2i{ 0, 0 };
}

void PlayerController::Inspect()
{
    std::cout << GetName() << " steps " << m_StepsTaken << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

bool PlayerController::TryMove( Vec2i const& offset )
{
    Entity* entity = GetEntity();
    if ( entity == nullptr )
        return false;

    Transform* transform = entity->GetComponent< Transform >();
    if ( transform == nullptr )
        return false;

    const Vec2f& position = transform->GetTranslation();
    const int targetX = static_cast< int >( position.x() ) + offset.x();
    const int targetY = static_cast< int >( position.y() ) + offset.y();

    // walking into something is how you hit it, there is no separate attack key
    if ( TryAttack( Vec2i{ targetX, targetY } ) > 0 )
        return false;

    // a blocked step costs nothing, the player just does not move
    if ( !GridSystem::GetInstance()->IsWalkable( targetX, targetY ) )
        return false;

    // only solid things block. loot and traps are Entities too, and treating those as
    // walls turned them into invisible ones
    if ( Solid::IsCellBlocked( Vec2i{ targetX, targetY }, entity ) )
        return false;

    transform->SetTranslation( Vec2f{ static_cast< float >( targetX ), static_cast< float >( targetY ) } );
    ++m_StepsTaken;

    GridSystem::GetInstance()->MarkDirty();

    return true;
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

int PlayerController::TryAttack( Vec2i const& cell )
{
    Entity* self = GetEntity();
    if ( self == nullptr )
        return 0;

    Attacker* attacker = self->GetComponent< Attacker >();
    if ( attacker == nullptr )
        return 0;

    Entity* target = Entities()->FindEntityAt( cell, self );
    if ( target == nullptr )
        return 0;

    const Health* targetHealth = target->GetComponent< Health >();
    if ( targetHealth == nullptr || !targetHealth->IsAlive() )
        return 0;

    const int dealt = attacker->Attack( target );

    // a landed hit staggers whatever took it, which is what makes a fight readable
    if ( dealt > 0 )
    {
        if ( StatusEffects* effects = target->GetComponent< StatusEffects >() )
            effects->Apply( EffectType::Stun, STAGGER_DURATION );

        GridSystem::GetInstance()->AddShake( SHAKE_ON_HIT );
    }

    return dealt;
}

void PlayerController::ResetSteps()
{
    m_StepsTaken = 0;
    m_MoveCooldown = 0.0;
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void PlayerController::Write( nlohmann::json& data ) const
{
    data[ "steps" ] = m_StepsTaken;
    data[ "moveInterval" ] = m_MoveInterval;
}

void PlayerController::Read( nlohmann::json const& data )
{
    if ( data.contains( "steps" ) )
        m_StepsTaken = std::max( 0, data[ "steps" ].get< int >() );

    if ( data.contains( "moveInterval" ) )
        m_MoveInterval = std::max( 0.0, data[ "moveInterval" ].get< double >() );

    m_MoveCooldown = 0.0;
}

REGISTER_COMPONENT(PlayerController)
