cmake_minimum_required(VERSION 3.16)
project(EternumEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force console subsystem for Windows
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()
# On Unix-like OS, no change is needed

# --------------------------------------------------------
# Include directories
# --------------------------------------------------------
include_directories(
        "Eternum Engine/src"
        "Eternum Engine/src/Core/pch"
        "Eternum Engine/third_party/googletest/googletest/include"
)

# --------------------------------------------------------
# Source files
# --------------------------------------------------------
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
        "Eternum Engine/src/*.cpp"
        "Eternum Engine/src/*.h"
)

file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS
        "Unit Tests/*.cpp"
)

# Remove main.cpp from engine sources for the library
list(FILTER ENGINE_SRC EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Eternum Engine/src/main\\.cpp$")

# --------------------------------------------------------
# Engine library (shared so both main & tests use it)
# --------------------------------------------------------
add_library(eternum-engine-lib ${ENGINE_SRC})
target_precompile_headers(eternum-engine-lib PRIVATE "Eternum Engine/src/Core/pch/pch.h")

# --------------------------------------------------------
# Main executable
# --------------------------------------------------------
add_executable(eternum-engine "Eternum Engine/src/main.cpp")
target_link_libraries(eternum-engine PRIVATE eternum-engine-lib)

# --------------------------------------------------------
# GoogleTest
# --------------------------------------------------------
add_subdirectory("Eternum Engine/third_party/googletest")

# --------------------------------------------------------
# Unit tests executable
# --------------------------------------------------------
enable_testing()
add_executable(eternum-tests ${TEST_SRC})
target_link_libraries(eternum-tests PRIVATE eternum-engine-lib gtest gtest_main)
target_precompile_headers(eternum-tests PRIVATE "Eternum Engine/src/Core/pch/pch.h")

# --------------------------------------------------------
# Automatically discover GoogleTest tests
# --------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(eternum-tests)
