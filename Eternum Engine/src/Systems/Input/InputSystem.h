/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: InputSystem
* Description:
*       Implements the Input System for handling user input.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#pragma once
#include <Systems/system.h>

class InputSystem final : public System
{
public:


    /// @brief Initializes the Input System.
    void Init() override;

    void Update(double deltaTime) override;

    void FixedUpdate(double fixedDeltaTime) override;

    void Render() override;

    void Shutdown() override;

    /// @brief Checks if a key has been pressed without blocking.
    /// @return true if a key was pressed, false otherwise.
    static bool KeyPressed();

    // ----------------------------------------------------------------
    // Singleton pattern to ensure only one instance of InputSystem exists
    // ----------------------------------------------------------------
    static std::shared_ptr<InputSystem> GetInstance()
    {
        static std::shared_ptr<InputSystem> instance(new InputSystem());
        return instance;
    }

private:

    // Private constructor to enforce singleton pattern
    // This ensures that InputSystem can only be created through GetInstance()
    explicit InputSystem() : System("Input System") {}

};

static InputSystem* Input()
{
    return InputSystem::GetInstance().get();
}



#endif //INPUTSYSTEM_H
