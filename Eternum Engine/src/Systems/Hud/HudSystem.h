/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: HudSystem
* Description:
*     Builds the panel that sits beside the map.
*
*     This one pulls rather than being pushed to. Every other System used to shove its own
*     line at the grid, which meant the layout was spread across four files and nothing
*     could lay anything out relative to anything else. The HUD is allowed to know about
*     everything, so it asks.
*
*     The health bar is drawn with coloured background blocks and eases towards the real
*     value instead of snapping, with a second slower bar behind it so a hit leaves a
*     visible wound that drains away.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef HUDSYSTEM_H
#define HUDSYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <Systems/Event/EventSystem.h>

class HudSystem final : public System
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
    // Public Methods
    // ----------------------------------------------------------------

    /// @brief  adds a line to the log beside the map
    /// @param  line    what happened
    void AddLogLine( std::string const& line );

    /// @brief  empties the log
    void ClearLog();

    /// @brief  gets what the log is currently showing, newest last
    /// @return the log lines
    std::vector< std::string > const& GetLog() const { return m_Log; }

    /// @brief  how many lines the log keeps before the oldest falls off
    static constexpr std::size_t LOG_LINES = 5;

    /// @brief  snaps the animated bars to wherever health actually is
    /// @note   called when a new dungeon starts so the bar does not slide up from empty
    void SnapBars();

    /// @brief  builds a bar out of coloured blocks
    /// @param  fraction    how full the bar is, from 0 to 1
    /// @param  trail       how full the slower trailing bar is, from 0 to 1
    /// @param  cells       how many blocks wide the bar is
    /// @return the bar as an escaped string
    static std::string BuildBar( double fraction, double trail, int cells );

    // ----------------------------------------------------------------
    // Settings
    // ----------------------------------------------------------------

    /// @brief  how many blocks wide the health bar is
    static constexpr int BAR_CELLS = 18;

    /// @brief  how quickly the bar catches up to the real value
    static constexpr double BAR_SPEED = 9.0;

    /// @brief  how quickly the wound behind the bar drains away
    static constexpr double TRAIL_SPEED = 0.5;

    /// @brief  below this fraction the bar turns amber
    static constexpr double HURT_FRACTION = 0.55;

    /// @brief  below this fraction the bar turns red
    static constexpr double CRITICAL_FRACTION = 0.25;

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< HudSystem > GetInstance()
    {
        static std::shared_ptr< HudSystem > instance( new HudSystem() );
        return instance;
    }

private:

    HudSystem(); // Private constructor

    /// @brief  starts listening for the things worth logging
    void subscribeToEvents();

    /// @brief  moves the animated bars towards where health actually is
    /// @param  deltaTime   seconds since the last frame
    void advanceBars( double deltaTime );

    /// @brief  puts the whole panel together
    /// @return the panel lines, top to bottom
    std::vector< std::string > buildPanel() const;

    /// @brief  a heading with a rule under it
    /// @param  title   the heading text
    /// @param  into    the panel being built
    static void addHeading( std::string const& title, std::vector< std::string >& into );

    /// @brief  a label on the left and a value on the right
    /// @param  label   what it is
    /// @param  value   what it says
    /// @param  into    the panel being built
    static void addRow( std::string const& label, std::string const& value,
                        std::vector< std::string >& into );

    /// @brief  where the animated bar currently sits, in health
    double m_DisplayHealth = 0.0;

    /// @brief  where the slower wound bar currently sits, in health
    double m_TrailHealth = 0.0;

    /// @brief  the last few things that happened, oldest first
    std::vector< std::string > m_Log;

    /// @brief  what the log is listening with, so it can stop on shutdown
    std::vector< EventSystem::Token > m_Listeners;

};

// Static HudSystem instance call
inline HudSystem* Hud()
{
    return HudSystem::GetInstance().get();
}

// Register the HudSystem with the SystemRegistry
REGISTER_SYSTEM(HudSystem)

#endif //HUDSYSTEM_H
