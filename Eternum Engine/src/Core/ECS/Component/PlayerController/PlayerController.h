/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PlayerController
* Description:
*     Turns held direction keys into movement on the grid. Movement is real time and paced
*     by a cooldown rather than by keypresses, so holding a direction walks. Refuses to walk
*     into walls or off the map, and keeps the status line under the map up to date.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>
#include <Systems/Event/EventSystem.h>

class PlayerController final : public ComponentOf< PlayerController >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    PlayerController();

    /// @brief  destructor
    ~PlayerController() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  starts listening for damage so the view can be knocked about
    void OnInit() override;

    /// @brief  stops listening
    void OnExit() override;

    /// @brief  reads input and moves the Entity
    /// @param  deltaTime   seconds since the last frame
    void OnUpdate( double deltaTime ) override;

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

    /// @brief  tries to move the Entity by one step, or attacks whatever is in the way
    /// @param  offset  the direction to step in, in cells
    /// @return whether the step was taken, a blocked step or an attack returns false
    bool TryMove( Vec2i const& offset );

    /// @brief  swings at whatever is standing on a cell
    /// @param  cell    the cell to swing at
    /// @return how much damage landed, zero when there was nothing to hit
    int TryAttack( Vec2i const& cell );

    /// @brief  whether this Entity is able to act at all
    /// @return false when it is dead or stunned
    bool CanAct() const;

    /// @brief  how long a hit leaves an enemy staggered
    static constexpr double STAGGER_DURATION = 0.35;

    /// @brief  the least the view is knocked about by any hit taken
    /// @note   measured, this always moves the view by a cell and never by two
    static constexpr double SHAKE_ON_HURT = 0.70;

    /// @brief  how much more a hit that takes a whole health bar would add on top
    /// @note   a heavy hit reaches the ceiling, which is where two cell shakes start
    static constexpr double SHAKE_PER_HEALTH_LOST = 0.30;

    /// @brief  how much the view is knocked about by landing a hit
    /// @note   lighter than taking one, hitting something should feel good not alarming
    static constexpr double SHAKE_ON_HIT = 0.50;

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets how many steps this Entity has taken
    /// @return the number of successful moves
    int GetStepsTaken() const { return m_StepsTaken; }

    /// @brief  resets the step count, used when a new dungeon is generated
    void ResetSteps();

    /// @brief  gets how long between steps while a direction is held
    /// @return the cooldown in seconds
    double GetMoveInterval() const { return m_MoveInterval; }

    /// @brief  sets how long between steps while a direction is held
    /// @param  seconds the new cooldown
    void SetMoveInterval( const double seconds ) { m_MoveInterval = seconds; }

    /// @brief  the direction currently being asked for
    /// @return the direction, or (0, 0) when no direction key is held
    Vec2i GetHeldDirection() const;

    /// @brief  how long between steps by default, about eight cells a second
    static constexpr double DEFAULT_MOVE_INTERVAL = 0.12;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------


    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  how many successful moves this Entity has made
    int m_StepsTaken = 0;

    /// @brief  what this Entity is listening to damage with
    EventSystem::Token m_DamageListener = EventSystem::NO_TOKEN;

    /// @brief  how long is left before this Entity may step again
    double m_MoveCooldown = 0.0;

    /// @brief  how long between steps while a direction is held
    double m_MoveInterval = DEFAULT_MOVE_INTERVAL;

};

#endif //PLAYERCONTROLLER_H
