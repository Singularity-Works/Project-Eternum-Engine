/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: PathfindingSystem
* Description:
*     Holds the navigation view of the current map and decides which algorithm to route with.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "PathfindingSystem.h"

#include <Systems/Grid System/GridSystem.h>
#include <Systems/Input/InputSystem.h>

PathfindingSystem::PathfindingSystem()
    : System( "Pathfinding System" )
{}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void PathfindingSystem::Init()
{
    System::Init();
}

void PathfindingSystem::Update( double deltaTime )
{
    if ( Input()->IsKeyPressed( Key::P ) )
        CycleAlgorithm();

    if ( Input()->IsKeyPressed( Key::V ) )
    {
        GridSystem* grid = GridSystem::GetInstance().get();
        grid->SetOverlayVisible( !grid->IsOverlayVisible() );
    }

    // holds the camera shake on, for looking at it without being hit
    if ( Input()->IsKeyPressed( Key::K ) )
    {
        GridSystem* grid = GridSystem::GetInstance().get();
        grid->SetContinuousShake( !grid->IsContinuousShake() );
    }

}

void PathfindingSystem::FixedUpdate()
{
}

void PathfindingSystem::Render()
{
}

void PathfindingSystem::Shutdown()
{
    System::Shutdown();
}

// ----------------------------------------------------------------
// Routing
// ----------------------------------------------------------------

Path PathfindingSystem::FindPath( Vec2i const& start, Vec2i const& goal )
{
    const Path path = Pathfinding::FindPath( GetNavGrid(), start, goal, m_Algorithm );

    m_LastExpanded = path.m_Expanded;
    m_LastStepCount = path.GetStepCount();

    return path;
}

NavGrid const& PathfindingSystem::GetNavGrid()
{
    // the map only changes when a new dungeon is made, so this is built once per dungeon
    if ( !m_NavGridValid )
    {
        m_NavGrid = NavGrid::FromActiveMap();
        m_NavGridValid = true;
    }

    return m_NavGrid;
}

void PathfindingSystem::InvalidateNavGrid()
{
    m_NavGridValid = false;
}

// ----------------------------------------------------------------
// Algorithm
// ----------------------------------------------------------------

void PathfindingSystem::SetAlgorithm( const Pathfinding::Algorithm algorithm )
{
    if ( m_Algorithm == algorithm )
        return;

    m_Algorithm = algorithm;
    ResetStatistics();
}

void PathfindingSystem::CycleAlgorithm()
{
    SetAlgorithm( Pathfinding::GetNextAlgorithm( m_Algorithm ) );
}

// ----------------------------------------------------------------
// Statistics
// ----------------------------------------------------------------

void PathfindingSystem::ResetStatistics()
{
    m_LastExpanded = 0;
    m_LastStepCount = 0;
}
