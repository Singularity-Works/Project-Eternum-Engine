/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: pch
* Description:
*    Precompiled header for the Eternum Engine, including standard libraries
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#pragma once
#ifndef PCH_H
#define PCH_H

// Standard Library
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <functional>
#include <random>
#include <chrono>
#include <thread>

#ifdef _WIN32
   #include <Windows.h>
    #include <conio.h>
#else // ifdef _WIN32
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
#endif // ifdef _WIN32

// Internal headers
#include <Systems/system.h>

#endif //PCH_H
