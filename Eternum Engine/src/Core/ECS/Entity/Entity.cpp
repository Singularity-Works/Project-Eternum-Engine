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
#include <Core/ECS/Component/Component.h>

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

Entity::Entity() :
    m_Id(GetUniqueId())
{}

Entity::~Entity()
{
    for (auto& [type, component] : m_Components)
        delete component;

    for (Entity* child : m_Children)
        delete child;
}

//-----------------------------------------------------------------------------
// Engine Lifecycle Methods
//-----------------------------------------------------------------------------

void Entity::Init()
{
    for ( auto& [ type, component ] : m_Components )
    {
        component->OnInit();
    }
}

void Entity::Exit()
{
    for ( auto& [ type, component ] : m_Components )
        component->OnExit();

    if (m_Parent)
        m_Parent->removeChild(this);
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
    m_Parent = parent;
}

void Entity::AddComponent(Component* component)
{
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
// Template Methods
//-----------------------------------------------------------------------------

template < typename ComponentType >
ComponentType const* Entity::GetComponent() const
{
    auto componentIterator = m_Components.find( typeid( ComponentType ) );
    if ( componentIterator != m_Components.end() )
    {
        return static_cast< ComponentType* >( componentIterator->second );
    }

    // if exact type not found, fall back to searching for a derived type
    for ( auto const& [ type, component ] : m_Components )
    {
        ComponentType const* found = dynamic_cast< ComponentType const* >( component );
        if ( found != nullptr )
        {
            return found;
        }
    }

    // if no derived component found, return nullptr
    return nullptr;
}

template <typename ComponentType>
ComponentType* Entity::GetComponent()
{
    return const_cast< ComponentType* >( const_cast< Entity const* >( this )->GetComponent< ComponentType >() );
}

template <typename ComponentType>
std::vector<ComponentType*> Entity::GetComponentsOfType()
{
    std::vector<ComponentType*> result;
    for (auto& [type, comp] : m_Components)
    {
        if (auto* match = dynamic_cast<ComponentType*>(comp))
            result.push_back(match);
    }
    return result;
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
}

void Entity::removeChild(const Entity* child)
{
    const auto it = std::find( m_Children.begin(), m_Children.end(), child );
    if ( it == m_Children.end() )
    {
        std::cout << "ERROR: cannot find child \"" << child->GetName() << "\" to remove" << std::endl;
        return;
    }
    m_Children.erase( it );
}

// Explicit template instantiations
template Component const* Entity::GetComponent<Component>() const;
template Component* Entity::GetComponent<Component>();
template std::vector<Component*> Entity::GetComponentsOfType<Component>();
