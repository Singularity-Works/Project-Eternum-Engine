/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Random
* Description:
*     A seeded random number source. Hand the same seed to two runs and they produce the
*     same dungeon, which is what makes generation reproducible and testable.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef RANDOM_H
#define RANDOM_H

#include <pch.h>

class Random
{

public:
    //-----------------------------------------------------------------------------
    // Constructors
    //-----------------------------------------------------------------------------

    /// @brief  constructor, picks an unpredictable seed
    Random();

    /// @brief  constructor
    /// @param  seed    the seed to start from
    explicit Random( unsigned seed );

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  restarts this source from a known seed
    /// @param  seed    the seed to start from
    void Seed( unsigned seed );

    /// @brief  restarts this source from an unpredictable seed
    /// @return the seed that was chosen
    unsigned Reseed();

    /// @brief  a whole number in a range
    /// @param  min the lowest value that can come back
    /// @param  max the highest value that can come back, included
    /// @return a number between min and max
    int Range( int min, int max );

    /// @brief  a real number in a range
    /// @param  min the lowest value that can come back
    /// @param  max the highest value that can come back
    /// @return a number between min and max
    float Range( float min, float max );

    /// @brief  a coin flip with adjustable odds
    /// @param  percent how often this should come back true, from 0 to 100
    /// @return whether the roll succeeded
    bool Chance( int percent );

    /// @brief  a valid index into a container of the given size
    /// @param  size    how many items the container holds
    /// @return an index from 0 to size - 1, or 0 when the container is empty
    std::size_t Index( std::size_t size );

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  gets the seed this source was started from
    /// @return the seed this source was started from
    unsigned GetSeed() const { return m_Seed; }

private:
    //-----------------------------------------------------------------------------
    // Private Member Variables
    //-----------------------------------------------------------------------------

    /// @brief  the underlying generator
    std::mt19937 m_Engine;

    /// @brief  the seed this source was started from
    unsigned m_Seed = 0;

};

/// @brief  the random source the engine uses by default
/// @return the shared Random instance
Random& Rng();

#endif //RANDOM_H
