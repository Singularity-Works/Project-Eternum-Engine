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
#include <algorithm>
#include <typeindex>
#include <array>
#include <map>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <chrono>
#include <thread>
#include <functional>
#include <random>
#include <limits>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>


#ifdef _WIN32
    // windows defines min and max as macros, which breaks std::min and numeric_limits::max
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
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

inline bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
           std::equal(prefix.begin(), prefix.end(), str.begin());
}


inline std::string PrefixlessName(const std::type_index& type) {
    std::string name = type.name();

    // gcc mangles the name length onto the front
    size_t i = 0;
    while (i < name.size() && std::isdigit(static_cast<unsigned char>(name[i]))) {
        ++i;
    }
    name = name.substr(i);

    // msvc spells it out instead
    static const std::string keywords[] = { "class ", "struct ", "enum ", "union " };
    for (const std::string& keyword : keywords) {
        if (StartsWith(name, keyword)) {
            return name.substr(keyword.size());
        }
    }

    return name;
}


#endif //PCH_H
