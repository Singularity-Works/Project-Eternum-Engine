/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CameraShake
* Description:
*     Shakes the view when something hits hard enough to deserve it.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "CameraShake.h"

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void CameraShake::AddTrauma( const double amount )
{
    if ( amount <= 0.0 )
        return;

    // trauma stacks up to a ceiling, so being hit twice shakes harder but never forever
    m_Trauma = std::clamp( m_Trauma + amount, 0.0, 1.0 );
}

void CameraShake::Update( const double deltaTime )
{
    m_Time += deltaTime;

    if ( m_Continuous )
        m_Trauma = CONTINUOUS_TRAUMA;
    else if ( m_Trauma > 0.0 )
        m_Trauma = std::max( 0.0, m_Trauma - DECAY_PER_SECOND * deltaTime );

    if ( m_Trauma <= 0.0 )
    {
        m_Offset = Vec2i{ 0, 0 };
        return;
    }

    // straight through, see the note at the top of the header for why there is no curve
    const double shake = m_Trauma;
    const double sample = m_Time * NOISE_SPEED;

    // two different seeds, otherwise the view would only ever move diagonally
    const double x = MAX_OFFSET * shake * Noise( sample, 1u );
    const double y = MAX_OFFSET * shake * Noise( sample, 2u );

    m_Offset = Vec2i{ static_cast< int >( std::lround( x ) ),
                      static_cast< int >( std::lround( y ) ) };
}

void CameraShake::Clear()
{
    m_Trauma = 0.0;
    m_Offset = Vec2i{ 0, 0 };
    m_Continuous = false;
}

void CameraShake::SetContinuous( const bool continuous )
{
    m_Continuous = continuous;

    if ( !continuous )
        return;

    m_Trauma = CONTINUOUS_TRAUMA;
}

//-----------------------------------------------------------------------------
// Noise
//-----------------------------------------------------------------------------

double CameraShake::Noise( const double t, const unsigned seed )
{
    const double floored = std::floor( t );
    const auto sample = static_cast< long long >( floored );
    const double fraction = t - floored;

    const double from = hash( sample, seed );
    const double to = hash( sample + 1, seed );

    // smoothstep between samples, so the view glides rather than snapping about
    const double eased = fraction * fraction * ( 3.0 - 2.0 * fraction );

    return from + ( to - from ) * eased;
}

double CameraShake::hash( const long long sample, const unsigned seed )
{
    unsigned value = static_cast< unsigned >( sample ) * 374761393u + seed * 668265263u;
    value = ( value ^ ( value >> 13 ) ) * 1274126177u;
    value ^= value >> 16;

    // spread it over -1 to 1
    return ( static_cast< double >( value ) / 4294967295.0 ) * 2.0 - 1.0;
}
