/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EntitySystem
* Description:
*     Owns every Entity in the Scene.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "EntitySystem.h"
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <nlohmann/json.hpp>

EntitySystem::EntitySystem()
    : System( "Entity System" )
{}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void EntitySystem::Init()
{
    System::Init();
}

void EntitySystem::Update( double deltaTime )
{
    // destruction is deferred to here so nothing dies mid frame
    for ( std::size_t i = 0; i < m_Entities.size(); )
    {
        Entity* entity = m_Entities[ i ];

        if ( entity->IsDestroyed() )
        {
            m_Entities.erase( m_Entities.begin() + static_cast< std::ptrdiff_t >( i ) );
            delete entity;
            continue;
        }

        destroyFlaggedChildren( entity );
        ++i;
    }
}

void EntitySystem::FixedUpdate()
{
}

void EntitySystem::Render()
{
}

void EntitySystem::Shutdown()
{
    ClearEntities();
    System::Shutdown();
}

// ----------------------------------------------------------------
// Scene Management
// ----------------------------------------------------------------

void EntitySystem::AddEntity( Entity* entity )
{
    if ( entity == nullptr )
        return;

    if ( entity->GetParent() != nullptr )
    {
        std::cout << "WARNING: Entity \"" << entity->GetName()
                  << "\" has a parent and is already owned by it" << std::endl;
        return;
    }

    if ( std::find( m_Entities.begin(), m_Entities.end(), entity ) != m_Entities.end() )
    {
        std::cout << "WARNING: Entity \"" << entity->GetName()
                  << "\" is already in the Scene" << std::endl;
        return;
    }

    m_Entities.push_back( entity );

    // this is what puts the Entity's Components into their ComponentSystems
    entity->Init();
}

Entity* EntitySystem::CreateEntity( std::string const& name )
{
    Entity* entity = new Entity();
    entity->SetName( name );
    AddEntity( entity );
    return entity;
}

Entity* EntitySystem::FindEntity( std::string const& name ) const
{
    for ( Entity* entity : m_Entities )
    {
        if ( Entity* found = findInBranch( entity, name ) )
            return found;
    }

    return nullptr;
}

Entity* EntitySystem::FindEntityAt( Vec2i const& cell, Entity const* ignore ) const
{
    for ( Entity* entity : m_Entities )
    {
        if ( Entity* found = findInBranchAt( entity, cell, ignore ) )
            return found;
    }

    return nullptr;
}

void EntitySystem::ClearEntities()
{
    // take the list first, each delete unhooks itself as it goes
    std::vector< Entity* > entities;
    entities.swap( m_Entities );

    for ( Entity* entity : entities )
        delete entity;
}

void EntitySystem::WriteScene( nlohmann::json& data ) const
{
    nlohmann::json entities = nlohmann::json::array();

    for ( Entity const* entity : m_Entities )
    {
        nlohmann::json entry = nlohmann::json::object();
        entity->Write( entry );
        entities.push_back( entry );
    }

    data[ "entities" ] = entities;
}

void EntitySystem::ReadScene( nlohmann::json const& data )
{
    ClearEntities();

    if ( !data.contains( "entities" ) || !data[ "entities" ].is_array() )
        return;

    for ( nlohmann::json const& entry : data[ "entities" ] )
    {
        // build it fully before it joins the Scene, so Components enter their Systems once
        Entity* entity = new Entity();
        entity->Read( entry );
        AddEntity( entity );
    }
}

// ----------------------------------------------------------------
// Accessors
// ----------------------------------------------------------------

int EntitySystem::GetEntityCount() const
{
    int count = 0;
    for ( Entity const* entity : m_Entities )
        count += countBranch( entity );

    return count;
}

// ----------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------

void EntitySystem::destroyFlaggedChildren( Entity* entity )
{
    // walk a copy, deleting a child edits the list we are walking
    std::vector< Entity* > children = entity->GetChildren();

    for ( Entity* child : children )
    {
        if ( child->IsDestroyed() )
        {
            // the destructor exits the Entity and unhooks it from its parent
            delete child;
            continue;
        }

        destroyFlaggedChildren( child );
    }
}

Entity* EntitySystem::findInBranchAt( Entity* entity, Vec2i const& cell, Entity const* ignore )
{
    if ( entity != ignore )
    {
        if ( const Transform* transform = entity->GetComponent< Transform >() )
        {
            Vec2f const& position = transform->GetTranslation();

            if ( static_cast< int >( position.x() ) == cell.x()
              && static_cast< int >( position.y() ) == cell.y() )
            {
                return entity;
            }
        }
    }

    for ( Entity* child : entity->GetChildren() )
    {
        if ( Entity* found = findInBranchAt( child, cell, ignore ) )
            return found;
    }

    return nullptr;
}

Entity* EntitySystem::findInBranch( Entity* entity, std::string const& name )
{
    if ( entity->GetName() == name )
        return entity;

    for ( Entity* child : entity->GetChildren() )
    {
        if ( Entity* found = findInBranch( child, name ) )
            return found;
    }

    return nullptr;
}

int EntitySystem::countBranch( Entity const* entity )
{
    int count = 1;
    for ( Entity const* child : entity->GetChildren() )
        count += countBranch( child );

    return count;
}
