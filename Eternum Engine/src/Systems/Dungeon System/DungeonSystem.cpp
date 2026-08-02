/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: DungeonSystem
* Description:
*     Picks a generation algorithm, runs it, and sends the result to the GridSystem.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "DungeonSystem.h"

#include "Generation/BspGenerator.h"
#include "Generation/CaveGenerator.h"
#include "Generation/RoomGenerator.h"
#include "Generation/GridRegions.h"

#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Pickup/Pickup.h>
#include <Core/ECS/Component/Solid/Solid.h>
#include <Core/ECS/Component/Trap/Trap.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Input/InputSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <Systems/Event/GameEvents.h>
#include <nlohmann/json.hpp>

namespace
{
    /// @brief  the cell that means nothing was found
    const Vec2i NO_CELL{ -1, -1 };
}

DungeonSystem::DungeonSystem()
    : System( "Dungeon System" )
{
    // the order here is the order the number keys select
    m_Generators.push_back( std::make_unique< BspGenerator >() );
    m_Generators.push_back( std::make_unique< RoomGenerator >() );
    m_Generators.push_back( std::make_unique< CaveGenerator >() );
}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void DungeonSystem::Init()
{
    System::Init();

    m_IsStarted = true;

    // open on a real dungeon instead of a blank screen
    if ( m_HasStartupSeed )
        StartNewDungeon( m_StartupSeed );
    else
        StartNewDungeon();
}

void DungeonSystem::Shutdown()
{
    System::Shutdown();
}

void DungeonSystem::Update( double deltaTime )
{
    if ( Input()->IsKeyPressed( Key::M ) )
        StartNewDungeon();

    if ( Input()->IsKeyPressed( Key::NUM_1 ) )
        SetGenerator( 0 );

    if ( Input()->IsKeyPressed( Key::NUM_2 ) )
        SetGenerator( 1 );

    if ( Input()->IsKeyPressed( Key::NUM_3 ) )
        SetGenerator( 2 );

}

void DungeonSystem::FixedUpdate()
{
}

void DungeonSystem::Render()
{
}

// ----------------------------------------------------------------
// Generation
// ----------------------------------------------------------------

void DungeonSystem::StartNewDungeon()
{
    StartNewDungeon( Rng().Reseed() );
}

void DungeonSystem::StartNewDungeon( const unsigned seed )
{
    if ( m_Generators.empty() )
        return;

    m_Seed = seed;

    // everything downstream of here draws from this one source, so the seed decides the
    // map and where the player ends up standing on it
    m_Rng.Seed( seed );

    // gameplay draws from the shared source, so seed that too or the dungeon repeats but
    // the enemies wandering around it do not
    Rng().Seed( seed );
    m_Layout = m_Generators[ m_GeneratorIndex ]->Generate( DUNGEON_WIDTH, DUNGEON_HEIGHT, m_Rng );

    SendToGridSystem( "GeneratedDungeon" );

    const Vec2i playerCell = spawnPlayer();
    spawnEnemies( playerCell );
    spawnProps( playerCell );

    Paths()->ResetStatistics();

    DungeonGeneratedEvent generated;
    generated.m_Seed = m_Seed;
    generated.m_Generator = GetGeneratorName();
    generated.m_Message = "New dungeon, seed " + std::to_string( m_Seed );
    Events()->Post( generated );
}

void DungeonSystem::SetGenerator( const std::size_t index )
{
    if ( index >= m_Generators.size() || index == m_GeneratorIndex )
        return;

    m_GeneratorIndex = index;

    // before the engine starts this only picks what the first dungeon will use
    if ( m_IsStarted )
        StartNewDungeon();
}

std::size_t DungeonSystem::FindGenerator( std::string const& name ) const
{
    std::string wanted = name;
    std::transform( wanted.begin(), wanted.end(), wanted.begin(),
        []( const unsigned char c ) { return static_cast< char >( std::tolower( c ) ); } );

    // match the short key exactly, names overlap and would pick the wrong one
    for ( std::size_t i = 0; i < m_Generators.size(); ++i )
    {
        if ( m_Generators[ i ]->GetKey() == wanted )
            return i;
    }

    return m_Generators.size();
}

