/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Chaser
* Description:
*     An Entity that wanders the dungeon until it catches sight of the player, then hunts.
*
*     Three states. Roaming picks a random reachable cell and walks to it. Hunting routes
*     straight at the player and moves faster. Searching is what happens when the player
*     breaks line of sight, it walks to where they were last seen before giving up and
*     going back to roaming.
*
*     All of it is real time, paced by a cooldown rather than by the player taking turns.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef CHASER_H
#define CHASER_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>
#include <Systems/Pathfinding/Pathfinding.h>

/// @brief  what a Chaser is currently doing
enum class ChaserState
{
    Roaming,    // no idea where the player is, wandering
    Hunting,    // can see the player right now
    Searching   // lost sight, heading for where they were
};

class Chaser final : public ComponentOf< Chaser >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Chaser();

    /// @brief  destructor
    ~Chaser() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  looks around, then steps when the cooldown allows it
    /// @param  deltaTime   seconds since the last frame
    void OnUpdate( double deltaTime ) override;

    /// @brief  drops this Entity's trail when it leaves the Scene
    void OnExit() override;

    /// @brief  Used by the Inspection System to display information about this Component
    void Inspect() override;

    /// @brief  writes this Component's state so it can be saved
    /// @param  data    the object to write into
    void Write( nlohmann::json& data ) const override;

    /// @brief  reads this Component's state back out of a save
    /// @param  data    the object to read from
    void Read( nlohmann::json const& data ) override;


    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  decides what this Entity should be doing based on what it can see
    /// @note   called by OnUpdate, exposed so a test can drive it a step at a time
    void LookAround();

    /// @brief  routes to whatever this Entity is heading for and takes one step
    /// @return whether a step was taken
    bool TakeStep();

    /// @brief  forgets everything and goes back to roaming
    /// @note   called when a new dungeon is generated
    void Reset();

    /// @brief  whether this Entity can see its target right now
    /// @return whether the target is in range and in line of sight
    bool CanSeeTarget() const;

    /// @brief  swings at the target when it is close enough to reach
    /// @return how much damage landed, zero when it could not reach or was not ready
    int TryAttackTarget();

    /// @brief  whether this Entity is able to act at all
    /// @return false when it is dead or stunned
    bool CanAct() const;

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets what this Entity is currently doing
    /// @return the current state
    ChaserState GetState() const { return m_State; }

    /// @brief  gets the route worked out on the last step
    /// @return the last route, invalid when nowhere could be reached
    Path const& GetPath() const { return m_Path; }

    /// @brief  gets where this Entity is heading
    /// @return the destination cell, or (-1, -1) when it has none
    Vec2i const& GetDestination() const { return m_Destination; }

    /// @brief  gets how far this Entity can see
    /// @return the range in cells
    int GetSightRange() const { return m_SightRange; }

    /// @brief  sets how far this Entity can see
    /// @param  range   the new range in cells
    void SetSightRange( const int range ) { m_SightRange = range; }

    /// @brief  gets the name of the Entity this one hunts
    /// @return the target's name
    std::string const& GetTargetName() const { return m_TargetName; }

    /// @brief  sets the name of the Entity this one hunts
    /// @param  name    the target's name
    void SetTargetName( std::string const& name ) { m_TargetName = name; }

    /// @brief  the character this Entity is drawn as in each state
    static char GetSymbolFor( ChaserState state );

    /// @brief  the readable name of a state
    /// @param  state   the state to name
    /// @return its name
    static std::string GetStateName( ChaserState state );

    /// @brief  turns a name from GetStateName back into a state
    /// @param  name    the name to look up
    /// @return the matching state, or Roaming when the name is not known
    static ChaserState GetStateFromName( std::string const& name );

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    /// @brief  how long between steps while hunting, faster than roaming so a chase reads
    static constexpr double HUNT_INTERVAL = 0.17;

    /// @brief  how long between steps while roaming or searching
    static constexpr double ROAM_INTERVAL = 0.34;

    /// @brief  how far a wandering Entity will pick a destination from where it stands
    /// @note   without this they walk to random cells anywhere on the map, which scatters
    ///         them into the corners and means nothing ever finds anything
    static constexpr int ROAM_RADIUS = 12;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------

    /// @brief  the cell this Entity is standing on
    /// @return the cell, or (-1, -1) when it has no Transform
    Vec2i currentCell() const;

    /// @brief  where this Entity should be heading given its state
    /// @return the goal cell, or (-1, -1) when it has none
    Vec2i chooseGoal();

    /// @brief  picks a random reachable cell to wander towards
    /// @return a walkable cell, or (-1, -1) when the map has none
    Vec2i pickRoamDestination() const;

    /// @brief  pushes the current route to the grid so it can be drawn
    void publishTrail() const;

    /// @brief  updates the Glyph so the state is visible on screen
    void publishSymbol() const;

    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  what this Entity is currently doing
    ChaserState m_State = ChaserState::Roaming;

    /// @brief  the route worked out on the last step
    Path m_Path;

    /// @brief  where this Entity is heading
    Vec2i m_Destination{ -1, -1 };

    /// @brief  how long is left before this Entity may step again
    double m_MoveCooldown = 0.0;

    /// @brief  how far this Entity can see
    int m_SightRange = 12;

    /// @brief  the name of the Entity this one hunts
    std::string m_TargetName = "Player";

};

#endif //CHASER_H
