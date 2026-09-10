/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CameraShake
* Description:
*     Shakes the view when something hits hard enough to deserve it.
*
*     Nothing here plays an animation. There is a single trauma value that things add to and
*     that decays on its own, and the offset is worked out from whatever trauma happens to
*     be right now. Two hits close together therefore stack into one bigger shake instead of
*     the second one restarting the first.
*
*     The offset follows smooth noise rather than a fresh random number each frame, which
*     reads as a jolt rather than static.
*
*     Trauma maps to movement linearly, which is not what a floating point camera would do.
*     A terminal moves in whole cells, so the entire useful range is zero, one or two, and
*     any falloff curve spends that range erasing every shake that is not a near death blow.
*     The stacking is what gives weight here, not a curve.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef CAMERASHAKE_H
#define CAMERASHAKE_H

#include <pch.h>

class CameraShake
{

public:

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  adds to the trauma, which is the thing that decays
    /// @param  amount  how much to add, 1 is as shaken as it gets
    void AddTrauma( double amount );

    /// @brief  runs the trauma down and works out this frame's offset
    /// @param  deltaTime   seconds since the last frame
    void Update( double deltaTime );

    /// @brief  stops the shake immediately
    void Clear();

    /// @brief  holds the shake on so it can be looked at
    /// @param  continuous  whether to keep topping the trauma up
    void SetContinuous( bool continuous );

    /// @brief  whether the shake is being held on
    /// @return whether continuous mode is on
    bool IsContinuous() const { return m_Continuous; }

    //-----------------------------------------------------------------------------
    // Public Accessors
    //-----------------------------------------------------------------------------

    /// @brief  how far the view should be pushed this frame
    /// @return the offset in cells, zero when nothing is shaking
    Vec2i const& GetOffset() const { return m_Offset; }

    /// @brief  how shaken things currently are
    /// @return the trauma, from 0 to 1
    double GetTrauma() const { return m_Trauma; }

    /// @brief  whether the view is currently being pushed around
    /// @return whether there is any trauma left
    bool IsShaking() const { return m_Trauma > 0.0; }

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    /// @brief  how much trauma bleeds off every second
    static constexpr double DECAY_PER_SECOND = 1.7;

    /// @brief  how many cells the view can be pushed at full trauma
    /// @note   two is about the limit before a terminal stops reading as a jolt and starts
    ///         reading as the whole screen falling over
    static constexpr int MAX_OFFSET = 2;

    /// @brief  the trauma a continuous shake is held at
    static constexpr double CONTINUOUS_TRAUMA = 0.85;

    /// @brief  how fast the noise is walked, higher is a faster rattle
    static constexpr double NOISE_SPEED = 24.0;

    //-----------------------------------------------------------------------------
    // Noise
    //-----------------------------------------------------------------------------

    /// @brief  smooth value noise along one dimension
    /// @param  t       where to sample
    /// @param  seed    picks an independent wobble, so x and y do not move together
    /// @return a value between -1 and 1
    static double Noise( double t, unsigned seed );

private:

    /// @brief  a repeatable pseudo random value for a whole numbered sample
    /// @param  sample  which sample
    /// @param  seed    which wobble
    /// @return a value between -1 and 1
    static double hash( long long sample, unsigned seed );

    /// @brief  how shaken things are, from 0 to 1
    double m_Trauma = 0.0;

    /// @brief  how far along the noise the shake has walked
    double m_Time = 0.0;

    /// @brief  how far the view is pushed this frame
    Vec2i m_Offset{ 0, 0 };

    /// @brief  whether the trauma is being held up rather than left to decay
    bool m_Continuous = false;

};

#endif //CAMERASHAKE_H
