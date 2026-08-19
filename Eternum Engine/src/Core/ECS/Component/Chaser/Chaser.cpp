/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Chaser
* Description:
*     An Entity that wanders until it sees the player, then hunts.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Chaser.h"

#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Solid/Solid.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Core/Random/Random.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

namespace
{
    /// @brief  the cell that means nowhere
    const Vec2i NO_CELL{ -1, -1 };

    /// @brief  how many times to try for a roaming destination before giving up this step
    constexpr int ROAM_ATTEMPTS = 30;

    /// @brief  reads a cell out of an Entity's Transform
    /// @param  entity  the Entity to read
    /// @return the cell it stands on, or NO_CELL when it has no Transform
    Vec2i CellOf( Entity* entity )
    {
        if ( entity == nullptr )
            return NO_CELL;

        const Transform* transform = entity->GetComponent< Transform >();
        if ( transform == nullptr )
            return NO_CELL;

        Vec2f const& position = transform->GetTranslation();
        return Vec2i{ static_cast< int >( position.x() ), static_cast< int >( position.y() ) };
    }
}

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Chaser::Chaser() = default;

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

void Chaser::OnUpdate( const double deltaTime )
{
    if ( m_MoveCooldown > 0.0 )
        m_MoveCooldown -= deltaTime;

    // a staggered or dead enemy still burns its cooldown, it just cannot do anything
    if ( !CanAct() )
        return;

    LookAround();

    // reaching the player matters more than moving, so swing first
    if ( TryAttackTarget() > 0 )
        return;

    if ( m_MoveCooldown > 0.0 )
        return;

    TakeStep();

    // hunting is quicker than wandering, which is what makes being spotted feel like something
    double interval = ( m_State == ChaserState::Hunting ) ? HUNT_INTERVAL : ROAM_INTERVAL;

    if ( Entity* self = GetEntity() )
        if ( const StatusEffects* effects = self->GetComponent< StatusEffects >() )
            interval *= effects->GetCooldownMultiplier();

    m_MoveCooldown = interval;
}

bool Chaser::CanAct() const
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

int Chaser::TryAttackTarget()
{
    Entity* self = GetEntity();
    if ( self == nullptr )
        return 0;

    Attacker* attacker = self->GetComponent< Attacker >();
    if ( attacker == nullptr || !attacker->CanAttack() )
        return 0;

    Entity* target = Entities()->FindEntity( m_TargetName );
    if ( target == nullptr )
        return 0;

    const Vec2i here = currentCell();
    const Vec2i there = CellOf( target );

    if ( here == NO_CELL || there == NO_CELL )
        return 0;

    // melee only, it has to be standing right next to them
    if ( Pathfinding::ManhattanDistance( here, there ) != 1 )
        return 0;

    const Health* targetHealth = target->GetComponent< Health >();
    if ( targetHealth == nullptr || !targetHealth->IsAlive() )
        return 0;

    return attacker->Attack( target );
}

void Chaser::OnExit()
{
    GridSystem::GetInstance()->ClearOverlay( GetId() );
}