void DungeonSystem::SetStartupSeed( const unsigned seed )
{
    m_StartupSeed = seed;
    m_HasStartupSeed = true;
}

void DungeonSystem::SendToGridSystem( const std::string& mapName )
{
    GridSystem::GetInstance()->AddMap( mapName, m_Layout.m_Grid );
    GridSystem::GetInstance()->LoadMap( mapName );
    GridSystem::GetInstance()->ClearAllOverlays();

    if (!GridSystem::GetInstance()->IsContinuousShake())
        GridSystem::GetInstance()->ClearShake();

    GridSystem::GetInstance()->MarkDirty();

    // the routes everything was following belong to a map that no longer exists
    Paths()->InvalidateNavGrid();
}

// ----------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------

void DungeonSystem::WriteState( nlohmann::json& data ) const
{
    data[ "generator" ] = ( m_GeneratorIndex < m_Generators.size() )
        ? m_Generators[ m_GeneratorIndex ]->GetKey()
        : std::string( "bsp" );

    data[ "seed" ] = m_Seed;
    data[ "width" ] = m_Layout.m_Grid.m_Dimension.m_Width;
    data[ "height" ] = m_Layout.m_Grid.m_Dimension.m_Height;

    // the whole map as one string, which keeps the file readable and small
    data[ "cells" ] = std::string( m_Layout.m_Grid.cells.begin(), m_Layout.m_Grid.cells.end() );

    nlohmann::json rooms = nlohmann::json::array();
    for ( Room const& room : m_Layout.m_Rooms )
        rooms.push_back( { { "x", room.m_X }, { "y", room.m_Y },
                           { "w", room.m_Width }, { "h", room.m_Height } } );

    data[ "rooms" ] = rooms;
}

void DungeonSystem::ReadState( nlohmann::json const& data )
{
    const int width = data.value( "width", 0 );
    const int height = data.value( "height", 0 );
    const std::string cells = data.value( "cells", std::string() );

    if ( width <= 0 || height <= 0
      || cells.size() != static_cast< std::size_t >( width ) * height )
    {
        std::cout << "WARNING: save has a map that does not add up, leaving the current one"
                  << std::endl;
        return;
    }

    m_Seed = data.value( "seed", 0u );

    const std::size_t generator = FindGenerator( data.value( "generator", std::string() ) );
    if ( generator < m_Generators.size() )
        m_GeneratorIndex = generator;

    m_Layout.m_Grid = GridSystem::Grid( width, height );
    m_Layout.m_Grid.cells.assign( cells.begin(), cells.end() );

    m_Layout.m_Rooms.clear();
    if ( data.contains( "rooms" ) )
    {
        for ( nlohmann::json const& room : data[ "rooms" ] )
        {
            m_Layout.m_Rooms.emplace_back( room.value( "x", 0 ), room.value( "y", 0 ),
                                           room.value( "w", 0 ), room.value( "h", 0 ) );
        }
    }

    SendToGridSystem( "GeneratedDungeon" );
}

// ----------------------------------------------------------------
// Queries
// ----------------------------------------------------------------

Vec2i DungeonSystem::GetRandomSpawnPoint()
{
    // a room centre is the nicest place to stand when the layout has rooms
    if ( !m_Layout.m_Rooms.empty() )
        return GetRandomRoomCenter();

    // a cave has no rooms, so take any open cell in the largest open space
    const std::vector< std::vector< Vec2i > > regions = GridRegions::FindRegions( m_Layout.m_Grid );
    if ( regions.empty() || regions.front().empty() )
        return NO_CELL;

    return regions.front()[ m_Rng.Index( regions.front().size() ) ];
}

Vec2i DungeonSystem::GetRandomRoomCenter()
{
    if ( m_Layout.m_Rooms.empty() )
        return NO_CELL;

    return m_Layout.m_Rooms[ m_Rng.Index( m_Layout.m_Rooms.size() ) ].Center();
}

