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
#include <Systems/Input/Key/Key.h>

class InputSystem final : public System
{
public:
    static std::shared_ptr<InputSystem> GetInstance()
    {
        static std::shared_ptr<InputSystem> instance(new InputSystem());
        return instance;
    };

    void Init() override;
    void Update(double dt) override;
    void FixedUpdate() override;
    void Render() override;
    void Shutdown() override;

    /// @return last key read, or INVALID/UNKNOWN if none
    static Key KeyPressed();

    /// @return true on the frame the key first goes down
    bool    IsKeyPressed(Key key = Key::UNKNOWN);

    /// @return true as long as the key stays down
    bool IsKeyDown(Key key = Key::UNKNOWN);

private:
    explicit InputSystem();

    /// map raw int → uppercase Key
    static Key  NormalizeKey(int raw);

    /// log it and return the char
    static char MapKey(Key k);

    std::unordered_map<Key, bool> m_CurrentState;
    std::unordered_map<Key, bool> m_PreviousState;
};

static InputSystem* Input()
{
    return InputSystem::GetInstance().get();
}

// Register the InputSystem with the SystemRegistry
REGISTER_SYSTEM(InputSystem)

#endif //INPUTSYSTEM_H
