/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Component
* Description:
*          Implements the base class for all components in the Entity-Component-System architecture.
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef COMPONENT_H
#define COMPONENT_H

#include <pch.h>
#include <Core/ECS/Entity/Entity.h>

class Component
{

public:

//-----------------------------------------------------------------------------
// Public Destructor
//-----------------------------------------------------------------------------

    /// @brief virtual destructor
    virtual ~Component() {};

//-----------------------------------------------------------------------------
// Public Virtual Methods
//-----------------------------------------------------------------------------

    /// @brief called when this Component's Entity is added to the Scene
    virtual void OnInit() {};

    /// @brief  called when this Component's Entity is removed from the Scene
    virtual void OnExit() {};

    /// @brief  called every time after the Entity, this Component is attached to's hierarchy changes
    /// @param  previousParent   the parent of this Entity before the hierarchy change
    virtual void OnHierarchyChange( Entity* previousParent ) {};

    /// @brief  called whenever a child is added to this Entity
    /// @param  newChild    the child that was added
    virtual void OnAddChild( Entity* newChild ) {};

    /// @brief  called whenever a child is about to be removed from this Entity
    /// @param  newChild    the child that is about to be removed
    virtual void OnRemoveChild( Entity* newChild ) {};

    /// @brief Used by the Inspection System to display information about this Component
    virtual void Inspect() {};

    /// @brief virtual component clone function
    /// @return new clone of the component
    virtual Component* Clone() const = 0;

//-----------------------------------------------------------------------------
// Public Accessor Methods
//-----------------------------------------------------------------------------

    /// @brief  gets the component's type
    /// @return component type
    std::type_index GetType() const { return m_Type; }

    /// @brief  sets the parent entity of the component
    /// @param  entity  the parent entity of the component
    void SetEntity( Entity* entity ) { m_Parent = entity; }

    /// @brief returns the components parent entity
    /// @return the parent entity of the component
    Entity* GetEntity() const { return m_Parent; }

    /// @brief  gets the ID of this Component
    /// @return the ID of this Component
    unsigned GetId() const { return m_Id; }

    /// @brief  gets this Component's name
    /// @return this Component's name
    std::string GetName() const
    {
        return m_Parent->GetName() + "->" + PrefixlessName( m_Type );
    }

//-----------------------------------------------------------------------------
// Protected Constructors
//-----------------------------------------------------------------------------
protected:

    /// @brief default component constructor
    /// @param type The type of component this is.
    explicit Component (const std::type_index type ) :
        m_Type( type ),
        m_Parent( nullptr ),
        m_Id( GetUniqueId() )
    {}

    /// @brief copy constructor
    /// @param other the component to clone
    Component( Component const& other ) :
    m_Type( other.m_Type ),
    m_Parent( nullptr ),
    m_Id( GetUniqueId() )
    {}

//-----------------------------------------------------------------------------
// Private Member Variables
//-----------------------------------------------------------------------------
private:

    /// @brief  the type of this Component
    std::type_index const m_Type;

    /// @brief  the parent Entity of this Component
    Entity* m_Parent;

    /// @brief  the ID of this Component
    unsigned m_Id;
};

#endif //COMPONENT_H
