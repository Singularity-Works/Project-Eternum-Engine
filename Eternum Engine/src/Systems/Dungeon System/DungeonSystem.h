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
#ifndef DUNGEONSYSTEM_H
#define DUNGEONSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Systems/System Registry/SystemRegistry.h>
#include <Systems/Grid System/GridSystem.h>

#define Dimension GridSystem::Dimension


class DungeonSystem final : public System
{


public:

    // ----------------------------------------------------------------
    // Room Structure
    // ----------------------------------------------------------------
    struct Room
    {
        int m_X;
        int m_Y;
        int m_Width;
        int m_Height;

        Room(int x, int y, int w, int h)
            : m_X(x), m_Y(y), m_Width(w), m_Height(h) {}
    };


    // --------------------------------------------------------
    // Utility
    // --------------------------------------------------------

    Dimension GetRandomRoomCenter() const;
    Dimension GetRandomRoomEdge() const;
    static Dimension GetRandomTileInRoom(const Room& room);
    Dimension GetRandomUnoccupiedTileInRoom(const Room& room, char emptyChar = '.') const;
    const std::vector<Room>& GetRooms() const { return m_Rooms; }

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------
    static std::shared_ptr<DungeonSystem> GetInstance()
    {
        static std::shared_ptr<DungeonSystem> instance(new DungeonSystem());
        return instance;
    }

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------
    void Init() override;
    void Shutdown() override;
    void Update(double deltaTime) override;
    void FixedUpdate() override;
    void Render() override;

    // ----------------------------------------------------------------
    // Map Management
    // ----------------------------------------------------------------
    void GenerateRoomAndCorridor(int width, int height, char floorChar = '.', char wallChar = '#');
    void GenerateCave(int width, int height, int smoothSteps = 4);
    void SendToGridSystem(const std::string& mapName);

private:
    DungeonSystem(); // Private constructor

    static void AddRoom(GridSystem::Grid& grid, int x, int y, int w, int h, char floorChar);
    static void AddCorridor(GridSystem::Grid& grid, int x1, int y1, int x2, int y2, char floorChar);
    int CountWallNeighbors(int gridX, int gridY) const;

private:
    GridSystem::Grid m_CurrentGrid;
    int m_Width = 0;
    int m_Height = 0;
    std::vector<Room> m_Rooms;
};

// Register the DungeonSystem with the SystemRegistry
REGISTER_SYSTEM(DungeonSystem)

#endif // DUNGEONSYSTEM_H
