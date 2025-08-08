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
#include <cstdlib>
#include <ctime>
#include <typeindex>


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
#include <Core/Vector/Vector.h>

// Global utility functions

inline unsigned GetUniqueId()
{
    static unsigned nextId = 0;
    return nextId++;
}

inline std::string PrefixlessName(const std::type_index& type) {
    const std::string name = type.name();

    size_t i = 0;
    while (i < name.size() && std::isdigit(name[i])) {
        ++i;
    }

    return name.substr(i);
}


inline bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
           std::equal(prefix.begin(), prefix.end(), str.begin());
}


#endif //PCH_H
