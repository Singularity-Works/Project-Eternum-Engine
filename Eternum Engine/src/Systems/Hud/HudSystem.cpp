/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: HudSystem
* Description:
*     Builds the panel that sits beside the map.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "HudSystem.h"

#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Core/Terminal/TerminalRenderer.h>
#include <Systems/Dungeon System/DungeonSystem.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <Systems/Save/SaveSystem.h>
#include <Systems/Event/GameEvents.h>

namespace
{
    /// @brief  how many columns a label takes before its value starts
    constexpr int LABEL_WIDTH = 11;

    /// @brief  how wide a heading rule is drawn
    constexpr int RULE_WIDTH = 28;

    // background colours, a run of spaces on one of these reads as a solid block
    const char* const BAR_GREEN  = "\x1b[42m";
    const char* const BAR_AMBER  = "\x1b[43m";
    const char* const BAR_RED    = "\x1b[41m";
    const char* const BAR_WOUND  = "\x1b[101m";
    const char* const BAR_EMPTY  = "\x1b[100m";

    // foreground colours for the text around it
    const char* const DIM   = "\x1b[90m";
    const char* const BOLD  = "\x1b[97m";
    const char* const RESET = "\x1b[0m";

    /// @brief  pads a string out to a width, so the values line up in a column
    std::string Pad( std::string const& text, const std::size_t width )
    {
        if ( text.size() >= width )
            return text;

        return text + std::string( width - text.size(), ' ' );
    }
}

HudSystem::HudSystem()
    : System( "Hud System" )
{}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void HudSystem::Init()
{
    System::Init();
    SnapBars();
    subscribeToEvents();
}

void HudSystem::subscribeToEvents()
{
    // the HUD knows nothing about traps or loot, it just writes down what it is told
    m_Listeners.push_back( Events()->Subscribe< TrapTriggeredEvent >(
        [ this ]( TrapTriggeredEvent const& event ) { AddLogLine( event.m_Message ); } ) );

    m_Listeners.push_back( Events()->Subscribe< PickupCollectedEvent >(
        [ this ]( PickupCollectedEvent const& event ) { AddLogLine( event.m_Message ); } ) );

    m_Listeners.push_back( Events()->Subscribe< EntityDiedEvent >(
        [ this ]( EntityDiedEvent const& event ) { AddLogLine( event.m_Message ); } ) );

    m_Listeners.push_back( Events()->Subscribe< DungeonGeneratedEvent >(
        [ this ]( DungeonGeneratedEvent const& event )
        {
            ClearLog();
            AddLogLine( event.m_Message );
        } ) );
}

void HudSystem::AddLogLine( std::string const& line )
{
    if ( line.empty() )
        return;

    m_Log.push_back( line );

    while ( m_Log.size() > LOG_LINES )
        m_Log.erase( m_Log.begin() );
}

void HudSystem::ClearLog()
{
    m_Log.clear();
}

void HudSystem::Update( const double deltaTime )
{
    advanceBars( deltaTime );

    // SetPanel only marks the grid dirty when something actually changed
    GridSystem::GetInstance()->SetPanel( buildPanel() );
}

void HudSystem::FixedUpdate()
{
}

void HudSystem::Render()
{
}

void HudSystem::Shutdown()
{
    for ( const EventSystem::Token token : m_Listeners )
        Events()->Unsubscribe( token );

    m_Listeners.clear();

    System::Shutdown();
}

// ----------------------------------------------------------------
// Public Methods
// ----------------------------------------------------------------

void HudSystem::SnapBars()
{
    Entity* player = Entities()->FindEntity( "Player" );
    if ( player == nullptr )
        return;

    const Health* health = player->GetComponent< Health >();
    if ( health == nullptr )
        return;

    m_DisplayHealth = health->GetHealth();
    m_TrailHealth = m_DisplayHealth;
}

std::string HudSystem::BuildBar( double fraction, double trail, const int cells )
{
    fraction = std::clamp( fraction, 0.0, 1.0 );
    trail = std::clamp( trail, fraction, 1.0 );

    const int filled = static_cast< int >( std::lround( fraction * cells ) );
    const int wounded = static_cast< int >( std::lround( trail * cells ) );

    const char* fillColor = BAR_GREEN;
    if ( fraction <= CRITICAL_FRACTION )
        fillColor = BAR_RED;
    else if ( fraction <= HURT_FRACTION )
        fillColor = BAR_AMBER;

    std::string bar;

    // the bar is spaces on a coloured background, which draws as a solid block
    if ( filled > 0 )
    {
        bar += fillColor;
        bar += std::string( filled, ' ' );
    }

    // the wound is what was just lost, it drains away a moment later
    if ( wounded > filled )
    {
        bar += BAR_WOUND;
        bar += std::string( wounded - filled, ' ' );
    }

    if ( cells > wounded )
    {
        bar += BAR_EMPTY;
        bar += std::string( cells - wounded, ' ' );
    }

    bar += RESET;
    return bar;
}

// ----------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------

