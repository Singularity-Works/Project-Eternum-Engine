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
        std::cout << "Number of Components in system: " <<  GetComponents().size() << "\n";
    }
    void Update(double deltaTime) override {};
    void FixedUpdate() override {};
    void Render() override {};


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
        m_Components.erase( std::find( m_Components.begin(), m_Components.end(), component ) );
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

REGISTER_COMPONENT_SYSTEM(Component)
#endif //COMPONENTSYSTEM_H
