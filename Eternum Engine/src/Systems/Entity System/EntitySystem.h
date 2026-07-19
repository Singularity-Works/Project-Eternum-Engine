/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EntitySystem
* Description:
*     Owns every Entity in the Scene. Entities added here are initialized, which is what
*     puts their Components into the matching ComponentSystems, and are deleted on the
*     frame after they are flagged for destruction.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Core/ECS/Entity/Entity.h>
#include <nlohmann/json_fwd.hpp>

class EntitySystem final : public System
{

public:

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    void Init() override;
    void Update( double deltaTime ) override;
    void FixedUpdate() override;
    void Render() override;
    void Shutdown() override;

    // ----------------------------------------------------------------
    // Scene Management
    // ----------------------------------------------------------------

    /// @brief  adds a root Entity to the Scene and initializes it
    /// @param  entity  the Entity to add, the Scene takes ownership of it
    /// @note   an Entity that already has a parent is owned by that parent, do not add it here
    void AddEntity( Entity* entity );

    /// @brief  creates a named Entity that is already in the Scene
    /// @param  name    the name to give the new Entity
    /// @return the new Entity, owned by the Scene
    Entity* CreateEntity( std::string const& name = "" );

    /// @brief  finds an Entity anywhere in the Scene by name
    /// @param  name    the name to look for
    /// @return the first Entity with that name, or nullptr
    Entity* FindEntity( std::string const& name ) const;

    /// @brief  finds an Entity standing on a cell
    /// @param  cell    the cell to look at
    /// @param  ignore  an Entity to skip, normally the one doing the looking
    /// @return the first Entity found there, or nullptr
    /// @note   only Entities with a Transform can be standing anywhere
    Entity* FindEntityAt( Vec2i const& cell, Entity const* ignore = nullptr ) const;

    /// @brief  removes and deletes every Entity in the Scene
    void ClearEntities();

    /// @brief  writes the whole Scene so it can be saved
    /// @param  data    the object to write into
    void WriteScene( nlohmann::json& data ) const;

    /// @brief  throws the Scene away and rebuilds it from a save
    /// @param  data    the object to read from
    void ReadScene( nlohmann::json const& data );

    // ----------------------------------------------------------------
    // Accessors
    // ----------------------------------------------------------------

    /// @brief  gets the root Entities in the Scene
    /// @return the root Entities in the Scene
    std::vector< Entity* > const& GetEntities() const { return m_Entities; }

    /// @brief  gets how many Entities are in the Scene, children included
    /// @return the total Entity count
    int GetEntityCount() const;

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< EntitySystem > GetInstance()
    {
        static std::shared_ptr< EntitySystem > instance( new EntitySystem() );
        return instance;
    }

private:

    EntitySystem(); // Private constructor

    /// @brief  deletes any destroyed Entity found below this one
    /// @param  entity  the Entity to search under
    static void destroyFlaggedChildren( Entity* entity );

    /// @brief  searches an Entity and everything under it for a name
    /// @param  entity  the Entity to search from
    /// @param  name    the name to look for
    /// @return the matching Entity, or nullptr
    static Entity* findInBranch( Entity* entity, std::string const& name );

    /// @brief  searches an Entity and everything under it for one standing on a cell
    /// @param  entity  the Entity to search from
    /// @param  cell    the cell to look at
    /// @param  ignore  an Entity to skip
    /// @return the matching Entity, or nullptr
    static Entity* findInBranchAt( Entity* entity, Vec2i const& cell, Entity const* ignore );

    /// @brief  counts an Entity and everything under it
    /// @param  entity  the Entity to count from
    /// @return the number of Entities in the branch
    static int countBranch( Entity const* entity );

    /// @brief  the root Entities in the Scene, this System owns them
    std::vector< Entity* > m_Entities;

};

// Static EntitySystem instance call
inline EntitySystem* Entities()
{
    return EntitySystem::GetInstance().get();
}

// Register the EntitySystem with the SystemRegistry
REGISTER_SYSTEM(EntitySystem)

#endif //ENTITYSYSTEM_H
