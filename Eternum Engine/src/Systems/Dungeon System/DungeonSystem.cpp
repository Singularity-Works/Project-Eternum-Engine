/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: DungeonSystem.h
* Description:
*     Generates procedural dungeon maps (rooms, corridors, caves) and sends them to GridSystem.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include "DungeonSystem.h"
#include <Systems/Grid System/Key/Key.h>
#include <Systems/Input/InputSystem.h>

// ----------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------
DungeonSystem::DungeonSystem()
    : System("Dungeon System")
{
}

// --------------------------------------------------------
// Utility
// --------------------------------------------------------

Dimension DungeonSystem::GetRandomRoomCenter() const
{
    if (m_Rooms.empty())
        return { -1, -1 }; // No valid rooms

    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<size_t> dist(0, m_Rooms.size() - 1);

    const Room& chosen = m_Rooms[dist(rng)];
    return { chosen.m_X + chosen.m_Width / 2, chosen.m_Y + chosen.m_Height / 2 };
}

 Dimension DungeonSystem::GetRandomRoomEdge() const
{
    if (m_Rooms.empty())
        return { -1, -1 };

    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<size_t> roomDist(0, m_Rooms.size() - 1);
    const Room& chosen = m_Rooms[roomDist(rng)];

    // Choose a side: 0=top, 1=bottom, 2=left, 3=right
    std::uniform_int_distribution<int> sideDist(0, 3);
    int side = sideDist(rng);

    switch (side)
    {
        case 0: // Top edge
            return { static_cast<int>(chosen.m_X + rng() % chosen.m_Width), chosen.m_Y };
        case 1: // Bottom edge
            return { static_cast<int>(chosen.m_X + rng() % chosen.m_Width), chosen.m_Y + chosen.m_Height - 1 };
        case 2: // Left edge
            return { chosen.m_X, static_cast<int>(chosen.m_Y + rng() % chosen.m_Height) };
        case 3: // Right edge
            return { chosen.m_X + chosen.m_Width - 1, static_cast<int>(chosen.m_Y + rng() % chosen.m_Height) };
    }

    return { -1, -1 };
}

 Dimension DungeonSystem::GetRandomTileInRoom(const Room& room)
 {
    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> xDist(room.m_X + 1, room.m_X + room.m_Width - 2);
    std::uniform_int_distribution<int> yDist(room.m_Y + 1, room.m_Y + room.m_Height - 2);

    return { xDist(rng), yDist(rng) };
}

 Dimension DungeonSystem::GetRandomUnoccupiedTileInRoom(const Room& room, char emptyChar) const
{
    for (int attempt = 0; attempt < 50; ++attempt) // avoid infinite loop
    {
        Dimension pos = GetRandomTileInRoom(room);
        if (m_CurrentGrid.GetCell(pos.m_Width, pos.m_Height) == emptyChar)
            return pos;
    }
    return { -1, -1 }; // none found
}


// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------
void DungeonSystem::Init()
{
    System::Init();
}

void DungeonSystem::Shutdown()
{
    System::Shutdown();
}

void DungeonSystem::Update(double deltaTime)
{
    if (Input()->IsKeyPressed(Key::M))
    {
        GenerateRoomAndCorridor(40, 20, '.', '#');
        SendToGridSystem("GeneratedDungeon");
    }
}

void DungeonSystem::FixedUpdate()
{
}

void DungeonSystem::Render()
{
}

