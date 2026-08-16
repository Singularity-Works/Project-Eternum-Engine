/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PathfindingSystem
* Description:
*     Holds the navigation view of the current map and decides which algorithm everything
*     routes with. Keeping the choice in one place is what lets the demo switch between
*     BFS, Dijkstra and A star at runtime and show what each one costs.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef PATHFINDINGSYSTEM_H
#define PATHFINDINGSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Systems/Pathfinding/Pathfinding.h>

class PathfindingSystem final : public System
{

public:

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    void Init() override;
    void Update( double deltaTime ) override;
    void FixedUpdate() override;
    void Render() override;
    void Shutdown() override;

    // ----------------------------------------------------------------
    // Routing
    // ----------------------------------------------------------------

    /// @brief  finds a route using whichever algorithm is currently selected
    /// @param  start   where to route from
    /// @param  goal    where to route to
    /// @return the route, invalid when the goal cannot be reached
    Path FindPath( Vec2i const& start, Vec2i const& goal );

    /// @brief  the navigation view of the current map, rebuilt only when the map changes
    /// @return the current NavGrid
    NavGrid const& GetNavGrid();

    /// @brief  tells this System the map changed underneath it
    void InvalidateNavGrid();

    // ----------------------------------------------------------------
    // Algorithm
    // ----------------------------------------------------------------

    /// @brief  gets the algorithm in use
    /// @return the algorithm in use
    Pathfinding::Algorithm GetAlgorithm() const { return m_Algorithm; }

    /// @brief  sets the algorithm to use
    /// @param  algorithm   the algorithm to use
    void SetAlgorithm( Pathfinding::Algorithm algorithm );

    /// @brief  moves to the next algorithm, wrapping round at the end
    void CycleAlgorithm();

    /// @brief  gets the readable name of the algorithm in use
    /// @return the name of the algorithm in use
    std::string GetAlgorithmName() const { return Pathfinding::GetAlgorithmName( m_Algorithm ); }

    // ----------------------------------------------------------------
    // Statistics
    // ----------------------------------------------------------------

    /// @brief  how many cells the last search opened
    /// @return the cell count, which is the honest way to compare algorithms
    int GetLastExpanded() const { return m_LastExpanded; }

    /// @brief  how many steps the last route took
    /// @return the step count
    int GetLastStepCount() const { return m_LastStepCount; }

    /// @brief  forgets the last search, used when a new dungeon starts
    void ResetStatistics();

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< PathfindingSystem > GetInstance()
    {
        static std::shared_ptr< PathfindingSystem > instance( new PathfindingSystem() );
        return instance;
    }

private:

    PathfindingSystem(); // Private constructor

    /// @brief  which algorithm everything routes with
    Pathfinding::Algorithm m_Algorithm = Pathfinding::Algorithm::AStar;

    /// @brief  the navigation view of the current map
    NavGrid m_NavGrid;

    /// @brief  whether the NavGrid still matches the map
    bool m_NavGridValid = false;

    int m_LastExpanded = 0;
    int m_LastStepCount = 0;

};

// Static PathfindingSystem instance call
inline PathfindingSystem* Paths()
{
    return PathfindingSystem::GetInstance().get();
}

// Register the PathfindingSystem with the SystemRegistry
REGISTER_SYSTEM(PathfindingSystem)

#endif //PATHFINDINGSYSTEM_H
