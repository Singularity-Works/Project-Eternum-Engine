/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Entity
* Description:
*
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef ENTITY_H
#define ENTITY_H

//-----------------------------------------------------------------------------
// Include Files:
//-----------------------------------------------------------------------------
#include <pch.h>

class Component;

class Entity {

public:
//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

    Entity();
    ~Entity();

//-----------------------------------------------------------------------------
// Public Engine Methods
//-----------------------------------------------------------------------------

    /// @brief  Initializes all components / children of this Entity
    /// @warning   This is mostly used by the Scene System to initialize all Entities in a Scene
    ///         and should not be called manually unless you know what you're doing.
    ///         This will call Init() on all Components and recursively on all children of this Entity
    ///         to ensure that all Entities in the hierarchy are initialized.
    void Init();

    /// @brief  exits all components / children of this Entity
    /// @warning  This is mostly used by the Scene System to exit all Entities in a Scene
    ///         and should not be called manually unless you know what you're doing.
    ///        This will call Exit() on all Components and recursively on all children of this Entity
    ///        to ensure that all Entities in the hierarchy are exited.
    void Exit();

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

    /// @brief  flags this Entity for destruction
    void Destroy();

    /// @brief  gets the component of the specified type from this Entity
    /// @tparam ComponentType   the type of component to get
    /// @return the component of the specified type (nullptr if component doesn't exist)
    template < typename ComponentType >
    ComponentType const* GetComponent() const;


    /// @brief  gets the component of the specified type from this Entity
    /// @tparam ComponentType   the type of component to get
    /// @return the component of the specified type (nullptr if component doesn't exist)
    template < typename ComponentType >
    ComponentType* GetComponent();


    /// @brief  gets all the components derived from the specified type from this Entity
    /// @tparam ComponentType   the type of component to get
    /// @return a vector of all components of the specified type
    template < typename ComponentType >
    std::vector< ComponentType* > GetComponentsOfType();


    /// @brief  sets the parent of this Entity
    /// @param  parent  the Entity that should be the parent of this one
    void SetParent( Entity* parent );

    /// @brief  Adds a Component to this Entity
    /// @param  component the Component to add to this Entity
    void AddComponent( Component* component );

    /// @brief  checks if this Entity is descended from another Entity
    /// @param  ancestor    The entity to check if this Entity is descended from
    /// @return whether this Entity is descended from the other Entity
    bool IsDescendedFrom( Entity const* ancestor ) const;

//-----------------------------------------------------------------------------
// Public Accessors
//-----------------------------------------------------------------------------

    /// @brief  gets all components in this Entity
    /// @return the map of all components in this Entity
    std::map< std::type_index, Component* >& getComponents();


    /// @brief  gets whether this Entity is flagged for destruction
    /// @return whether this Entity is flagged for destruction
    bool IsDestroyed() const;


    /// @brief  gets this Entity's name
    /// @return this Entity's name
    std::string const& GetName() const;

    /// @brief  sets this Entity's name
    /// @param  name    the new name for this Entity
    void SetName( std::string const& name );


    /// @brief  gets the ID of this Component
    /// @return the ID of this Component
    unsigned GetId() const;


    /// @brief  gets the parent of this Entity
    /// @return the parent of this Entity
    Entity const* GetParent() const;

    /// @brief  gets the parent of this Entity
    /// @return the parent of this Entity
    Entity* GetParent();


    /// @brief  gets the children of this Entity
    /// @return the children of this Entity
    std::vector< Entity const* > const& GetChildren() const;

    /// @brief  gets the children of this Entity
    /// @return the children of this Entity
    std::vector< Entity* > const& GetChildren();


    /// @brief  gets the number of descendants this Entity has
    /// @return the number of descendants this Entity has
    int GetNumDescendants() const;

//-----------------------------------------------------------------------------
// Public Copy Methods
//-----------------------------------------------------------------------------

    /// @brief  makes a copy of this Entity
    /// @return the new copy of this Entity
    Entity* Clone() const
    {
        Entity* clone = new Entity;
        *clone = *this;
        return clone;
    }


    /// @brief  copies all of another Entity's data and Components into this Entity
    /// @param  other   the entity to copy from
    void operator =( Entity const& other );


    /// @brief deletes the copy constructor to prevent copying
    /// @param other   the entity to copy from
    /// @note  this is deleted to prevent copying of Entities, which can lead to issues
    ///         with multiple Entities having the same ID and Components.
    ///         Use the Clone() method instead to create a new Entity with the same data.
    Entity( Entity const& other ) = delete;

//-----------------------------------------------------------------------------
// Private Member Variables
//-----------------------------------------------------------------------------
private:

    /// @brief  this Entity's name
    std::string m_Name = "";

    /// @brief  container of components attached to this Entity
    std::map< std::type_index, Component* > m_Components = {};

    /// @brief  the ID of this Entity
    unsigned m_Id = -1;

    /// @brief  the children of this Entity
    std::vector< Entity* > m_Children = {};

    /// @brief  the number of descendants this Entity has
    int m_NumDescendants = 0;

    /// @brief  the parent Entity of this Entity
    Entity* m_Parent = nullptr;

    /// @brief  flag of whether this Entity should be destroyed
    bool m_IsDestroyed = false;

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

    /// @brief  adds a child to this Enity
    /// @param  child   - the child to add to this Enitity
    void addChild( Entity* child );

    /// @brief  removes a child from this Enity
    /// @param  child   - the child to remove from this Enitity
    void removeChild( const Entity* child );



};


#endif //ENTITY_H
