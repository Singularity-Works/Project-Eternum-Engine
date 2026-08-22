/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Health
* Description:
*     How much damage an Entity can take before it dies.
*
*     Real time combat needs a moment of mercy after a hit, otherwise standing next to two
*     enemies kills you faster than you can read the screen. So a hit opens a short window
*     where further damage is ignored.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef HEALTH_H
#define HEALTH_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Health final : public ComponentOf< Health >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Health();

    /// @brief  constructor
    /// @param  maxHealth   how much damage this Entity can take
    explicit Health( int maxHealth );

    /// @brief  destructor
    ~Health() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  counts down the mercy window after a hit
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

    /// @brief  takes damage, unless this Entity is already dead or still recovering
    /// @param  amount  how much damage to deal
    /// @return how much damage actually landed, zero when it was ignored
    int TakeDamage( int amount );

    /// @brief  takes damage with no mercy window, used for effects that tick over time
    /// @param  amount  how much damage to deal
    /// @return how much damage actually landed
    int TakeDamageIgnoringMercy( int amount );

    /// @brief  restores health, never above the maximum
    /// @param  amount  how much to restore
    /// @return how much was actually restored
    int Heal( int amount );

    /// @brief  drops this Entity to zero health straight away
    void Kill();

    /// @brief  puts this Entity back to full health and clears the mercy window
    void Reset();

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets how much health is left
    /// @return the current health
    int GetHealth() const { return m_Health; }

    /// @brief  gets how much health this Entity has at full
    /// @return the maximum health
    int GetMaxHealth() const { return m_MaxHealth; }

    /// @brief  sets how much health this Entity has at full
    /// @param  maxHealth   the new maximum, current health is clamped to it
    void SetMaxHealth( int maxHealth );

    /// @brief  gets whether this Entity still has health left
    /// @return whether it is alive
    bool IsAlive() const { return m_Health > 0; }

    /// @brief  gets whether this Entity is still recovering from the last hit
    /// @return whether further damage would be ignored
    bool IsRecovering() const { return m_MercyRemaining > 0.0; }

    /// @brief  gets how long damage is ignored for after a hit
    /// @return the window in seconds
    double GetMercyWindow() const { return m_MercyWindow; }

    /// @brief  sets how long damage is ignored for after a hit
    /// @param  seconds the new window, zero to turn it off
    void SetMercyWindow( const double seconds ) { m_MercyWindow = std::max( seconds, 0.0 ); }

    /// @brief  gets whether this Entity is removed from the Scene when it dies
    /// @return whether death destroys the Entity
    bool GetDestroyOnDeath() const { return m_DestroyOnDeath; }

    /// @brief  sets whether this Entity is removed from the Scene when it dies
    /// @param  destroy whether death should destroy the Entity
    /// @note   the player stays so the game has something to report on, enemies do not
    void SetDestroyOnDeath( const bool destroy ) { m_DestroyOnDeath = destroy; }

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    static constexpr int DEFAULT_MAX_HEALTH = 20;
    static constexpr double DEFAULT_MERCY_WINDOW = 0.35;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------

    /// @brief  handles hitting zero health
    void die();

    /// @brief  tells the rest of the engine that health came off
    /// @param  dealt   how much was lost
    void announceDamage( int dealt ) const;

    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    int m_MaxHealth = DEFAULT_MAX_HEALTH;
    int m_Health = DEFAULT_MAX_HEALTH;

    /// @brief  how long damage is ignored for after a hit
    double m_MercyWindow = DEFAULT_MERCY_WINDOW;

    /// @brief  how much of the mercy window is left
    double m_MercyRemaining = 0.0;

    /// @brief  whether death removes this Entity from the Scene
    bool m_DestroyOnDeath = true;

};

#endif //HEALTH_H
