/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: InputSystem
* Description:
*       Handles
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "Systems/system.h"

class InputSystem final : public System
{
public:

    explicit InputSystem() : System("Input System") {}

    void Init() override;

    void Update(double deltaTime) override;

    void FixedUpdate(double fixedDeltaTime) override;

    void Render() override;

    void Shutdown() override;
};


#endif //INPUTSYSTEM_H
