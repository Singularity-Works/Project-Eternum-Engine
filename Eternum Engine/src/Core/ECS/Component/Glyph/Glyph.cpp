/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Glyph
* Description:
*     The character an Entity is drawn as on the grid.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Glyph.h"
#include <nlohmann/json.hpp>
#include <Core/ECS/Component/ComponentFactory.h>

Glyph::Glyph() = default;

Glyph::Glyph( const char symbol, const int drawOrder ) :
    m_Symbol( symbol ),
    m_DrawOrder( drawOrder )
{}

void Glyph::Inspect()
{
    std::cout << GetName()
              << " symbol " << m_Symbol
              << " drawOrder " << m_DrawOrder
              << " visible " << ( m_IsVisible ? "yes" : "no" )
              << std::endl;
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------

void Glyph::Write( nlohmann::json& data ) const
{
    // a one character string reads better in a save file than a number
    data[ "symbol" ] = std::string( 1, m_Symbol );
    data[ "drawOrder" ] = m_DrawOrder;
    data[ "visible" ] = m_IsVisible;
}

void Glyph::Read( nlohmann::json const& data )
{
    if ( data.contains( "symbol" ) )
    {
        const std::string symbol = data[ "symbol" ].get< std::string >();
        if ( !symbol.empty() )
            m_Symbol = symbol.front();
    }

    if ( data.contains( "drawOrder" ) )
        m_DrawOrder = data[ "drawOrder" ].get< int >();

    if ( data.contains( "visible" ) )
        m_IsVisible = data[ "visible" ].get< bool >();
}

REGISTER_COMPONENT(Glyph)
