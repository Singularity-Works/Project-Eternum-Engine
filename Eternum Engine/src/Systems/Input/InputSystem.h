/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: InputSystem
* Description:
*     Reports which keys are held right now, which is what real time movement needs.
*
*     On Windows it reads the console's own key down and key up events, so a held key is
*     held from the moment it goes down until it comes up, with no keyboard repeat delay
*     in the way. Elsewhere there are no key up events to read, so it falls back to
*     treating a key as held for a short window after the last event, which is the best a
*     plain terminal allows.
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

    /// @brief  whether a key went down this frame
    /// @param  key the key to test
    /// @return true only on the frame it first goes down
    bool IsKeyPressed(Key key) const;

    /// @brief  whether a key is held right now
    /// @param  key the key to test
    /// @return true for as long as it stays down
    bool IsKeyDown(Key key) const;

    /// @brief  whether a key came up this frame
    /// @param  key the key to test
    /// @return true only on the frame it is released
    bool IsKeyReleased(Key key) const;

    /// @brief  forgets every held key, used when the window loses focus
    void ClearHeldKeys();

    /// @brief  how long a key counts as held after its last event, on platforms that
    ///         cannot report key up
    static constexpr double HOLD_WINDOW = 0.18;

private:
    explicit InputSystem();

    /// @brief  reads the keyboard into the current state
    /// @param  dt  seconds since the last frame
    void pollKeyboard(double dt);

    /// @brief  puts the console into raw key mode so key up events arrive
    void beginRawInput();

    /// @brief  puts the console back the way it was found
    void endRawInput();

    std::unordered_map<Key, bool> m_CurrentState;
    std::unordered_map<Key, bool> m_PreviousState;

    /// @brief  how long each key stays held, only used where key up cannot be read
    std::unordered_map<Key, double> m_HeldRemaining;

    /// @brief  the console input mode as it was before the engine started
    unsigned long m_SavedConsoleMode = 0;

    /// @brief  whether the console mode was changed and still needs restoring
    bool m_RawInputActive = false;
};

static InputSystem* Input()
{
    return InputSystem::GetInstance().get();
}

// Register the InputSystem with the SystemRegistry
REGISTER_SYSTEM(InputSystem)

#endif //INPUTSYSTEM_H
