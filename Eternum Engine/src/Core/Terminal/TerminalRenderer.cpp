/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: TerminalRenderer
* Description:
*     Paints a grid of characters to the console.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "TerminalRenderer.h"

//-----------------------------------------------------------------------------
// Session
//-----------------------------------------------------------------------------

void TerminalRenderer::BeginSession()
{
    if ( m_SessionOpen )
        return;

    // the game gets a screen of its own, so quitting leaves the shell scrollback intact
    std::cout << ENTER_OWN_SCREEN << HIDE_CURSOR << std::flush;

    m_SessionOpen = true;
}

void TerminalRenderer::EndSession()
{
    if ( !m_SessionOpen )
        return;

    std::cout << RESET << SHOW_CURSOR << LEAVE_OWN_SCREEN << std::flush;

    m_SessionOpen = false;
}

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

void TerminalRenderer::Draw( std::vector< char > const& cells, const int width, const int height,
                             std::vector< std::string > const& panel, Palette const& colors )
{
    if ( width <= 0 || height <= 0 )
        return;

    const std::size_t expected = static_cast< std::size_t >( width ) * height;
    if ( cells.size() < expected )
        return;

    m_PanelWasBeside = panelFitsBeside( width );

    std::cout << CLEAR_SCREEN;

    for ( int y = 0; y < height; ++y )
    {
        const int rowStart = y * width;

        for ( int x = 0; x < width; ++x )
        {
            const char cell = cells[ rowStart + x ];
            std::cout << colorFor( cell, colors ) << cell;
        }

        std::cout << RESET << "\r\n";
    }

    for ( std::size_t i = 0; i < panel.size(); ++i )
        drawPanelLine( i, panel[ i ], width, height );

    // park the cursor below everything so a stray redraw cannot land in the map
    const int parkRow = m_PanelWasBeside
        ? std::max( height, static_cast< int >( panel.size() ) ) + 1
        : height + static_cast< int >( panel.size() ) + 1;

    std::cout << RESET << moveTo( parkRow, 1 ) << std::flush;
}

void TerminalRenderer::drawPanelLine( const std::size_t index, std::string const& text,
                                      const int width, const int height ) const
{
    const int row = m_PanelWasBeside
        ? static_cast< int >( index ) + 1
        : height + static_cast< int >( index ) + 1;

    const int column = m_PanelWasBeside ? width + PANEL_GAP : 1;

    std::cout << moveTo( row, column ) << RESET << text << CLEAR_LINE_END;
}

bool TerminalRenderer::panelFitsBeside( const int width ) const
{
    const int available = terminalWidth();

    // with no way to ask, assume there is room, which is true of any normal window
    if ( available <= 0 )
        return true;

    return available >= width + PANEL_GAP + PANEL_WIDTH;
}

int TerminalRenderer::terminalWidth()
{
    const HANDLE output = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( output == nullptr || output == INVALID_HANDLE_VALUE )
        return 0;

    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if ( !GetConsoleScreenBufferInfo( output, &info ) )
        return 0;

    return info.srWindow.Right - info.srWindow.Left + 1;
}

//-----------------------------------------------------------------------------
// Private Helpers
//-----------------------------------------------------------------------------

const char* TerminalRenderer::colorFor( const char cell, Palette const& colors )
{
    const auto it = colors.find( cell );
    return ( it != colors.end() ) ? it->second : RESET;
}

std::string TerminalRenderer::moveTo( const int row, const int column )
{
    return "\x1b[" + std::to_string( row ) + ";" + std::to_string( column ) + "H";
}