Vec2i DungeonSystem::GetRandomTileInRoom( Room const& room )
{
    // stay off the room's own edge so nothing spawns against a wall
    const int minX = ( room.m_Width  > 2 ) ? room.Left() + 1 : room.Left();
    const int maxX = ( room.m_Width  > 2 ) ? room.Right() - 1 : room.Right();
    const int minY = ( room.m_Height > 2 ) ? room.Top() + 1 : room.Top();
    const int maxY = ( room.m_Height > 2 ) ? room.Bottom() - 1 : room.Bottom();

    return Vec2i{ m_Rng.Range( minX, maxX ), m_Rng.Range( minY, maxY ) };
}

Vec2i DungeonSystem::GetRandomRoomEdge()
{
    if ( m_Layout.m_Rooms.empty() )
        return NO_CELL;

    Room const& room = m_Layout.m_Rooms[ m_Rng.Index( m_Layout.m_Rooms.size() ) ];

    switch ( m_Rng.Range( 0, 3 ) )
    {
        case 0:  return Vec2i{ m_Rng.Range( room.Left(), room.Right() ), room.Top() };
        case 1:  return Vec2i{ m_Rng.Range( room.Left(), room.Right() ), room.Bottom() };
        case 2:  return Vec2i{ room.Left(),  m_Rng.Range( room.Top(), room.Bottom() ) };
        default: return Vec2i{ room.Right(), m_Rng.Range( room.Top(), room.Bottom() ) };
    }
}

std::string DungeonSystem::GetGeneratorName() const
{
    if ( m_GeneratorIndex >= m_Generators.size() )
        return "None";

    return m_Generators[ m_GeneratorIndex ]->GetName();
}

// ----------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------

Vec2i DungeonSystem::spawnPlayer()
{
    const Vec2i spawn = GetRandomSpawnPoint();
    if ( spawn == NO_CELL )
        return NO_CELL;

    Entity* player = Entities()->FindEntity( "Player" );

    if ( player == nullptr )
    {
        player = Entities()->CreateEntity( "Player" );
        player->AddComponent( new Transform() );
        player->AddComponent( new Glyph( '@', 10 ) );
        player->AddComponent( new PlayerController() );
        player->AddComponent( new StatusEffects() );
        player->AddComponent( new Solid() );

        Health* health = new Health( PLAYER_HEALTH );
        // the player stays in the Scene after dying so the game has something to report
        health->SetDestroyOnDeath( false );
        player->AddComponent( health );

        player->AddComponent( new Attacker( PLAYER_DAMAGE, PLAYER_ATTACK_INTERVAL ) );
    }

    player->GetComponent< Transform >()->SetTranslation(
        Vec2f{ static_cast< float >( spawn.x() ), static_cast< float >( spawn.y() ) } );

    player->GetComponent< PlayerController >()->ResetSteps();
    player->GetComponent< Health >()->Reset();
    player->GetComponent< Attacker >()->Reset();
    player->GetComponent< StatusEffects >()->ClearAll();

    GridSystem::GetInstance()->MarkDirty();
    return spawn;
}

Vec2i DungeonSystem::pickEnemyCell( Vec2i const& playerCell )
{
    // try a handful of times for somewhere far from the player, then take what we can get
    Vec2i fallback = NO_CELL;

    for ( int attempt = 0; attempt < 20; ++attempt )
    {
        const Vec2i candidate = GetRandomSpawnPoint();
        if ( candidate == NO_CELL )
            return NO_CELL;

        fallback = candidate;

        if ( Pathfinding::ManhattanDistance( candidate, playerCell ) >= ENEMY_SPAWN_DISTANCE )
            return candidate;
    }

    return fallback;
}

