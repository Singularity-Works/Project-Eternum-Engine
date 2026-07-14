/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Entity
* Description:
*     Implements the Entity class functionality in the ECS architecture.
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Entity.h"

#include <ranges>
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Component/ComponentFactory.h>
#include <nlohmann/json.hpp>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Entity::Entity() :
    m_Id(GetUniqueId())
{}

Entity::~Entity()
{
    // Components must leave their Systems before they are freed
    if ( m_IsInitialized )
        Exit();

    // stop the parent tracking a pointer that is about to dangle
    if ( m_Parent )
        m_Parent->removeChild( this );
    m_Parent = nullptr;

    // take the children first so their own detach cannot mutate what we walk
    std::vector< Entity* > children;
    children.swap( m_Children );
    for ( Entity* child : children )
    {
        child->m_Parent = nullptr;
        delete child;
    }

    for ( auto& [ type, component ] : m_Components )
        delete component;
    m_Components.clear();
}

//-----------------------------------------------------------------------------
// Engine Lifecycle Methods
//-----------------------------------------------------------------------------

void Entity::Init()
{
    if ( m_IsInitialized )
        return;

    m_IsInitialized = true;

    for ( auto& [ type, component ] : m_Components )
    {
        component->AddToSystem();
        component->OnInit();
    }

    // walk a copy, a child may reparent itself while initializing
    std::vector< Entity* > children = m_Children;
    for ( Entity* child : children )
        child->Init();
}

void Entity::Exit()
{
    if ( !m_IsInitialized )
        return;

    std::vector< Entity* > children = m_Children;
    for ( Entity* child : children )
        child->Exit();

    for ( auto& [ type, component ] : m_Components )
    {
        component->OnExit();
        component->RemoveFromSystem();
    }

    m_IsInitialized = false;
}

//-----------------------------------------------------------------------------
// Public Copy Methods
//-----------------------------------------------------------------------------

void Entity::operator=(Entity const& other)
{
    m_Name = other.m_Name;
    m_IsDestroyed = false;

    for ( auto& [ type, component ] : other.m_Components )
    {
        AddComponent( component->Clone() );
    }

    for ( Entity const* child : other.m_Children )
    {
        child->Clone()->SetParent( this );
    }
}


//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void Entity::Destroy()
{
    m_IsDestroyed = true;

    for ( Entity* child : m_Children )
    {
        child->Destroy();
    }
}

void Entity::SetParent(Entity* parent)
{
    if ( parent == this )
    {
        std::cout << "WARNING: Entity \"" << m_Name << "\" cannot be its own parent" << std::endl;
        return;
    }

    // reparenting under our own descendant would build a cycle
    if ( parent != nullptr && parent->IsDescendedFrom( this ) )
    {
        std::cout << "WARNING: cannot parent Entity \"" << m_Name << "\" to its own descendant" << std::endl;
        return;
    }

    if ( m_Parent == parent )
        return;

    Entity* previousParent = m_Parent;

    if ( previousParent != nullptr )
        previousParent->removeChild( this );

    m_Parent = parent;

    if ( parent != nullptr )
    {
        parent->addChild( this );

        // joining a branch that is already in the Scene brings this Entity in with it
        if ( parent->IsInitialized() && !m_IsInitialized )
            Init();
    }

    for ( auto& [ type, component ] : m_Components )
        component->OnHierarchyChange( previousParent );
}

void Entity::AddComponent(Component* component)
{
    if ( component == nullptr )
        return;

    // Check if the component already exists.
    if ( m_Components.find( component->GetType() ) != m_Components.end() )
    {
        std::cout << "WARNING: attempting to add a duplicate component to the Entity \"" << m_Name << "\"" << std::endl;
        return;
    }

    // Set the component's parent as this entity
    component->SetEntity( this );

    // add it to the entity.
    m_Components[ component->GetType() ] = component;

    // an Entity already in the Scene brings the Component in right away
    if ( m_IsInitialized )
    {
        component->AddToSystem();
        component->OnInit();
    }
}

void Entity::RemoveComponent(Component* component)
{
    if ( component == nullptr )
        return;

    const auto it = m_Components.find( component->GetType() );
    if ( it == m_Components.end() || it->second != component )
    {
        std::cout << "WARNING: cannot find component \"" << component->GetName() << "\" to remove" << std::endl;
        return;
    }

    if ( m_IsInitialized )
    {
        component->OnExit();
        component->RemoveFromSystem();
    }

    m_Components.erase( it );
    delete component;
}