void HudSystem::advanceBars( const double deltaTime )
{
    Entity* player = Entities()->FindEntity( "Player" );
    if ( player == nullptr )
        return;

    const Health* health = player->GetComponent< Health >();
    if ( health == nullptr )
        return;

    const double target = health->GetHealth();

    // ease towards the real value, framerate independent because the step scales with time
    const double catchUp = std::min( 1.0, deltaTime * BAR_SPEED );
    m_DisplayHealth += ( target - m_DisplayHealth ) * catchUp;

    if ( std::abs( target - m_DisplayHealth ) < 0.05 )
        m_DisplayHealth = target;

    // healing pulls the wound straight up with it, only damage leaves a trail
    if ( m_TrailHealth < m_DisplayHealth )
        m_TrailHealth = m_DisplayHealth;
    else
        m_TrailHealth = std::max( m_DisplayHealth, m_TrailHealth - deltaTime * TRAIL_SPEED * health->GetMaxHealth() );
}

std::vector< std::string > HudSystem::buildPanel() const
{
    std::vector< std::string > panel;

    Entity* player = Entities()->FindEntity( "Player" );
    DungeonSystem* dungeon = DungeonSystem::GetInstance().get();

    panel.emplace_back( std::string( BOLD ) + "ETERNUM" + RESET );
    panel.emplace_back( std::string( DIM ) + std::string( RULE_WIDTH, '-' ) + RESET );
    panel.emplace_back( "" );

    // ----------------------------------------------------------------
    // Health
    // ----------------------------------------------------------------

    if ( player != nullptr )
    {
        if ( const Health* health = player->GetComponent< Health >() )
        {
            const double maximum = std::max( 1, health->GetMaxHealth() );

            panel.emplace_back( BuildBar( m_DisplayHealth / maximum, m_TrailHealth / maximum, BAR_CELLS ) );

            std::string reading = std::to_string( health->GetHealth() )
                                + " / " + std::to_string( health->GetMaxHealth() );

            if ( !health->IsAlive() )
                reading += std::string( "   " ) + BAR_RED + " DEAD " + RESET;

            addRow( "Health", reading, panel );
        }

        if ( const StatusEffects* effects = player->GetComponent< StatusEffects >() )
        {
            const std::string summary = effects->GetSummary();
            addRow( "Status", summary.empty() ? "clear" : summary, panel );
        }

        if ( const PlayerController* controller = player->GetComponent< PlayerController >() )
            addRow( "Steps", std::to_string( controller->GetStepsTaken() ), panel );

        if ( const Transform* transform = player->GetComponent< Transform >() )
        {
            Vec2f const& position = transform->GetTranslation();
            addRow( "Position", "(" + std::to_string( static_cast< int >( position.x() ) )
                              + ", " + std::to_string( static_cast< int >( position.y() ) ) + ")", panel );
        }
    }

    // ----------------------------------------------------------------
    // Dungeon
    // ----------------------------------------------------------------

    panel.emplace_back( "" );
    addHeading( "DUNGEON", panel );
    addRow( "Layout", dungeon->GetGeneratorName(), panel );
    addRow( "Seed", std::to_string( dungeon->GetSeed() ), panel );
    addRow( "Rooms", std::to_string( dungeon->GetRooms().size() ), panel );
    addRow( "Enemies", std::to_string( dungeon->CountLivingEnemies() ), panel );

    // ----------------------------------------------------------------
    // Pathfinding
    // ----------------------------------------------------------------

    panel.emplace_back( "" );
    addHeading( "PATHFINDING", panel );
    addRow( "Algorithm", Paths()->GetAlgorithmName(), panel );

    if ( GridSystem::GetInstance()->IsContinuousShake() )
        addRow( "Shake", "held on", panel );
    addRow( "Route", std::to_string( Paths()->GetLastStepCount() ) + " steps", panel );
    addRow( "Searched", std::to_string( Paths()->GetLastExpanded() ) + " cells", panel );

    // ----------------------------------------------------------------
    // Controls
    // ----------------------------------------------------------------

    panel.emplace_back( "" );
    addHeading( "LOG", panel );

    if ( m_Log.empty() )
    {
        panel.emplace_back( std::string( DIM ) + "nothing yet" + RESET );
    }
    else
    {
        for ( std::string const& line : m_Log )
            panel.emplace_back( line );
    }

    panel.emplace_back( "" );
    addHeading( "SAVE", panel );

    SaveSystem::Result const& save = Saves()->GetLastResult();
    addRow( "Last", save.m_Message.empty() ? "nothing yet" : save.m_Message, panel );

    if ( save.m_JsonBytes > 0 && save.m_BinaryBytes > 0 )
    {
        const int percent = static_cast< int >( 100.0 * save.m_BinaryBytes / save.m_JsonBytes );
        addRow( "Binary", std::to_string( percent ) + "% of json", panel );
    }

    panel.emplace_back( "" );
    addHeading( "CONTROLS", panel );
    addRow( "Move", "WASD or arrows", panel );
    addRow( "New map", "M", panel );
    addRow( "Layout", "1  2  3", panel );
    addRow( "Algorithm", "P", panel );
    addRow( "Show paths", "V", panel );
    addRow( "Shake demo", "K", panel );
    addRow( "Save", "F5", panel );
    addRow( "Load", "F9", panel );
    addRow( "Quit", "ESC", panel );

    return panel;
}

void HudSystem::addHeading( std::string const& title, std::vector< std::string >& into )
{
    into.emplace_back( std::string( BOLD ) + title + RESET );
    into.emplace_back( std::string( DIM ) + std::string( RULE_WIDTH, '-' ) + RESET );
}

void HudSystem::addRow( std::string const& label, std::string const& value,
                        std::vector< std::string >& into )
{
    into.emplace_back( std::string( DIM ) + Pad( label, LABEL_WIDTH ) + RESET + value );
}
