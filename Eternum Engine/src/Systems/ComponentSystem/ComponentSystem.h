/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: ComponentSystem
* Description:
*
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef COMPONENTSYSTEM_H
#define COMPONENTSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Entity/Entity.h>
template<typename ComponentType>
class ComponentSystem;

template<typename ComponentType>
ComponentSystem<ComponentType>* Components();


template< class ComponentType >
class ComponentSystem : public System
{

    // -----------------------------------------------------------------------------
    // Friend Declarations
    // -----------------------------------------------------------------------------
    template<typename U>
    friend ComponentSystem<U>* ::Components();


public:
    void Init() override
    {
        System::Init();
        LogVerbose( "  holding " + std::to_string( GetComponents().size() ) + " components" );
    }
    void Update(double deltaTime) override
    {
        // walk a copy, a Component may add or remove others while it runs
        std::vector< ComponentType* > components = m_Components;
        for ( ComponentType* component : components )
            component->OnUpdate( deltaTime );
    }

    void FixedUpdate() override
    {
        std::vector< ComponentType* > components = m_Components;
        for ( ComponentType* component : components )
            component->OnFixedUpdate();
    }

    void Render() override
    {
        std::vector< ComponentType* > components = m_Components;
        for ( ComponentType* component : components )
            component->OnRender();
    }


//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------


    /// @brief  gets the array of components in this ComponentSystem
    /// @return the array of Components in this ComponentSystem
    std::vector< ComponentType* > const& GetComponents() const
    {
        return m_Components;
    }

    /// @brief  adds a component to the ComponentSystem
    /// @param  component   the component to add
    void AddComponent( ComponentType* component )
    {
        m_Components.push_back( component );
    }

    /// @brief  removes a component to the ComponentSystem
    /// @param  component   the component to remove
    void RemoveComponent( ComponentType* component )
    {
        const auto it = std::find( m_Components.begin(), m_Components.end(), component );
        if ( it == m_Components.end() )
            return;

        m_Components.erase( it );
    }


protected:
    explicit ComponentSystem( std::string const& name ) :
    System( name )
    {}

private:

    ComponentSystem() :
     System( "ComponentSystem<" + PrefixlessName( typeid( ComponentType ) ) + ">" )
    {}

    //-----------------------------------------------------------------------------
    // Singleton Functionality
    //-----------------------------------------------------------------------------

    /// @brief  Gets the singleton instance of the ComponentSystem
    /// @tparam ComponentType   the type of component this system manages
    static ComponentSystem< ComponentType > * GetInstance()
    {
        static std::unique_ptr<ComponentSystem< ComponentType >> s_Instance = nullptr;

        if ( !s_Instance )
        {
            s_Instance.reset(new ComponentSystem<ComponentType>());

            // a ComponentSystem joins the engine the first time anything asks for it
            SystemRegistry::Instance().Register( s_Instance.get() );
        }

        return s_Instance.get();
    }

    // Prevent copy construction and assignment
    ComponentSystem( const ComponentSystem< ComponentType > & ) = delete;
    void operator=( const ComponentSystem< ComponentType > & ) = delete;

//-----------------------------------------------------------------------------
// Private Members
//-----------------------------------------------------------------------------

    std::vector < ComponentType* > m_Components = {};

};

template< class ComponentType >
inline ComponentSystem< ComponentType >* Components()
{
    return ComponentSystem< ComponentType >::GetInstance();
}

//-----------------------------------------------------------------------------
// Component Base
//-----------------------------------------------------------------------------

/// @brief  Base class that wires a Component into the ComponentSystem for its own type.
///         Derive from this instead of Component and the registration, the type tag
///         and the Clone are all handled for you.
/// @tparam Derived the concrete Component type, for example Transform
template < class Derived >
class ComponentOf : public Component
{

public:

    /// @brief  adds this Component to ComponentSystem< Derived >
    void AddToSystem() override
    {
        Components< Derived >()->AddComponent( static_cast< Derived* >( this ) );
    }

    /// @brief  removes this Component from ComponentSystem< Derived >
    void RemoveFromSystem() override
    {
        Components< Derived >()->RemoveComponent( static_cast< Derived* >( this ) );
    }

    /// @brief  copies this Component through Derived's copy constructor
    /// @return new clone of the component
    Component* Clone() const override
    {
        return new Derived( *static_cast< Derived const* >( this ) );
    }

protected:

    /// @brief  default constructor, tags the Component with its concrete type
    ComponentOf() :
        Component( typeid( Derived ) )
    {}

    /// @brief  copy constructor
    /// @param  other   the component to copy from
    ComponentOf( ComponentOf const& other ) :
        Component( other )
    {}

};

#endif //COMPONENTSYSTEM_H
