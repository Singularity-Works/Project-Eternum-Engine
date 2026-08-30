/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GameEvents
* Description:
*     The things the game announces. Each one is a plain type, so listening for the wrong
*     thing fails to compile rather than quietly never firing.
*
*     Every event carries a short line of text with it. That is not decoration, it is what
*     the log beside the map is built from, and it means whatever raises an event decides
*     how it reads rather than the HUD guessing.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

#include <pch.h>
#include <Systems/Event/EventSystem.h>

class Entity;

//-----------------------------------------------------------------------------
// Combat
//-----------------------------------------------------------------------------

/// @brief  Something lost health.
struct DamageTakenEvent final : Event
{
    Entity* m_Target = nullptr;
    Entity* m_Source = nullptr;
    int m_Amount = 0;

    /// @brief  how much of the target's health that hit was worth, from 0 to 1
    double m_Fraction = 0.0;

    std::string m_Message;
};

/// @brief  Something ran out of health.
struct EntityDiedEvent final : Event
{
    Entity* m_Entity = nullptr;
    std::string m_Message;
};

//-----------------------------------------------------------------------------
// The Dungeon
//-----------------------------------------------------------------------------

/// @brief  Something stepped on a trap.
struct TrapTriggeredEvent final : Event
{
    Entity* m_Trap = nullptr;
    Entity* m_Victim = nullptr;
    int m_Damage = 0;

    std::string m_Message;
};

/// @brief  Something picked something up.
struct PickupCollectedEvent final : Event
{
    Entity* m_Pickup = nullptr;
    Entity* m_Collector = nullptr;
    int m_Restored = 0;

    std::string m_Message;
};

/// @brief  A new dungeon was built.
struct DungeonGeneratedEvent final : Event
{
    unsigned m_Seed = 0;
    std::string m_Generator;
    std::string m_Message;
};

#endif //GAMEEVENTS_H
