cmake_minimum_required(VERSION 3.16)
project(EternumEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(
        "Eternum Engine/src"
        "Eternum Engine/src/Core/pch"
        "Eternum Engine/src/third_party"
)

# Gather source files
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS "Eternum Engine/src/*.cpp" "Eternum Engine/src/*.h")
file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS "tests/*.cpp")

# Main executable
add_executable(eternum-engine ${ENGINE_SRC})

# Precompiled headers for main executable
target_precompile_headers(eternum-engine PRIVATE "Eternum Engine/src/Core/pch/pch.h")

# Unit tests
enable_testing()
add_executable(eternum-tests ${TEST_SRC} ${ENGINE_SRC})
target_include_directories(eternum-tests PRIVATE "Eternum Engine/src/third_party")

# Precompiled headers for tests
target_precompile_headers(eternum-tests PRIVATE "Eternum Engine/src/Core/pch/pch.h")

add_test(NAME EternumTests COMMAND eternum-tests)
