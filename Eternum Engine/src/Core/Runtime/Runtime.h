/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Runtime.cpp
* Description:
*     Implements the core runtime loop, handling initialization, the main update/render
*     cycle, and shutdown of all game systems.
*
*     This is the central entry point for executing the engine’s systems and managing the
*     flow of control between game states.
*
* Author:     Jax Clayton
* Created:    2025-08-02
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#ifndef RUNTIME_H
#define RUNTIME_H

#include <Systems/system.h>

class Runtime
{

    // ----------------------------------------------------------------
    // Singleton pattern to ensure only one instance of Runtime exists
    // ----------------------------------------------------------------
public:

    ~Runtime();

    // Singleton access
    static std::shared_ptr<Runtime> GetInstance()
    {
        static std::shared_ptr<Runtime> instance(new Runtime());
        return instance;
    }

    // Starts the game loop
    void Run();

    // Stops the game loop
    void Stop();

    // ------------------------------------------------------------------
    // === Private methods for the Runtime class ===
    // --------------------------------------------------------------------
private:
    Runtime(); // Private constructor

    // Initialization helpers
    void Init();
    void Shutdown();

    // Core loop stages
    void Update(double deltaTime);
    void FixedUpdate();
    void Render();

    double GetDeltaTime() const {
        return m_LastTime;
    }

    double GetFixedDeltaTime() const {
        return m_FixedDeltaTime;
    }

    // ----------------------------------------------------------------
    // Delete copy/move semantics for singleton safety
    // ---------------------------------------------------------------
public:
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    // ------------------------------------------------------------------
    // === Private Variables for the Runtime class ===
    // --------------------------------------------------------------------
private:
    bool m_Running = false;

    // Timing
    double m_LastTime = 0.0;
    double m_Accumulator = 0.0;
    const double m_FixedDeltaTime = 0.016;

};

// Static Runtime instance call
inline Runtime* RuntimeSystem()
{
    return Runtime::GetInstance().get();
}

#endif //RUNTIME_H
