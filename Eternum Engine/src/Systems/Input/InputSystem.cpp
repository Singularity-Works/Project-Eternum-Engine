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

InputSystem::InputSystem()
  : System("Input System")
{}

void InputSystem::Init()    { System::Init(); }
void InputSystem::FixedUpdate() {}
void InputSystem::Render()  {}
void InputSystem::Shutdown(){ System::Shutdown(); }

void InputSystem::Update(double dt)
{
    if (IsKeyPressed(Key::ESC))
    {
        std::cout << "Exiting..." << std::endl;
        RuntimeSystem()->Stop();
    }
}

//— read, normalize, log, return
Key InputSystem::KeyPressed()
{
    int raw = -1;

#ifdef _WIN32
    if (!_kbhit()) return Key::INVALID;
    raw = _getch();
#else
    termios orig, mod;
    int flags;

    // save & set noncanonical, no-echo
    tcgetattr(STDIN_FILENO, &orig);
    mod = orig;
    mod.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &mod);

    // nonblocking
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    raw = getchar();

    // restore
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    fcntl(STDIN_FILENO, F_SETFL, flags);

    if (raw == EOF) return Key::UNKNOWN;
#endif
    Key k = NormalizeKey(raw);
    MapKey(k);
    return k;
}

Key InputSystem::NormalizeKey(int raw)
{
    // turn a–z into A–Z
    if (raw >= 'a' && raw <= 'z')
        raw -= ('a' - 'A');
    return static_cast<Key>(raw);
}

char InputSystem::MapKey(Key k)
{
    char c = static_cast<char>(static_cast<int>(k));
    return c;
}

bool InputSystem::IsKeyPressed(Key key)
{
    const Key cur = KeyPressed();
    const bool hit = (key == Key::ANY)
               ? (cur != Key::INVALID && cur != Key::UNKNOWN)
               : (cur == key);

    const bool justDown = hit && !m_lastDown;
    m_lastDown = hit;
    return justDown;
}

bool InputSystem::IsKeyDown(const Key key)
{
    return (KeyPressed() == key);
}