/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: InputSystem
* Description:
*
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <pch.h>
#include "InputSystem.h"
#include <Core/Runtime/Runtime.h>

// Register the InputSystem with the SystemRegistry
AUTO_REGISTER_SYSTEM(InputSystem)

void InputSystem::Init()
{
    System::Init();
}

void InputSystem::Update(double deltaTime)
{
    System::Update(deltaTime);

    if (IsKeyPressed())
    {
        std::cout << "Key pressed!" << std::endl;
    }

     if (IsKeyPressed(Key::ESC))
    {
        std::cout << "Exiting..." << std::endl;
         RuntimeSystem()->Stop();
    }
}

void InputSystem::FixedUpdate()
{
    System::FixedUpdate();
}

void InputSystem::Render()
{
    System::Render();
}

void InputSystem::Shutdown()
{
    System::Shutdown();
}

Key InputSystem::KeyPressed()
{
#ifdef _WIN32
    if (_kbhit())
    {
        int ch = _getch(); // Read key
        return MapKey(ch);
    }
    return Key::UNKNOWN;
#else
    termios originalTermSettings;
    termios modifiedTermSettings;
    int pressedKey;
    int originalFileFlags;

    // Save current terminal settings
    tcgetattr(STDIN_FILENO, &originalTermSettings);
    modifiedTermSettings = originalTermSettings;

    // Non-canonical, no echo
    modifiedTermSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &modifiedTermSettings);

    // Non-blocking stdin
    originalFileFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, originalFileFlags | O_NONBLOCK);

    // Try read
    pressedKey = getchar();

    // Restore settings
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermSettings);
    fcntl(STDIN_FILENO, F_SETFL, originalFileFlags);

    if (pressedKey != EOF)
        return MapKey(pressedKey);

    return Key::UNKNOWN;

#endif
}

bool InputSystem::IsKeyPressed(Key key)
{

    if (key == Key::UNKNOWN)
    {
        // If no specific key is requested, check if any key is pressed
        return KeyPressed() != Key::UNKNOWN;
    }

    Key currentKey = KeyPressed();
    bool pressed = (currentKey == key);
    bool result = pressed && !m_lastKeyPressed;
    m_lastKeyPressed = pressed;
    return result;
}

Key InputSystem::MapKey(int rawCode)
{
    switch (rawCode)
    {
    case 27:  return Key::ESC;
    case 32:  return Key::SPACE;
    case 13:  return Key::ENTER;
    case 8:   return Key::BACKSPACE;

    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        return static_cast<Key>(rawCode);
    }
    return Key::UNKNOWN;
}