bool Entity::IsDescendedFrom(Entity const* ancestor) const
{
    Entity const* current = m_Parent;
    while (current)
    {
        if (current == ancestor)
            return true;
        current = current->m_Parent;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Entity::Write(nlohmann::json& data) const
{
    data[ "name" ] = m_Name;

    nlohmann::json components = nlohmann::json::object();
    for ( auto const& [ type, component ] : m_Components )
    {
        nlohmann::json fields = nlohmann::json::object();
        component->Write( fields );
        components[ component->GetTypeName() ] = fields;
    }
    data[ "components" ] = components;

    nlohmann::json children = nlohmann::json::array();
    for ( Entity const* child : m_Children )
    {
        nlohmann::json entry = nlohmann::json::object();
        child->Write( entry );
        children.push_back( entry );
    }
    data[ "children" ] = children;
}

void Entity::Read(nlohmann::json const& data)
{
    m_Name = data.value( "name", std::string() );

    // start from nothing, a load replaces rather than merges
    for ( auto& [ type, component ] : m_Components )
    {
        if ( m_IsInitialized )
        {
            component->OnExit();
            component->RemoveFromSystem();
        }
        delete component;
    }
    m_Components.clear();

    if ( data.contains( "components" ) )
    {
        for ( auto const& [ name, fields ] : data[ "components" ].items() )
        {
            Component* component = ComponentFactory::Instance().Create( name );

            if ( component == nullptr )
            {
                // an unknown type means an older or newer build wrote this file
                std::cout << "WARNING: save refers to unknown component \"" << name << "\"" << std::endl;
                continue;
            }

            component->Read( fields );
            AddComponent( component );
        }
    }

    std::vector< Entity* > existing;
    existing.swap( m_Children );
    for ( Entity* child : existing )
    {
        child->m_Parent = nullptr;
        delete child;
    }

    if ( data.contains( "children" ) )
    {
        for ( nlohmann::json const& entry : data[ "children" ] )
        {
            Entity* child = new Entity();
            child->Read( entry );
            child->SetParent( this );
        }
    }
}

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

std::map<std::type_index, Component*>& Entity::getComponents()
{
    return m_Components;
}

bool Entity::IsDestroyed() const
{
    return m_IsDestroyed;
}

bool Entity::IsInitialized() const
{
    return m_IsInitialized;
}

std::string const& Entity::GetName() const
{
    return m_Name;
}

void Entity::SetName(std::string const& name)
{
    m_Name = name;
}

unsigned Entity::GetId() const
{
    return m_Id;
}

Entity const* Entity::GetParent() const
{
    return m_Parent;
}

Entity* Entity::GetParent()
{
    return m_Parent;
}

std::vector<Entity const*> const& Entity::GetChildren() const
{
    return reinterpret_cast< std::vector< Entity const* > const& >( m_Children );
}

std::vector<Entity*> const& Entity::GetChildren()
{
    return m_Children;
}

int Entity::GetNumDescendants() const
{
    return m_NumDescendants;
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

void Entity::addChild(Entity* child)
{
    m_Children.push_back( child );

    // the child and everything under it joined this branch
    const int added = child->m_NumDescendants + 1;
    for ( Entity* ancestor = this; ancestor != nullptr; ancestor = ancestor->m_Parent )
        ancestor->m_NumDescendants += added;

    for ( auto& [ type, component ] : m_Components )
        component->OnAddChild( child );
}

void Entity::removeChild(const Entity* child)
{
    const auto it = std::find( m_Children.begin(), m_Children.end(), child );
    if ( it == m_Children.end() )
    {
        std::cout << "ERROR: cannot find child \"" << child->GetName() << "\" to remove" << std::endl;
        return;
    }

    for ( auto& [ type, component ] : m_Components )
        component->OnRemoveChild( *it );

    const int removed = child->m_NumDescendants + 1;
    for ( Entity* ancestor = this; ancestor != nullptr; ancestor = ancestor->m_Parent )
        ancestor->m_NumDescendants -= removed;

    m_Children.erase( it );
}


