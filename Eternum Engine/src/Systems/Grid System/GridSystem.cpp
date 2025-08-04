/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GridSystem.cpp
* Description:
*     Handles creation, storage, and rendering of grid-based maps.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "GridSystem.h"
#include <gtest/internal/gtest-internal.h>
#include "Systems/Input/InputSystem.h"


// --------------------------------------------------------
// Constructor / Destructor
// --------------------------------------------------------
GridSystem::GridSystem()
    :  System("Grid System") , m_activeMapName("")
{
}

// --------------------------------------------------------
// Lifecycle Methods
// --------------------------------------------------------
void GridSystem::Init()
{
    System::Init();
}

void GridSystem::Shutdown()
{
    System::Shutdown();
}

void GridSystem::Update(double deltaTime)
{

    // G Key to Generate a new map
    if (Input()->IsKeyPressed(Key::G))
    {
        Dimension dim(50, 10); // Example dimensions
        CreateMap("NewMap", dim, '.');
        LoadMap("NewMap");
        std::cout << "Created and loaded new map: NewMap" << std::endl;
    }

    // H to Generate a random map
    if (Input()->IsKeyPressed(Key::H))
    {
        const auto width = 20 + rand() % 80; // Random width between 20 and 100
        const auto height = 5 + rand() % 20; // Random height between 5 and 25

        const Dimension dim(width, height); // Example dimensions
        CreateMap("RandomMap", dim, '.');
        LoadMap("RandomMap");
        std::cout << "Created and loaded random map: RandomMap" << std::endl;
    }

}

void GridSystem::FixedUpdate()
{

}

void GridSystem::Render()
{
    // nothing to draw or map invalid? skip
    if (m_activeMapName.empty() ||
        m_maps.find(m_activeMapName) == m_maps.end() ||
        !m_needsRedraw)
    {
        return;
    }

    // clear & redraw
    ClearConsole();
    const Grid& grid = m_maps.at(m_activeMapName);

    for (int y = 0; y < grid.m_Dimension.m_Height; ++y)
    {
        for (int x = 0; x < grid.m_Dimension.m_Width; ++x)
            std::cout << grid.GetCell(x, y);
        std::cout << '\n';
    }

    // we’re now up to date
    m_needsRedraw = false;
}


// --------------------------------------------------------
// Map Management
// --------------------------------------------------------
void GridSystem::CreateMap(const std::string& name, const Dimension& dimensions, const char fill)
{
    m_maps[name] = Grid(dimensions.m_Width, dimensions.m_Height, fill);
    MarkDirty();
}

void GridSystem::AddMap(const std::string& name, const Grid& map)
{
    m_maps[name] = map;
}

bool GridSystem::LoadMap(const std::string& name)
{
    if (m_maps.find(name) == m_maps.end())
        return false;

    m_activeMapName = name;
    return true;
}

void GridSystem::DeleteMap(const std::string& name)
{
    m_maps.erase(name);
    if (m_activeMapName == name)
        m_activeMapName.clear();
    MarkDirty();
}

void GridSystem::ClearMaps()
{
    m_maps.clear();
    m_activeMapName.clear();
    MarkDirty();
}

// --------------------------------------------------------
// Cell Access
// --------------------------------------------------------
int GridSystem::GetWidth() const
{
    if (m_activeMapName.empty()) return 0;
    return m_maps.at(m_activeMapName).m_Dimension.m_Width;
}

int GridSystem::GetHeight() const
{
    if (m_activeMapName.empty()) return 0;
    return m_maps.at(m_activeMapName).m_Dimension.m_Height;
}

char GridSystem::GetCell(const int x, const int y) const
{
    if (m_activeMapName.empty()) return ' ';
    return m_maps.at(m_activeMapName).GetCell(x, y);
}

void GridSystem::SetCell(const int x, const int y, const char value)
{
    if (m_activeMapName.empty()) return;
    m_maps.at(m_activeMapName).SetCell(x, y, value);
    MarkDirty();
}

