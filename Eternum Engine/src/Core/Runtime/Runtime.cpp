/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Runtime
* Description:
*     TODO: Describe the purpose of this file.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
// ReSharper Disable all
#include <pch.h>
#include "Runtime.h"

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

        ProcessInput();
        Update(deltaTime);

        while (m_Accumulator >= deltaTime)
        {
            FixedUpdate();
            m_Accumulator -= m_FixedDeltaTime;
        }

        Render();
    }

    Shutdown();
}

// Stops the game loop, cleaning up resources and shutting down systems
void Runtime::Stop() {
    m_Running = false;
}

// Initializes the runtime, setting up necessary systems and resources
void Runtime::Init() {
    std::cout << "Initializing Runtime..." << std::endl;
    // TODO : Initialize game systems, load resources, etc.
}

// Shuts down the runtime, cleaning up resources and shutting down systems
void Runtime::Shutdown() {
    std::cout << "Shutting down..." << std::endl;
    // TODO : Clean up game systems, release resources, etc.
}

// Processes input events, handling user interactions and system events
void Runtime::ProcessInput() {
    // TODO: Handle input events
}

// Updates the game state, applying logic and changes based on the elapsed time
void Runtime::Update(double deltaTime) {
    // TODO: Update game state based on deltaTime
}

void Runtime::FixedUpdate() {
    // TODO: Perform fixed updates, such as physics calculations
}

// Renders the current frame, drawing the game state to the screen
void Runtime::Render() {
    // TODO: Render the current game state to the screen
}
