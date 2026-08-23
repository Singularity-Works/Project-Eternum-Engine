/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Attacker
* Description:
*     The ability to hurt something. Holds how hard it hits, how often it can swing, and
*     an optional effect to leave behind on whatever it hits.
*
*     Split from Health on purpose. An Entity with Health and no Attacker is a barrel, one
*     with an Attacker and no Health is a trap, and one with both is a fighter.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef ATTACKER_H
#define ATTACKER_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Entity;

class Attacker final : public ComponentOf< Attacker >
{

public:

    /// @brief  what a hit leaves behind on the target
    struct OnHit
    {
        EffectType m_Type = EffectType::None;
        double m_Duration = 0.0;
        int m_Magnitude = 0;

        /// @brief  how often the effect lands, from 0 to 100
        int m_Chance = 100;
    };

    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Attacker();

    /// @brief  constructor
    /// @param  damage      how much a hit takes off
    /// @param  interval    how long between swings in seconds
    Attacker( int damage, double interval );

    /// @brief  destructor
    ~Attacker() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  counts down the swing cooldown
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

    /// @brief  swings at a target
    /// @param  target  the Entity to hit
    /// @return how much damage landed, zero when the swing was not ready or missed nothing
    int Attack( Entity* target );

    /// @brief  puts the swing back on cooldown without hitting anything
    void ResetCooldown();

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  whether the swing is off cooldown
    /// @return whether this Entity can attack right now
    bool CanAttack() const { return m_Cooldown <= 0.0; }

    /// @brief  gets how much a hit takes off
    /// @return the damage per hit
    int GetDamage() const { return m_Damage; }

    /// @brief  sets how much a hit takes off
    /// @param  damage  the new damage per hit
    void SetDamage( const int damage ) { m_Damage = std::max( damage, 0 ); }

    /// @brief  gets how long between swings
    /// @return the interval in seconds
    double GetAttackInterval() const { return m_AttackInterval; }

    /// @brief  sets how long between swings
    /// @param  seconds the new interval
    void SetAttackInterval( const double seconds ) { m_AttackInterval = std::max( seconds, 0.0 ); }

    /// @brief  gets what a hit leaves behind
    /// @return the on hit effect
    OnHit const& GetOnHit() const { return m_OnHit; }

    /// @brief  sets what a hit leaves behind
    /// @param  onHit   the new on hit effect
    void SetOnHit( OnHit const& onHit ) { m_OnHit = onHit; }

    /// @brief  gets how much damage this Entity has dealt in total
    /// @return the running total
    int GetTotalDamageDealt() const { return m_TotalDamageDealt; }

    /// @brief  forgets the running total and clears the cooldown
    void Reset();

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    static constexpr int DEFAULT_DAMAGE = 3;
    static constexpr double DEFAULT_INTERVAL = 0.5;

private:
    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    int m_Damage = DEFAULT_DAMAGE;

    /// @brief  how long between swings
    double m_AttackInterval = DEFAULT_INTERVAL;

    /// @brief  how long until this Entity can swing again
    double m_Cooldown = 0.0;

    /// @brief  what a hit leaves behind on the target
    OnHit m_OnHit;

    /// @brief  how much damage this Entity has dealt in total
    int m_TotalDamageDealt = 0;

};

#endif //ATTACKER_H
