/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: system.h
* Description:
*     Defines the base class for all systems in the Eternum Engine.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
// ReSharper Disable all

#ifndef SYSTEM_H
#define SYSTEM_H

#include <pch.h>

/// @brief Base class for all systems in the Eternum Engine.
///
class System
{
    // ----------------------------------------------------------------
    //                   === Public Methods ===
    // ----------------------------------------------------------------
public:
    virtual ~System() = default;

    // explicit keyword ensures that this constructor cannot be used for implicit conversions
    // This constructor initializes the system with a name for identification
    explicit System(const std::string& name)
        : m_Name(name)
    {
    }

    // Called once when the system is created
    virtual void Init()
    {
        std::cout << "Initializing system: " << m_Name << std::endl;
    }

    // Called every frame before fixed updates
    virtual void Update(double deltaTime) {}

    // Called on fixed intervals (e.g., physics updates)
    virtual void FixedUpdate() {}

    // Called every frame after update for rendering
    virtual void Render() {}

    // Called when the system is shutting down
    virtual void Shutdown()
    {
        std::cout << "Shutdown system: " << m_Name << std::endl;
    }

    // Returns the name of the system for identification
    const std::string& GetName() const { return m_Name; }

    // -- ----------------------------------------------------------------
    //                   === Private Members ===
    // ----------------------------------------------------------------
private:
    std::string m_Name; // Name of the system for identification
};


#include <Systems/System Registry/SystemRegistry.h>
#endif // SYSTEM_H
