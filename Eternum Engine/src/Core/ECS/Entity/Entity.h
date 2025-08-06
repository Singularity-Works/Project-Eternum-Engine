/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Entity
* Description:
*     TODO: Describe the purpose of this file.
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


class Component;

class Entity {

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------
public:

    Entity();
    ~Entity();



    /// @brief  gets the name of the Entity
    /// @return the name of the Entity
    std::string const& GetName() const
    {
        return m_Name;
    };

    void SetName( std::string const& name )
    {
        m_Name = name;
    }

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
};



#endif //ENTITY_H
