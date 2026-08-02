/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: DungeonSystem.h
* Description:
*     Picks a generation algorithm, runs it, and sends the result to the GridSystem.
*     The algorithms themselves live under Generation, this only chooses between them.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef DUNGEONSYSTEM_H
#define DUNGEONSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Systems/System Registry/SystemRegistry.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Dungeon System/Generation/DungeonGenerator.h>
#include <nlohmann/json_fwd.hpp>

class DungeonSystem final : public System
{

public:

    /// @brief  width height pair borrowed from the grid
    using Dimension = GridSystem::Dimension;

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    void Init() override;
    void Shutdown() override;
    void Update( double deltaTime ) override;
    void FixedUpdate() override;
    void Render() override;

    // ----------------------------------------------------------------
    // Generation
    // ----------------------------------------------------------------

    /// @brief  builds a dungeon with a fresh seed, sends it to the grid and places the player
    void StartNewDungeon();

    /// @brief  builds a dungeon from a known seed
    /// @param  seed    the seed to build from, the same seed always gives the same dungeon
    void StartNewDungeon( unsigned seed );

    /// @brief  switches to a different generation algorithm
    /// @param  index   which generator to use, out of range values are ignored
    /// @note   rebuilds straight away once the engine is running, before that it just
    ///         decides what the first dungeon will be built with
    void SetGenerator( std::size_t index );

    /// @brief  finds a generator by its short key, for example "bsp" or "cave"
    /// @param  name    the key to look for, case does not matter
    /// @return the index of the generator, or the count when nothing matched
    std::size_t FindGenerator( std::string const& name ) const;

    /// @brief  fixes the seed the first dungeon is built from
    /// @param  seed    the seed to use
    /// @note   call before the engine starts, this is what --seed sets
    void SetStartupSeed( unsigned seed );

    /// @brief  hands the current map to the GridSystem
    /// @param  mapName the name to store it under
    void SendToGridSystem( const std::string& mapName );

    /// @brief  writes the current map and how it was made
    /// @param  data    the object to write into
    void WriteState( nlohmann::json& data ) const;

    /// @brief  restores a map from a save, without regenerating it
    /// @param  data    the object to read from
    /// @note   the map is stored cell by cell rather than regenerated, so a save survives
    ///         a change to the generators
    void ReadState( nlohmann::json const& data );

    // ----------------------------------------------------------------
    // Queries
    // ----------------------------------------------------------------

    /// @brief  a sensible place to drop something, a room centre when there are rooms
    /// @return a walkable cell, or (-1, -1) when the map has no open cells at all
    Vec2i GetRandomSpawnPoint();

    /// @brief  the middle of a random room
    /// @return a room centre, or (-1, -1) when there are no rooms
    Vec2i GetRandomRoomCenter();

    /// @brief  a random cell inside a room, never on its wall
    /// @param  room    the room to pick inside
    /// @return a cell inside the room
    Vec2i GetRandomTileInRoom( Room const& room );

    /// @brief  a random cell on the edge of a random room
    /// @return a cell on a room edge, or (-1, -1) when there are no rooms
    Vec2i GetRandomRoomEdge();

    /// @brief  gets the rooms in the current dungeon, empty for cave layouts
    /// @return the rooms in the current dungeon
    std::vector< Room > const& GetRooms() const { return m_Layout.m_Rooms; }

    /// @brief  gets the current map
    /// @return the current map
    GridSystem::Grid const& GetCurrentGrid() const { return m_Layout.m_Grid; }

    /// @brief  gets the name of the algorithm in use
    /// @return the name of the algorithm in use
    std::string GetGeneratorName() const;

    /// @brief  gets how many algorithms are available
    /// @return how many algorithms are available
    std::size_t GetGeneratorCount() const { return m_Generators.size(); }

    /// @brief  gets the seed the current dungeon was built from
    /// @return the seed the current dungeon was built from
    unsigned GetSeed() const { return m_Seed; }

    // ----------------------------------------------------------------
    // Settings
    // ----------------------------------------------------------------

    /// @brief  the size every generated dungeon is built at
    static constexpr int DUNGEON_WIDTH  = 60;
    static constexpr int DUNGEON_HEIGHT = 22;