void Chaser::Inspect()
{
    std::cout << GetName()
              << " " << GetStateName( m_State )
              << " steps to goal " << m_Path.GetStepCount()
              << " cells opened " << m_Path.m_Expanded
              << std::endl;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void Chaser::LookAround()
{
    const ChaserState previous = m_State;

    if ( CanSeeTarget() )
    {
        // seen, so head straight for them and remember where they are
        m_State = ChaserState::Hunting;
        m_Destination = CellOf( Entities()->FindEntity( m_TargetName ) );
    }
    else if ( m_State == ChaserState::Hunting )
    {
        // they went round a corner, walk to where they were before giving up
        m_State = ChaserState::Searching;
    }

    if ( m_State != previous )
    {
        m_MoveCooldown = 0.0;
        publishSymbol();
    }
}

bool Chaser::TakeStep()
{
    Entity* self = GetEntity();
    if ( self == nullptr )
        return false;

    Transform* transform = self->GetComponent< Transform >();
    if ( transform == nullptr )
        return false;

    const Vec2i start = currentCell();
    if ( start == NO_CELL )
        return false;

    const Vec2i goal = chooseGoal();
    if ( goal == NO_CELL || goal == start )
    {
        m_Path = {};
        publishTrail();
        return false;
    }

    m_Path = Paths()->FindPath( start, goal );
    publishTrail();

    if ( !m_Path.IsValid() )
    {
        // nowhere to go, drop the destination so the next step picks a new one
        m_Destination = NO_CELL;
        return false;
    }

    const Vec2i next = m_Path.GetNextStep();

    // stop short of the player rather than standing on them
    if ( m_State == ChaserState::Hunting && next == goal )
        return false;

    if ( Solid::IsCellBlocked( next, self ) )
    {
        // wandering into something means go somewhere else. without this a roaming Entity
        // pushes at whatever is in the way forever, because it only chooses a new
        // destination on arriving at the old one
        if ( m_State != ChaserState::Hunting )
            m_Destination = NO_CELL;

        return false;
    }

    transform->SetTranslation( Vec2f{ static_cast< float >( next.x() ),
                                      static_cast< float >( next.y() ) } );

    GridSystem::GetInstance()->MarkDirty();
    return true;
}

void Chaser::Reset()
{
    m_State = ChaserState::Roaming;
    m_Path = {};
    m_Destination = NO_CELL;
    m_MoveCooldown = 0.0;

    GridSystem::GetInstance()->ClearOverlay( GetId() );
    publishSymbol();
}

bool Chaser::CanSeeTarget() const
{
    Entity* target = Entities()->FindEntity( m_TargetName );
    if ( target == nullptr )
        return false;

    const Vec2i self = currentCell();
    const Vec2i other = CellOf( target );

    if ( self == NO_CELL || other == NO_CELL )
        return false;

    if ( Pathfinding::ManhattanDistance( self, other ) > m_SightRange )
        return false;

    return Pathfinding::HasLineOfSight( Paths()->GetNavGrid(), self, other );
}

char Chaser::GetSymbolFor( const ChaserState state )
{
    // a capital reads as alert, lowercase as idle, so the state is visible at a glance
    return ( state == ChaserState::Roaming ) ? 'e' : 'E';
}

std::string Chaser::GetStateName( const ChaserState state )
{
    switch ( state )
    {
        case ChaserState::Roaming:   return "roaming";
        case ChaserState::Hunting:   return "hunting";
        case ChaserState::Searching: return "searching";
    }

    return "unknown";
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

Vec2i Chaser::currentCell() const
{
    return CellOf( GetEntity() );
}

Vec2i Chaser::chooseGoal()
{
    if ( m_State == ChaserState::Hunting )
        return CellOf( Entities()->FindEntity( m_TargetName ) );

    const Vec2i here = currentCell();

    if ( m_State == ChaserState::Searching )
    {
        // arrived at the last place they were seen and they are not here, give up
        if ( m_Destination == NO_CELL || m_Destination == here )
        {
            m_State = ChaserState::Roaming;
            m_Destination = NO_CELL;
            publishSymbol();
        }
        else
        {
            return m_Destination;
        }
    }

    // roaming, pick somewhere new once the last destination is reached
    if ( m_Destination == NO_CELL || m_Destination == here )
        m_Destination = pickRoamDestination();

    return m_Destination;
}

Vec2i Chaser::pickRoamDestination() const
{
    NavGrid const& grid = Paths()->GetNavGrid();

    if ( grid.GetCellCount() <= 0 )
        return NO_CELL;

    const Vec2i here = currentCell();
    Vec2i anywhere = NO_CELL;

    // sampling beats scanning the whole map, most of a dungeon is walkable
    for ( int attempt = 0; attempt < ROAM_ATTEMPTS; ++attempt )
    {
        const Vec2i candidate = grid.ToCell(
            static_cast< int >( Rng().Index( static_cast< std::size_t >( grid.GetCellCount() ) ) ) );

        if ( !grid.IsWalkable( candidate ) || candidate == here )
            continue;

        // prefer somewhere nearby so this Entity patrols an area instead of crossing the map
        if ( Pathfinding::ManhattanDistance( candidate, here ) <= ROAM_RADIUS )
            return candidate;

        anywhere = candidate;
    }

    return anywhere;
}

void Chaser::publishTrail() const
{
    GridSystem* grid = GridSystem::GetInstance().get();

    if ( !m_Path.IsValid() )
    {
        grid->ClearOverlay( GetId() );
        return;
    }

    grid->SetOverlay( GetId(), m_Path.m_Cells );
}

void Chaser::publishSymbol() const
{
    Entity* self = GetEntity();
    if ( self == nullptr )
        return;

    if ( Glyph* glyph = self->GetComponent< Glyph >() )
    {
        glyph->SetSymbol( GetSymbolFor( m_State ) );
        GridSystem::GetInstance()->MarkDirty();
    }
}

ChaserState Chaser::GetStateFromName( std::string const& name )
{
    if ( name == "hunting" )   return ChaserState::Hunting;
    if ( name == "searching" ) return ChaserState::Searching;

    return ChaserState::Roaming;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Chaser::Write( nlohmann::json& data ) const
{
    data[ "state" ] = GetStateName( m_State );
    data[ "destination" ] = { m_Destination.x(), m_Destination.y() };
    data[ "sightRange" ] = m_SightRange;
    data[ "target" ] = m_TargetName;
}

void Chaser::Read( nlohmann::json const& data )
{
    if ( data.contains( "state" ) )
        m_State = GetStateFromName( data[ "state" ].get< std::string >() );

    if ( data.contains( "destination" ) && data[ "destination" ].size() >= 2 )
        m_Destination = Vec2i{ data[ "destination" ][ 0 ].get< int >(),
                               data[ "destination" ][ 1 ].get< int >() };

    if ( data.contains( "sightRange" ) )
        m_SightRange = data[ "sightRange" ].get< int >();

    if ( data.contains( "target" ) )
        m_TargetName = data[ "target" ].get< std::string >();

    // the route belonged to the old map, work a new one out on the next step
    m_Path = {};
    m_MoveCooldown = 0.0;
}

REGISTER_COMPONENT(Chaser)
