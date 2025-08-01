cmake_minimum_required(VERSION 3.16)
project(TurboKitExample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use the parent directory's TurboKit configuration
add_subdirectory(.. turbokit_build EXCLUDE_FROM_ALL)

# Build the TurboKit example
add_executable(turbokit_example turbokit_example.cpp)
target_link_libraries(turbokit_example PRIVATE TurboKit)

# Compiler flags for better performance and warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(turbokit_example PRIVATE 
        -Wall -Wextra -Wpedantic 
        -O2 
        -march=native
    )
endif()

# Enable debug info in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(turbokit_example PRIVATE -g)
endif()