/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: TerminalRenderer
* Description:
*     Paints a grid of characters to the console without flicker.
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
    m_NeedsFullRedraw = true;
}

void TerminalRenderer::EndSession()
{
    if ( !m_SessionOpen )
        return;

    std::cout << RESET << SHOW_CURSOR << LEAVE_OWN_SCREEN << std::flush;

    m_SessionOpen = false;
    m_NeedsFullRedraw = true;
    m_PreviousCells.clear();
    m_PreviousPanel.clear();
    m_Width = 0;
    m_Height = 0;
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

    const bool beside = panelFitsBeside( width );

    const bool full = m_NeedsFullRedraw
                   || width != m_Width
                   || height != m_Height
                   || beside != m_PanelWasBeside
                   || m_PreviousCells.size() != expected;

    // the layout decision has to be in place before any panel line is positioned
    m_PanelWasBeside = beside;

    std::string out;
    out.reserve( full ? expected * 2 + 1024 : 2048 );

    if ( full )
        appendFullFrame( out, cells, width, height, panel, colors );
    else
        appendDiffFrame( out, cells, width, height, panel, colors );

    // a diff frame is empty when nothing moved, which is the common case
    if ( !out.empty() )
    {
        // park the cursor below everything so a stray redraw cannot land in the map
        const int parkRow = beside
            ? std::max( height, static_cast< int >( panel.size() ) ) + 1
            : height + static_cast< int >( panel.size() ) + 1;

        out += RESET;
        out += moveTo( parkRow, 1 );

        // one write, a frame that arrives in pieces is what the flicker was
        std::cout.write( out.data(), static_cast< std::streamsize >( out.size() ) );
        std::cout.flush();
    }

    m_PreviousCells.assign( cells.begin(), cells.begin() + static_cast< std::ptrdiff_t >( expected ) );
    m_PreviousPanel = panel;
    m_Width = width;
    m_Height = height;
    m_NeedsFullRedraw = false;
}

void TerminalRenderer::appendFullFrame( std::string& out, std::vector< char > const& cells,
                                        const int width, const int height,
                                        std::vector< std::string > const& panel,
                                        Palette const& colors ) const
{
    out += CLEAR_SCREEN;

    const char* current = nullptr;

    for ( int y = 0; y < height; ++y )
    {
        const int rowStart = y * width;

        for ( int x = 0; x < width; ++x )
        {
            const char cell = cells[ rowStart + x ];
            const char* color = colorFor( cell, colors );

            // only spend bytes on a colour code when the colour actually changes
            if ( color != current )
            {
                out += color;
                current = color;
            }

            out += cell;
        }

        out += RESET;
        current = RESET;
        out += "\r\n";
    }

    for ( std::size_t i = 0; i < panel.size(); ++i )
        appendPanelLine( out, i, panel[ i ], width, height );
}

void TerminalRenderer::appendDiffFrame( std::string& out, std::vector< char > const& cells,
                                        const int width, const int height,
                                        std::vector< std::string > const& panel,
                                        Palette const& colors ) const
{
    const char* current = nullptr;

    for ( int y = 0; y < height; ++y )
    {
        const int rowStart = y * width;
        int x = 0;

        while ( x < width )
        {
            if ( cells[ rowStart + x ] == m_PreviousCells[ rowStart + x ] )
            {
                ++x;
                continue;
            }

            // stretch the run over short unchanged gaps, repainting a few cells beats
            // paying for another cursor move
            int runEnd = x + 1;
            int gap = 0;

            for ( int probe = runEnd; probe < width; ++probe )
            {
                if ( cells[ rowStart + probe ] != m_PreviousCells[ rowStart + probe ] )
                {
                    runEnd = probe + 1;
                    gap = 0;
                }
                else if ( ++gap > MAX_GAP )
                {
                    break;
                }
            }

            out += moveTo( y + 1, x + 1 );

            for ( int cell = x; cell < runEnd; ++cell )
            {
                const char symbol = cells[ rowStart + cell ];
                const char* color = colorFor( symbol, colors );

                if ( color != current )
                {
                    out += color;
                    current = color;
                }

                out += symbol;
            }

            x = runEnd;
        }
    }

    for ( std::size_t i = 0; i < panel.size(); ++i )
    {
        if ( i < m_PreviousPanel.size() && m_PreviousPanel[ i ] == panel[ i ] )
            continue;

        appendPanelLine( out, i, panel[ i ], width, height );
    }

    // wipe any panel line that is no longer used
    for ( std::size_t i = panel.size(); i < m_PreviousPanel.size(); ++i )
        appendPanelLine( out, i, "", width, height );
}

void TerminalRenderer::appendPanelLine( std::string& out, const std::size_t index,
                                        std::string const& text,
                                        const int width, const int height ) const
{
    const int row = m_PanelWasBeside
        ? static_cast< int >( index ) + 1
        : height + static_cast< int >( index ) + 1;

    const int column = m_PanelWasBeside ? width + PANEL_GAP : 1;

    out += moveTo( row, column );
    out += RESET;
    out += text;
    out += CLEAR_LINE_END;
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
