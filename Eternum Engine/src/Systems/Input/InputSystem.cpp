/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: InputSystem
* Description:
*     Reports which keys are held right now.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <pch.h>
#include "InputSystem.h"
#include <Core/Runtime/Runtime.h>

namespace
{
#ifdef _WIN32
    /// @brief  how many console events to pull off in one go
    constexpr int EVENT_BATCH = 64;

    /// @brief  turns a windows virtual key code into a Key
    /// @param  virtualKey  the code from the console event
    /// @return the matching Key, or UNKNOWN for anything the game does not use
    Key KeyFromVirtualKey( const unsigned short virtualKey )
    {
        switch ( virtualKey )
        {
            case VK_UP:     return Key::ARROW_UP;
            case VK_DOWN:   return Key::ARROW_DOWN;
            case VK_LEFT:   return Key::ARROW_LEFT;
            case VK_RIGHT:  return Key::ARROW_RIGHT;
            case VK_ESCAPE: return Key::ESC;
            case VK_SPACE:  return Key::SPACE;
            case VK_RETURN: return Key::ENTER;
            case VK_TAB:    return Key::TAB;
            default:        break;
        }

        // letters and digits already share their ascii value with their virtual key code
        if ( virtualKey >= VK_F1 && virtualKey <= VK_F12 )
            return static_cast< Key >( static_cast< int >( Key::F1 ) + ( virtualKey - VK_F1 ) );

        if ( ( virtualKey >= 'A' && virtualKey <= 'Z' ) || ( virtualKey >= '0' && virtualKey <= '9' ) )
            return static_cast< Key >( virtualKey );

        return Key::UNKNOWN;
    }
#endif
}

InputSystem::InputSystem()
  : System("Input System")
{}

void InputSystem::Init()
{
    System::Init();
    beginRawInput();
}

void InputSystem::FixedUpdate() {}
void InputSystem::Render()  {}

void InputSystem::Shutdown()
{
    endRawInput();
    System::Shutdown();
}

void InputSystem::Update(const double dt)
{
    m_PreviousState = m_CurrentState;

    pollKeyboard(dt);

    if (IsKeyPressed(Key::ESC))
    {
        RuntimeSystem()->Stop();
    }
}

void InputSystem::ClearHeldKeys()
{
    m_CurrentState.clear();
    m_HeldRemaining.clear();
}

//-----------------------------------------------------------------------------
// Raw Mode
//-----------------------------------------------------------------------------

void InputSystem::beginRawInput()
{
#ifdef _WIN32
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input == nullptr || input == INVALID_HANDLE_VALUE)
        return;

    DWORD mode = 0;
    if (!GetConsoleMode(input, &mode))
        return;

    m_SavedConsoleMode = mode;
    m_RawInputActive = true;

    // raw keys with no line buffering and no echo, and window events so focus loss shows up.
    // quick edit has to go or a stray click pauses the whole game, and clearing it needs
    // extended flags set at the same time
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE);
    mode |= ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT;

    SetConsoleMode(input, mode);
#endif
}

void InputSystem::endRawInput()
{
#ifdef _WIN32
    if (!m_RawInputActive)
        return;

    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input != nullptr && input != INVALID_HANDLE_VALUE)
        SetConsoleMode(input, m_SavedConsoleMode);

    m_RawInputActive = false;
#endif
}

//-----------------------------------------------------------------------------
// Polling
//-----------------------------------------------------------------------------

void InputSystem::pollKeyboard(const double dt)
{
#ifdef _WIN32
    (void)dt;

    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input == nullptr || input == INVALID_HANDLE_VALUE)
        return;

    DWORD pending = 0;
    if (!GetNumberOfConsoleInputEvents(input, &pending) || pending == 0)
        return;

    INPUT_RECORD records[EVENT_BATCH];

    while (pending > 0)
    {
        const DWORD wanted = (pending < EVENT_BATCH) ? pending : EVENT_BATCH;

        DWORD read = 0;
        if (!ReadConsoleInput(input, records, wanted, &read) || read == 0)
            break;

        for (DWORD i = 0; i < read; ++i)
        {
            const INPUT_RECORD& record = records[i];

            // losing focus means the key up events never arrive, so drop everything
            if (record.EventType == FOCUS_EVENT && record.Event.FocusEvent.bSetFocus == FALSE)
            {
                ClearHeldKeys();
                continue;
            }

            if (record.EventType != KEY_EVENT)
                continue;

            const Key key = KeyFromVirtualKey(record.Event.KeyEvent.wVirtualKeyCode);
            if (key == Key::UNKNOWN)
                continue;

            m_CurrentState[key] = (record.Event.KeyEvent.bKeyDown != FALSE);
        }

        pending -= read;
    }
#endif
}

//-----------------------------------------------------------------------------
// Queries
//-----------------------------------------------------------------------------

bool InputSystem::IsKeyPressed(const Key key) const
{
    const auto current = m_CurrentState.find(key);
    if (current == m_CurrentState.end() || !current->second)
        return false;

    const auto previous = m_PreviousState.find(key);
    return previous == m_PreviousState.end() || !previous->second;
}

bool InputSystem::IsKeyDown(const Key key) const
{
    const auto current = m_CurrentState.find(key);
    return current != m_CurrentState.end() && current->second;
}

bool InputSystem::IsKeyReleased(const Key key) const
{
    const auto previous = m_PreviousState.find(key);
    if (previous == m_PreviousState.end() || !previous->second)
        return false;

    const auto current = m_CurrentState.find(key);
    return current == m_CurrentState.end() || !current->second;
}
