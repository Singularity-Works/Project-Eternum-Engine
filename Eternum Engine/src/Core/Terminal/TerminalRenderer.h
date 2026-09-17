/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: TerminalRenderer
* Description:
*     Paints a grid of characters to the console without flicker.
*
*     Three things make it steady. The whole frame is built into one string and written in
*     a single call, so a frame never lands half drawn. Colour codes are only emitted when
*     the colour actually changes instead of once per cell. And after the first frame only
*     the cells that differ from the last one are repainted, so a step costs two cells
*     instead of the whole map.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef TERMINALRENDERER_H
#define TERMINALRENDERER_H

#include <pch.h>

class TerminalRenderer
{

public:

    /// @brief  what colour each character is drawn in, as ansi escape strings
    using Palette = std::unordered_map< char, const char* >;

    //-----------------------------------------------------------------------------
    // Session
    //-----------------------------------------------------------------------------

    /// @brief  takes over the terminal, switching to its own screen and hiding the cursor
    /// @note   on windows this also turns on escape code handling, which older consoles
    ///         leave off by default
    void BeginSession();

    /// @brief  hands the terminal back exactly as it was found
    void EndSession();

    /// @brief  forces the next frame to be painted in full instead of diffed
    /// @note   needed after anything outside this class writes to the console
    void ForceFullRedraw() { m_NeedsFullRedraw = true; }

    //-----------------------------------------------------------------------------
    // Drawing
    //-----------------------------------------------------------------------------

    /// @brief  paints a frame, repainting only what changed since the last one
    /// @param  cells   the characters to draw, row by row, width * height of them
    /// @param  width   how many cells across
    /// @param  height  how many cells down
    /// @param  panel   lines drawn beside the grid, or under it on a narrow terminal
    /// @param  colors  what colour to draw each character in
    /// @note   panel lines may carry their own escape codes, they are written as given
    void Draw( std::vector< char > const& cells, int width, int height,
               std::vector< std::string > const& panel, Palette const& colors );

    /// @brief  how many columns of space sit between the grid and the panel
    static constexpr int PANEL_GAP = 3;

    /// @brief  how wide the panel is allowed to be before it will not fit beside the grid
    static constexpr int PANEL_WIDTH = 34;

    //-----------------------------------------------------------------------------
    // Escape Codes
    //-----------------------------------------------------------------------------

    static constexpr const char* RESET            = "\x1b[0m";
    static constexpr const char* CLEAR_SCREEN     = "\x1b[2J\x1b[H";
    static constexpr const char* CLEAR_LINE_END   = "\x1b[K";
    static constexpr const char* HIDE_CURSOR      = "\x1b[?25l";
    static constexpr const char* SHOW_CURSOR      = "\x1b[?25h";
    static constexpr const char* ENTER_OWN_SCREEN = "\x1b[?1049h";
    static constexpr const char* LEAVE_OWN_SCREEN = "\x1b[?1049l";

    //-----------------------------------------------------------------------------
    // Singleton Pattern
    //-----------------------------------------------------------------------------

    static TerminalRenderer& Instance()
    {
        static TerminalRenderer instance;
        return instance;
    }

    TerminalRenderer( TerminalRenderer const& ) = delete;
    TerminalRenderer& operator=( TerminalRenderer const& ) = delete;

private:

    TerminalRenderer() = default;

    /// @brief  builds a complete frame, used for the first one and after a size change
    void appendFullFrame( std::string& out, std::vector< char > const& cells, int width, int height,
                          std::vector< std::string > const& panel, Palette const& colors ) const;

    /// @brief  builds only the difference from the last frame
    void appendDiffFrame( std::string& out, std::vector< char > const& cells, int width, int height,
                          std::vector< std::string > const& panel, Palette const& colors ) const;

    /// @brief  writes one panel line wherever the panel currently lives
    /// @param  out     the frame being built
    /// @param  index   which panel line
    /// @param  text    what to write
    /// @param  width   how many cells the grid is across
    /// @param  height  how many cells the grid is down
    void appendPanelLine( std::string& out, std::size_t index, std::string const& text,
                          int width, int height ) const;

    /// @brief  whether the panel fits beside the grid on this terminal
    /// @param  width   how many cells the grid is across
    /// @return true to draw beside, false to drop it underneath
    bool panelFitsBeside( int width ) const;

    /// @brief  how wide the terminal is, or zero when that cannot be found out
    static int terminalWidth();

    /// @brief  the escape code a character is drawn with
    static const char* colorFor( char cell, Palette const& colors );

    /// @brief  an escape code that moves the cursor, rows and columns start at one
    static std::string moveTo( int row, int column );

    /// @brief  how many unchanged cells are worth repainting rather than moving the cursor over
    /// @note   a cursor move costs about eight bytes, a cell costs one, so hopping a short
    ///         gap is cheaper than jumping it
    static constexpr int MAX_GAP = 6;

    /// @brief  the frame currently on screen
    std::vector< char > m_PreviousCells;

    /// @brief  the panel currently on screen
    std::vector< std::string > m_PreviousPanel;

    /// @brief  whether the last frame drew the panel beside the grid
    bool m_PanelWasBeside = true;

    int m_Width = 0;
    int m_Height = 0;

    bool m_NeedsFullRedraw = true;
    bool m_SessionOpen = false;

};

/// @brief  the shared TerminalRenderer
/// @return the shared TerminalRenderer
inline TerminalRenderer& Terminal()
{
    return TerminalRenderer::Instance();
}

#endif //TERMINALRENDERER_H
