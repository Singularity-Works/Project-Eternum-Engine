/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Pickup
* Description:
*     Something lying on the floor that heals whatever walks over it.
*
*     Like the Trap it does not care who picks it up, so an enemy crossing a room can take
*     the potion you were heading for. Also like the Trap, it announces what happened rather
*     than reaching into the HUD, because it has no business knowing there is one.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef PICKUP_H
#define PICKUP_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Pickup final : public ComponentOf< Pickup >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Pickup();

    /// @brief  constructor
    /// @param  restores    how much health it gives back
    explicit Pickup( int restores );

    /// @brief  destructor
    ~Pickup() override = default;

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

    /// @brief  hands this Pickup to whatever walked over it
    /// @param  collector   what picked it up
    /// @return how much health was restored, zero when nothing happened
    /// @note   a full health collector leaves it on the floor rather than wasting it
    int Collect( Entity* collector );

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets how much health this Pickup gives back
    /// @return the amount restored
    int GetRestores() const { return m_Restores; }

    /// @brief  sets how much health this Pickup gives back
    /// @param  restores    the new amount
    void SetRestores( const int restores ) { m_Restores = std::max( restores, 0 ); }

    /// @brief  whether this Pickup has already been taken
    /// @return whether it is spent
    bool IsCollected() const { return m_Collected; }

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    /// @brief  what a Pickup is drawn as
    static constexpr char SYMBOL = '!';

    static constexpr int DEFAULT_RESTORES = 6;

private:
    //-----------------------------------------------------------------------------
    // Private Methods
    //-----------------------------------------------------------------------------

    /// @brief  the cell this Pickup sits on
    /// @return the cell, or (-1, -1) when it has no Transform
    Vec2i currentCell() const;

    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  how much health this Pickup gives back
    int m_Restores = DEFAULT_RESTORES;

    /// @brief  whether it has already been taken
    bool m_Collected = false;

};

#endif //PICKUP_H
