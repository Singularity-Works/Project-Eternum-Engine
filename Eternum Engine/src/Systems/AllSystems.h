/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: AllSystems
* Description:
*     Includes all systems in the Eternum Engine for easy access.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#ifndef ALLSYSTEMS_H
#define ALLSYSTEMS_H

#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Input/InputSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Dungeon System/DungeonSystem.h>


// Component Systems
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Trap/Trap.h>
#include <Core/ECS/Component/Pickup/Pickup.h>
#include <Core/ECS/Component/Solid/Solid.h>


#endif //ALLSYSTEMS_H
