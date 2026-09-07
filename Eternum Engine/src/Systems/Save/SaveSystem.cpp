/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: SaveSystem
* Description:
*     Writes the whole game out and reads it back.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "SaveSystem.h"

#include <nlohmann/json.hpp>

#include <Systems/Dungeon System/DungeonSystem.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Input/InputSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>

#include <fstream>

namespace
{
    /// @brief  reads a whole file into memory
    /// @param  path    what to read
    /// @param  bytes   where to put it
    /// @return whether the file could be read
    bool ReadFile( std::string const& path, std::vector< std::uint8_t >& bytes )
    {
        std::ifstream file( path, std::ios::binary );
        if ( !file )
            return false;

        bytes.assign( std::istreambuf_iterator< char >( file ),
                      std::istreambuf_iterator< char >() );

        return true;
    }

    /// @brief  writes a block of bytes over a file
    /// @param  path    what to write
    /// @param  bytes   what to put in it
    /// @return whether the file could be written
    bool WriteFile( std::string const& path, std::vector< std::uint8_t > const& bytes )
    {
        std::ofstream file( path, std::ios::binary | std::ios::trunc );
        if ( !file )
            return false;

        file.write( reinterpret_cast< const char* >( bytes.data() ),
                    static_cast< std::streamsize >( bytes.size() ) );

        return file.good();
    }

    /// @brief  turns a byte count into something readable
    /// @param  bytes   how many bytes
    /// @return a short string like "3.1 KB"
    std::string Describe( const std::size_t bytes )
    {
        if ( bytes < 1024 )
            return std::to_string( bytes ) + " B";

        std::ostringstream text;
        text << std::fixed << std::setprecision( 1 ) << ( bytes / 1024.0 ) << " KB";
        return text.str();
    }
}

SaveSystem::SaveSystem()
    : System( "Save System" )
{}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void SaveSystem::Init()
{
    System::Init();
}

void SaveSystem::Update( double deltaTime )
{
    if ( Input()->IsKeyPressed( Key::F5 ) )
        m_LastResult = Save();

    if ( Input()->IsKeyPressed( Key::F9 ) )
        m_LastResult = Load();
}

void SaveSystem::FixedUpdate()
{
}

void SaveSystem::Render()
{
}

void SaveSystem::Shutdown()
{
    System::Shutdown();
}

// ----------------------------------------------------------------
// Saving and Loading
// ----------------------------------------------------------------

nlohmann::json SaveSystem::BuildSnapshot() const
{
    nlohmann::json snapshot;
    snapshot[ "version" ] = FORMAT_VERSION;

    nlohmann::json dungeon = nlohmann::json::object();
    DungeonSystem::GetInstance()->WriteState( dungeon );
    snapshot[ "dungeon" ] = dungeon;

    nlohmann::json scene = nlohmann::json::object();
    Entities()->WriteScene( scene );
    snapshot[ "scene" ] = scene;

    snapshot[ "pathfinding" ] = {
        { "algorithm", Paths()->GetAlgorithmName() }
    };

    return snapshot;
}

bool SaveSystem::ApplySnapshot( nlohmann::json const& snapshot )
{
    const int version = snapshot.value( "version", 0 );
    if ( version != FORMAT_VERSION )
    {
        std::cout << "WARNING: save is version " << version
                  << ", this build writes " << FORMAT_VERSION << std::endl;
        return false;
    }

    if ( snapshot.contains( "dungeon" ) )
        DungeonSystem::GetInstance()->ReadState( snapshot[ "dungeon" ] );

    if ( snapshot.contains( "scene" ) )
        Entities()->ReadScene( snapshot[ "scene" ] );

    if ( snapshot.contains( "pathfinding" ) )
    {
        const std::string algorithm = snapshot[ "pathfinding" ].value( "algorithm", std::string() );

        for ( int i = 0; i < Pathfinding::ALGORITHM_COUNT; ++i )
        {
            const auto candidate = static_cast< Pathfinding::Algorithm >( i );
            if ( Pathfinding::GetAlgorithmName( candidate ) == algorithm )
            {
                Paths()->SetAlgorithm( candidate );
                break;
            }
        }
    }

    // the map moved under everything, so routes and the screen both have to be redone
    Paths()->InvalidateNavGrid();
    GridSystem::GetInstance()->ClearAllOverlays();
    GridSystem::GetInstance()->ForceFullRedraw();

    return true;
}

SaveSystem::Result SaveSystem::Save()
{
    Result result;

    try
    {
        const nlohmann::json snapshot = BuildSnapshot();

        const std::string text = snapshot.dump( 2 );
        const std::vector< std::uint8_t > json( text.begin(), text.end() );
        const std::vector< std::uint8_t > binary = nlohmann::json::to_msgpack( snapshot );

        if ( !WriteFile( JSON_PATH, json ) || !WriteFile( BINARY_PATH, binary ) )
        {
            result.m_Message = "Save failed, could not write";
            return result;
        }

        result.m_Succeeded = true;
        result.m_JsonBytes = json.size();
        result.m_BinaryBytes = binary.size();
        result.m_Message = "Saved " + Describe( json.size() ) + " / " + Describe( binary.size() );
    }
    catch ( std::exception const& error )
    {
        result.m_Message = std::string( "Save failed, " ) + error.what();
    }

    return result;
}

SaveSystem::Result SaveSystem::Load()
{
    Result result;

    std::vector< std::uint8_t > bytes;

    // the binary is what the game reads, the text is there for a person to open
    const bool haveBinary = ReadFile( BINARY_PATH, bytes );
    const bool haveJson = !haveBinary && ReadFile( JSON_PATH, bytes );

    if ( !haveBinary && !haveJson )
    {
        result.m_Message = "No save to load";
        return result;
    }

    try
    {
        const nlohmann::json snapshot = haveBinary
            ? nlohmann::json::from_msgpack( bytes )
            : nlohmann::json::parse( bytes );

        if ( !ApplySnapshot( snapshot ) )
        {
            result.m_Message = "Save could not be applied";
            return result;
        }

        result.m_Succeeded = true;
        result.m_Message = haveBinary ? "Loaded from binary" : "Loaded from json";
    }
    catch ( std::exception const& error )
    {
        result.m_Message = std::string( "Load failed, " ) + error.what();
    }

    return result;
}