void DungeonSystem::GenerateRoomAndCorridor(int width, int height, char floorChar, char wallChar)
{
    m_Width = width;
    m_Height = height;
    m_CurrentGrid = GridSystem::Grid(width, height, wallChar);
    m_Rooms.clear();

    // Random generator
    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> roomWidthDist(4, 8);
    std::uniform_int_distribution<int> roomHeightDist(4, 8);
    std::uniform_int_distribution<int> posXDist(1, width - 10);
    std::uniform_int_distribution<int> posYDist(1, height - 10);

    // --------------------------------------------------------
    // Generate multiple random rooms
    // --------------------------------------------------------
    for (int i = 0; i < 6; ++i) {
        int rw = roomWidthDist(rng);
        int rh = roomHeightDist(rng);
        int rx = posXDist(rng);
        int ry = posYDist(rng);

        AddRoom(m_CurrentGrid, rx, ry, rw, rh, floorChar);
        m_Rooms.emplace_back(rx, ry, rw, rh);
    }


    // --------------------------------------------------------
    // Connect rooms with corridors
    // --------------------------------------------------------
    for (size_t i = 1; i < m_Rooms.size(); ++i) {
        int x1 = m_Rooms[i - 1].m_X + m_Rooms[i - 1].m_Width / 2;
        int y1 = m_Rooms[i - 1].m_Y + m_Rooms[i - 1].m_Height / 2;
        int x2 = m_Rooms[i].m_X + m_Rooms[i].m_Width / 2;
        int y2 = m_Rooms[i].m_Y + m_Rooms[i].m_Height / 2;

        AddCorridor(m_CurrentGrid, x1, y1, x2, y2, floorChar);
    }

    // --------------------------------------------------------
    // Add some random scatter tiles for irregularity
    // --------------------------------------------------------
    std::uniform_int_distribution<int> scatterChance(0, 100);
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            if (m_CurrentGrid.GetCell(x, y) == wallChar && scatterChance(rng) < 4) {
                m_CurrentGrid.SetCell(x, y, floorChar);
            }
        }
    }
}

void DungeonSystem::GenerateCave(int width, int height, int smoothSteps)
{
    m_Width = width;
    m_Height = height;
    m_CurrentGrid = GridSystem::Grid(width, height, '#'); // start all walls

    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> fillChance(0, 100);

    const int initialFillPercent = 45; // percentage of walls

    // Step 1: Randomly fill
    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            if (fillChance(rng) < initialFillPercent)
                m_CurrentGrid.SetCell(x, y, '#');
            else
                m_CurrentGrid.SetCell(x, y, '.');
        }
    }

    // Step 2: Smooth
    for (int step = 0; step < smoothSteps; ++step)
    {
        GridSystem::Grid newGrid = m_CurrentGrid;

        for (int y = 1; y < height - 1; ++y)
        {
            for (int x = 1; x < width - 1; ++x)
            {
                int wallCount = CountWallNeighbors(x, y);

                if (wallCount > 4)
                    newGrid.SetCell(x, y, '#');
                else if (wallCount < 4)
                    newGrid.SetCell(x, y, '.');
            }
        }
        m_CurrentGrid = newGrid;
    }
}

void DungeonSystem::AddRoom(GridSystem::Grid& grid, int x, int y, int w, int h, char floorChar)
{
    for (int j = y; j < y + h; ++j)
        for (int i = x; i < x + w; ++i)
            grid.SetCell(i, j, floorChar);
}

void DungeonSystem::AddCorridor(GridSystem::Grid& grid, int x1, int y1, int x2, int y2, char floorChar)
{
    const int dx = (x2 > x1) ? 1 : -1;
    for (int x = x1; x != x2; x += dx)
        grid.SetCell(x, y1, floorChar);

    const int dy = (y2 > y1) ? 1 : -1;
    for (int y = y1; y != y2; y += dy)
        grid.SetCell(x2, y, floorChar);
}

int DungeonSystem::CountWallNeighbors(int gridX, int gridY) const
{
    int count = 0;
    for (int ny = -1; ny <= 1; ++ny)
    {
        for (int nx = -1; nx <= 1; ++nx)
        {
            if (nx == 0 && ny == 0) continue; // skip self

            char cell = m_CurrentGrid.GetCell(gridX + nx, gridY + ny);
            if (cell == '#')
                count++;
        }
    }
    return count;
}
void DungeonSystem::SendToGridSystem(const std::string& mapName)
{
    GridSystem::GetInstance()->CreateMap(mapName, { m_Width, m_Height }, '#');
    GridSystem::GetInstance()->AddMap(mapName, m_CurrentGrid);
    GridSystem::GetInstance()->LoadMap(mapName);
}
