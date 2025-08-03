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

// Register the InputSystem with the SystemRegistry
AUTO_REGISTER_SYSTEM(InputSystem)

void InputSystem::Init()
{
    System::Init();
}

void InputSystem::Update(double deltaTime)
{
    System::Update(deltaTime);
}

void InputSystem::FixedUpdate(double fixedDeltaTime)
{
    System::FixedUpdate(fixedDeltaTime);
}

void InputSystem::Render()
{
    System::Render();
}

void InputSystem::Shutdown()
{
    System::Shutdown();
}

bool InputSystem::KeyPressed()
{

#ifdef _WIN32
    return _kbhit() != 0; // _kbhit() returns a non-zero value if a key has been pressed
#else
    termios originalTermSettings;  // Stores original terminal attributes so we can restore them later
    termios modifiedTermSettings;  // Stores modified terminal attributes for non-blocking input
    int pressedKey;                 // Holds the read character (if any)
    int originalFileFlags;          // Stores original file descriptor flags

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &originalTermSettings);
    modifiedTermSettings = originalTermSettings;

    // Turn off canonical mode (ICANON) and echo (ECHO) so input is available immediately
    modifiedTermSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &modifiedTermSettings);

    // Get current file descriptor flags for stdin
    originalFileFlags = fcntl(STDIN_FILENO, F_GETFL, 0);

    // Set stdin to non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, originalFileFlags | O_NONBLOCK);

    // Try to read a single character
    pressedKey = getchar();

    // Restore terminal settings and file descriptor flags
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermSettings);
    fcntl(STDIN_FILENO, F_SETFL, originalFileFlags);

    // If a key was pressed, put it back into stdin buffer and return true
    if (pressedKey != EOF) {
        ungetc(pressedKey, stdin);
        return true;
    }

    return false;

#endif

}