    /// @brief  how many chasing Entities a dungeon gets
    static constexpr int ENEMY_COUNT = 3;

    /// @brief  how far from the player an enemy should start, if the map allows it
    /// @note   close enough that they find you soon, far enough that a dungeon opens quiet
    static constexpr int ENEMY_SPAWN_DISTANCE = 9;

    /// @brief  how far an enemy can see
    /// @note   this is walking distance, and a dungeon is mostly walls, so the range that
    ///         actually matters after line of sight is a lot shorter than this
    static constexpr int ENEMY_SIGHT_RANGE = 18;

    /// @brief  how many enemies are still standing
    /// @return the count of living Chasers
    int CountLivingEnemies() const;

    // ----------------------------------------------------------------
    // Combat Balance
    // ----------------------------------------------------------------

    static constexpr int    PLAYER_HEALTH          = 20;
    static constexpr int    PLAYER_DAMAGE          = 4;
    static constexpr double PLAYER_ATTACK_INTERVAL = 0.35;

    static constexpr int    ENEMY_HEALTH           = 10;
    static constexpr int    ENEMY_DAMAGE           = 2;
    static constexpr double ENEMY_ATTACK_INTERVAL  = 0.9;

    /// @brief  how many traps and pickups a dungeon gets
    static constexpr int TRAP_COUNT   = 6;
    static constexpr int PICKUP_COUNT = 4;

    static constexpr int TRAP_DAMAGE     = 5;
    static constexpr int PICKUP_RESTORES = 6;

    /// @brief  how often an enemy bite poisons, and what that costs
    static constexpr int    POISON_CHANCE   = 35;
    static constexpr int    POISON_DAMAGE   = 1;
    static constexpr double POISON_DURATION = 3.0;

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< DungeonSystem > GetInstance()
    {
        static std::shared_ptr< DungeonSystem > instance( new DungeonSystem() );
        return instance;
    }

private:

    DungeonSystem(); // Private constructor

    /// @brief  creates the chasing Entities if they do not exist, then places them
    /// @note   same story as spawnPlayer, this belongs in a game layer eventually
    void spawnEnemies( Vec2i const& playerCell );

    /// @brief  scatters traps and loot around the dungeon
    /// @param  playerCell  where the player is standing, nothing spawns on top of them
    void spawnProps( Vec2i const& playerCell );

    /// @brief  picks a cell for an enemy, preferring one well away from the player
    /// @param  playerCell  where the player is standing
    /// @return a walkable cell
    Vec2i pickEnemyCell( Vec2i const& playerCell );

    /// @brief  picks an empty cell for a trap or a pickup
    /// @param  playerCell  where the player is, nothing spawns underfoot
    /// @return a free walkable cell, or (-1, -1) when none could be found
    Vec2i pickPropCell( Vec2i const& playerCell );

    /// @brief  creates the player Entity if it does not exist, then places it on the map
    /// @note   this lives here for now because the DungeonSystem is what knows where the
    ///         open cells are, it should move to a game layer once there is one
    Vec2i spawnPlayer();

    /// @brief  the algorithms available, in the order the number keys select them
    std::vector< std::unique_ptr< DungeonGenerator > > m_Generators;

    /// @brief  which algorithm is in use
    std::size_t m_GeneratorIndex = 0;

    /// @brief  the current map and its rooms
    DungeonLayout m_Layout;

    /// @brief  the source every generation and placement draw comes from
    /// @note   reseeded per dungeon, which is what makes a seed reproduce the spawn
    ///         point as well as the map
    Random m_Rng;

    /// @brief  the seed the current dungeon was built from
    unsigned m_Seed = 0;

    /// @brief  the seed the first dungeon should use, when one was asked for
    unsigned m_StartupSeed = 0;

    /// @brief  whether a startup seed was asked for
    bool m_HasStartupSeed = false;

    /// @brief  whether the engine has built its first dungeon yet
    bool m_IsStarted = false;

};

// Register the DungeonSystem with the SystemRegistry
REGISTER_SYSTEM(DungeonSystem)

#endif // DUNGEONSYSTEM_H