void DungeonSystem::spawnEnemies( Vec2i const& playerCell )
{
    if ( playerCell == NO_CELL )
        return;

    for ( int i = 0; i < ENEMY_COUNT; ++i )
    {
        const std::string name = "Enemy" + std::to_string( i );

        Entity* enemy = Entities()->FindEntity( name );

        if ( enemy == nullptr )
        {
            enemy = Entities()->CreateEntity( name );
            enemy->AddComponent( new Transform() );
            enemy->AddComponent( new Glyph( Chaser::GetSymbolFor( ChaserState::Roaming ), 5 ) );
            enemy->AddComponent( new Chaser() );
            enemy->AddComponent( new Solid() );
            enemy->AddComponent( new Health( ENEMY_HEALTH ) );
            enemy->AddComponent( new StatusEffects() );

            Attacker* attacker = new Attacker( ENEMY_DAMAGE, ENEMY_ATTACK_INTERVAL );

            // their bite sometimes leaves you poisoned and always slows you for a moment
            Attacker::OnHit onHit;
            onHit.m_Type = EffectType::Poison;
            onHit.m_Duration = POISON_DURATION;
            onHit.m_Magnitude = POISON_DAMAGE;
            onHit.m_Chance = POISON_CHANCE;
            attacker->SetOnHit( onHit );

            enemy->AddComponent( attacker );
        }

        const Vec2i cell = pickEnemyCell( playerCell );
        if ( cell == NO_CELL )
            continue;

        enemy->GetComponent< Transform >()->SetTranslation(
            Vec2f{ static_cast< float >( cell.x() ), static_cast< float >( cell.y() ) } );

        Chaser* chaser = enemy->GetComponent< Chaser >();
        chaser->SetSightRange( ENEMY_SIGHT_RANGE );
        chaser->Reset();

        enemy->GetComponent< Health >()->Reset();
        enemy->GetComponent< Attacker >()->Reset();
        enemy->GetComponent< StatusEffects >()->ClearAll();
    }
}

void DungeonSystem::spawnProps( Vec2i const& playerCell )
{
    // the props from the last dungeon belong to a map that no longer exists
    for ( Entity* entity : Entities()->GetEntities() )
    {
        if ( entity->GetComponent< Trap >() != nullptr
          || entity->GetComponent< Pickup >() != nullptr )
        {
            entity->Destroy();
        }
    }

    for ( int i = 0; i < TRAP_COUNT; ++i )
    {
        const Vec2i cell = pickPropCell( playerCell );
        if ( cell == NO_CELL )
            continue;

        Entity* trap = Entities()->CreateEntity( "Trap" + std::to_string( i ) );
        trap->AddComponent( new Transform( Vec2f{ static_cast< float >( cell.x() ),
                                                  static_cast< float >( cell.y() ) } ) );

        // drawn as floor until something finds it the hard way
        trap->AddComponent( new Glyph( Trap::HIDDEN_SYMBOL, 1 ) );
        trap->AddComponent( new Trap( TRAP_DAMAGE ) );
    }

    for ( int i = 0; i < PICKUP_COUNT; ++i )
    {
        const Vec2i cell = pickPropCell( playerCell );
        if ( cell == NO_CELL )
            continue;

        Entity* pickup = Entities()->CreateEntity( "Pickup" + std::to_string( i ) );
        pickup->AddComponent( new Transform( Vec2f{ static_cast< float >( cell.x() ),
                                                    static_cast< float >( cell.y() ) } ) );
        pickup->AddComponent( new Glyph( Pickup::SYMBOL, 2 ) );
        pickup->AddComponent( new Pickup( PICKUP_RESTORES ) );
    }
}

Vec2i DungeonSystem::pickPropCell( Vec2i const& playerCell )
{
    // never underfoot, walking onto a fresh dungeon should not set anything off
    for ( int attempt = 0; attempt < 30; ++attempt )
    {
        const Vec2i candidate = GetRandomSpawnPoint();
        if ( candidate == NO_CELL )
            return NO_CELL;

        if ( candidate == playerCell )
            continue;

        if ( Entities()->FindEntityAt( candidate ) != nullptr )
            continue;

        return candidate;
    }

    return NO_CELL;
}

int DungeonSystem::CountLivingEnemies() const
{
    int alive = 0;

    for ( Chaser const* chaser : Components< Chaser >()->GetComponents() )
    {
        Entity* owner = chaser->GetEntity();
        if ( owner == nullptr )
            continue;

        const Health* health = owner->GetComponent< Health >();
        if ( health == nullptr || health->IsAlive() )
            ++alive;
    }

    return alive;
}
