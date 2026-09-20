/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Runtime
* Description:
*       The core runtime loop for the Eternum Engine, managing the main update/render cycle,
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
// ReSharper Disable all
#include <pch.h>

#include "Runtime.h"
#include <Systems/AllSystems.h>

Runtime::~Runtime() = default;
Runtime::Runtime() = default;

// Runs the game loop, managing the main update/render cycle
void Runtime::Run() {
    Init();
    m_Running = true;

    using clock = std::chrono::high_resolution_clock;
    auto previousTime = clock::now();


    while (m_Running) {

        auto currentTime = clock::now();
        std::chrono::duration<double> time = currentTime - previousTime;
        previousTime = currentTime;

        double deltaTime = time.count();

        m_Accumulator += deltaTime;

        Update(deltaTime);

        while (m_Accumulator >= m_FixedDeltaTime)
        {
            FixedUpdate();
            m_Accumulator -= m_FixedDeltaTime;
        }

        Render();

        // nothing in this loop blocks, so without a rest it would spin a core flat out
        std::this_thread::sleep_for( std::chrono::milliseconds( FRAME_SLEEP_MS ) );
    }

    Shutdown();
}

// Stops the game loop, cleaning up resources and shutting down systems
void Runtime::Stop() {
    m_Running = false;
}

// Initializes the runtime, setting up necessary systems and resources
void Runtime::Init() {
    LogVerbose("Initializing Runtime...");

    // Print the user's system name windows or linux
#ifdef _WIN32
    LogVerbose("Running on Windows");
#else
    LogVerbose("Running on Linux");
#endif

   auto systems = Registry()->GetSystems();

    for (auto& system : systems) {
        if (system) {
            system->Init();
        }
    }

}

// Shuts down the runtime, cleaning up resources and shutting down systems
void Runtime::Shutdown() {
    LogVerbose("Shutting down...");

    auto systems = Registry()->GetSystems();

    for (auto& system : systems) {
        if (system) {
            system->Shutdown();
        }
    }
}

// Updates the game state, applying logic and changes based on the elapsed time
void Runtime::Update(double deltaTime) {

    // Update all systems with the elapsed time
    auto systems = Registry()->GetSystems();
    for (auto& system : systems) {
        if (system) {
            system->Update(deltaTime);
        }
    }

}

void Runtime::FixedUpdate() {

    auto systems = Registry()->GetSystems();

    for (auto& system : systems) {
        if (system) {
            system->FixedUpdate();
        }
    }
}

// Renders the current frame, drawing the game state to the screen
void Runtime::Render() {
    auto systems = Registry()->GetSystems();

    for (auto& system : systems) {
        if (system) {
            system->Render();
        }
    }
}
