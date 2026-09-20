/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Log
* Description:
*     The switch that decides whether the engine narrates what it is doing.
*     Quiet by default, because the renderer owns the console once the game starts
*     and anything printed alongside it lands in the middle of the map.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>

/// @brief  the flag behind the narration, kept in a function so the header stands alone
/// @return a reference to it
inline bool& VerboseFlag()
{
    static bool enabled = false;
    return enabled;
}

/// @brief  turns the narration on or off
/// @param  enabled whether the engine should talk about itself
inline void SetVerboseLogging( const bool enabled )
{
    VerboseFlag() = enabled;
}

/// @brief  writes a line, but only when the engine was asked to be talkative
/// @param  text    what to write
inline void LogVerbose( std::string const& text )
{
    if ( !VerboseFlag() )
        return;

    std::cout << text << std::endl;
}

#endif // LOG_H
