/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Random
* Description:
*     A seeded random number source.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "Random.h"

namespace
{
    /// @brief  a seed that changes between calls made in the same second
    /// @return an unpredictable seed
    unsigned PickSeed()
    {
        // time() only ticks once a second, so two dungeons made quickly came out identical
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        return static_cast< unsigned >( std::chrono::duration_cast< std::chrono::nanoseconds >( now ).count() );
    }
}

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Random::Random()
{
    Seed( PickSeed() );
}

Random::Random( const unsigned seed )
{
    Seed( seed );
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void Random::Seed( const unsigned seed )
{
    m_Seed = seed;
    m_Engine.seed( seed );
}

unsigned Random::Reseed()
{
    Seed( PickSeed() );
    return m_Seed;
}

int Random::Range( int min, int max )
{
    // a backwards range is a caller mistake, swapping beats undefined behaviour
    if ( min > max )
        std::swap( min, max );

    std::uniform_int_distribution< int > distribution( min, max );
    return distribution( m_Engine );
}

float Random::Range( float min, float max )
{
    if ( min > max )
        std::swap( min, max );

    std::uniform_real_distribution< float > distribution( min, max );
    return distribution( m_Engine );
}

bool Random::Chance( const int percent )
{
    if ( percent <= 0 )
        return false;

    if ( percent >= 100 )
        return true;

    return Range( 1, 100 ) <= percent;
}

std::size_t Random::Index( const std::size_t size )
{
    if ( size == 0 )
        return 0;

    return static_cast< std::size_t >( Range( 0, static_cast< int >( size ) - 1 ) );
}

//-----------------------------------------------------------------------------
// Shared Instance
//-----------------------------------------------------------------------------

Random& Rng()
{
    static Random instance;
    return instance;
}
