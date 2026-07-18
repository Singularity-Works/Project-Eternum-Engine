/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Glyph
* Description:
*     The character an Entity is drawn as on the grid. GridSystem stamps every Glyph over
*     the map when it renders, so nothing ever writes into the map itself.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef GLYPH_H
#define GLYPH_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>

class Glyph final : public ComponentOf< Glyph >
{

public:
    //-----------------------------------------------------------------------------
    // Constructor / Destructor
    //-----------------------------------------------------------------------------

    /// @brief  constructor
    Glyph();

    /// @brief  constructor
    /// @param  symbol      the character to draw this Entity as
    /// @param  drawOrder   higher draws over lower, the player should sit above loot
    explicit Glyph( char symbol, int drawOrder = 0 );

    /// @brief  destructor
    ~Glyph() override = default;

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  Used by the Inspection System to display information about this Component
    void Inspect() override;

    /// @brief  writes this Component's state so it can be saved
    /// @param  data    the object to write into
    void Write( nlohmann::json& data ) const override;

    /// @brief  reads this Component's state back out of a save
    /// @param  data    the object to read from
    void Read( nlohmann::json const& data ) override;


    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets the character this Entity is drawn as
    /// @return the character this Entity is drawn as
    char GetSymbol() const { return m_Symbol; }

    /// @brief  sets the character this Entity is drawn as
    /// @param  symbol  the new character
    void SetSymbol( char symbol ) { m_Symbol = symbol; }

    /// @brief  gets the draw order of this Glyph
    /// @return higher values draw over lower ones
    int GetDrawOrder() const { return m_DrawOrder; }

    /// @brief  sets the draw order of this Glyph
    /// @param  drawOrder   higher values draw over lower ones
    void SetDrawOrder( int drawOrder ) { m_DrawOrder = drawOrder; }

    /// @brief  gets whether this Glyph is drawn at all
    /// @return whether this Glyph is drawn
    bool IsVisible() const { return m_IsVisible; }

    /// @brief  sets whether this Glyph is drawn at all
    /// @param  isVisible   whether this Glyph should be drawn
    void SetVisible( bool isVisible ) { m_IsVisible = isVisible; }

private:
    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  the character this Entity is drawn as
    char m_Symbol = '?';

    /// @brief  higher values draw over lower ones
    int m_DrawOrder = 0;

    /// @brief  whether this Glyph is drawn at all
    bool m_IsVisible = true;

};

#endif //GLYPH_H
