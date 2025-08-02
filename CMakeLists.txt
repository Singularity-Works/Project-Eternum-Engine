cmake_minimum_required(VERSION 3.16)
project(EternumEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(src/third_party)

# Gather source files
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS tests/*.cpp)

# Main executable
add_executable(eternum-engine ${ENGINE_SRC})

# Unit tests
enable_testing()
add_executable(eternum-tests ${TEST_SRC} ${ENGINE_SRC})
target_include_directories(eternum-tests PRIVATE src/third_party)
add_test(NAME EternumTests COMMAND eternum-tests)
