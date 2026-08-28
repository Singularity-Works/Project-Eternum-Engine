/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Trap
* Description:
*     Sits on a cell looking like floor until something walks onto it.
*
*     It does not care what stepped on it. Anything with Health sets it off, so the enemies
*     hunting you can walk into the same traps you can, which makes them part of the room
*     rather than a thing aimed only at the player.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef TRAP_H
#define TRAP_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Trap final : public ComponentOf< Trap >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Trap();

    /// @brief  constructor
    /// @param  damage  how much it takes off whatever steps on it
    explicit Trap( int damage );

    /// @brief  destructor
    ~Trap() override = default;

    //-----------------------------------------------------------------------------
    // Public Engine Methods
    //-----------------------------------------------------------------------------

    /// @brief  watches its own cell for anything standing on it
    /// @param  deltaTime   seconds since the last frame
    void OnUpdate( double deltaTime ) override;

    /// @brief  Used by the Inspection System to display information about this Component
    void Inspect() override;

    void Write( nlohmann::json& data ) const override;
    void Read( nlohmann::json const& data ) override;

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  sets this Trap off against something
    /// @param  victim  what stepped on it
    /// @return how much damage landed, zero when nothing happened
    int Trigger( Entity* victim );

    /// @brief  puts the Trap back to hidden and armed
    void Reset();

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  whether this Trap can still go off
    /// @return whether it is armed
    bool IsArmed() const { return m_Armed; }

    /// @brief  whether this Trap has been seen yet
    /// @return whether it is still disguised as floor
    bool IsHidden() const { return m_Hidden; }

    /// @brief  gets how much this Trap takes off
    /// @return the damage
    int GetDamage() const { return m_Damage; }

    /// @brief  sets how much this Trap takes off
    /// @param  damage  the new damage
    void SetDamage( const int damage ) { m_Damage = std::max( damage, 0 ); }

    /// @brief  gets what this Trap leaves behind on whatever it catches
    /// @return the effect applied on trigger
    EffectType GetEffect() const { return m_Effect; }

    /// @brief  sets what this Trap leaves behind
    /// @param  effect      the effect to apply
    /// @param  duration    how long it lasts
    /// @param  magnitude   how strong it is
    void SetEffect( EffectType effect, double duration, int magnitude );

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    /// @brief  what a hidden Trap is drawn as, which is ordinary floor
    static constexpr char HIDDEN_SYMBOL = '.';

    /// @brief  what a Trap is drawn as once it has been found
    static constexpr char REVEALED_SYMBOL = '^';

    static constexpr int DEFAULT_DAMAGE = 5;
    static constexpr double DEFAULT_EFFECT_DURATION = 3.0;
    static constexpr int DEFAULT_EFFECT_MAGNITUDE = 1;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------

    /// @brief  the cell this Trap sits on
    /// @return the cell, or (-1, -1) when it has no Transform
    Vec2i currentCell() const;

    /// @brief  updates the Glyph so a found Trap stops looking like floor
    void publishSymbol() const;

    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  how much this Trap takes off
    int m_Damage = DEFAULT_DAMAGE;

    /// @brief  whether it can still go off
    bool m_Armed = true;

    /// @brief  whether it still looks like floor
    bool m_Hidden = true;

    /// @brief  what it leaves behind on whatever it catches
    EffectType m_Effect = EffectType::Poison;
    double m_EffectDuration = DEFAULT_EFFECT_DURATION;
    int m_EffectMagnitude = DEFAULT_EFFECT_MAGNITUDE;

};

#endif //TRAP_H
