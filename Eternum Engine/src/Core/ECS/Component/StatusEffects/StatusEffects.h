/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: StatusEffects
* Description:
*     Timed effects hanging off an Entity. Poison takes health at intervals, Slow makes an
*     Entity act less often, and Stun stops it acting at all.
*
*     Applying an effect that is already running refreshes it rather than stacking a second
*     copy, so walking through three enemies cannot leave you stunned for a minute.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef STATUSEFFECTS_H
#define STATUSEFFECTS_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

/// @brief  the kinds of effect an Entity can be under
enum class EffectType
{
    None,
    Poison,     // loses health at intervals
    Slow,       // acts less often
    Stun        // cannot act at all
};

/// @brief  One effect currently running on an Entity.
struct StatusEffect
{
    EffectType m_Type = EffectType::None;

    /// @brief  how many seconds are left before this effect ends
    double m_Remaining = 0.0;

    /// @brief  how strong the effect is, damage per tick for Poison
    int m_Magnitude = 0;

    /// @brief  how long until this effect does something again
    double m_TickRemaining = 0.0;
};

class StatusEffects final : public ComponentOf< StatusEffects >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    StatusEffects();

    /// @brief  destructor
    ~StatusEffects() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  runs every effect down and applies whatever they do
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

    /// @brief  puts an effect on this Entity, or refreshes one already running
    /// @param  type        which effect
    /// @param  duration    how long it should last in seconds
    /// @param  magnitude   how strong it is, damage per tick for Poison
    void Apply( EffectType type, double duration, int magnitude = 0 );

    /// @brief  removes one effect
    /// @param  type    which effect to remove
    void Clear( EffectType type );

    /// @brief  removes every effect
    void ClearAll();

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  whether an effect is running
    /// @param  type    which effect to look for
    /// @return whether it is active
    bool Has( EffectType type ) const;

    /// @brief  how long an effect has left
    /// @param  type    which effect to look for
    /// @return the seconds remaining, or zero when it is not running
    double GetRemaining( EffectType type ) const;

    /// @brief  whether this Entity is stopped from acting
    /// @return whether Stun is running
    bool IsStunned() const { return Has( EffectType::Stun ); }

    /// @brief  what to multiply an action cooldown by
    /// @return more than one while slowed, otherwise one
    double GetCooldownMultiplier() const;

    /// @brief  gets every effect currently running
    /// @return the active effects
    std::vector< StatusEffect > const& GetActive() const { return m_Effects; }

    /// @brief  a short readable summary, for the line under the map
    /// @return something like "poisoned, slowed", or empty when nothing is running
    std::string GetSummary() const;

    /// @brief  the readable name of an effect
    /// @param  type    the effect to name
    /// @return its name
    static std::string GetEffectName( EffectType type );

    /// @brief  turns a name from GetEffectName back into an effect
    /// @param  name    the name to look up
    /// @return the matching effect, or None when the name is not known
    static EffectType GetEffectFromName( std::string const& name );

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    /// @brief  how long between Poison ticks
    static constexpr double POISON_TICK = 0.8;

    /// @brief  how much longer a slowed Entity waits between actions
    static constexpr double SLOW_MULTIPLIER = 2.0;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------

    /// @brief  finds a running effect
    /// @param  type    which effect to look for
    /// @return the effect, or nullptr when it is not running
    StatusEffect* find( EffectType type );

    /// @brief  finds a running effect
    /// @param  type    which effect to look for
    /// @return the effect, or nullptr when it is not running
    StatusEffect const* find( EffectType type ) const;

    /// @brief  does whatever an effect does on a tick
    /// @param  effect  the effect that ticked
    void applyTick( StatusEffect const& effect ) const;

    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  every effect currently running
    std::vector< StatusEffect > m_Effects;

};

#endif //STATUSEFFECTS_H
