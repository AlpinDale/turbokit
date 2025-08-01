cmake_minimum_required(VERSION 3.16)
project(TurboKit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TURBOKIT_BUILD_TESTS "Build TurboKit tests" OFF)
option(TURBOKIT_BUILD_EXAMPLES "Build TurboKit examples" OFF)
option(TURBOKIT_ENABLE_SANITIZERS "Enable sanitizers for debug builds" OFF)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)
FetchContent_MakeAvailable(fmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-pedantic -Wno-implicit-fallthrough")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

    # 128-bit atomic operations
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcx16")
    endif()

    if(TURBOKIT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")
    endif()
endif()

add_library(TurboKit INTERFACE)

target_include_directories(TurboKit INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(TurboKit INTERFACE fmt::fmt)

target_compile_definitions(TurboKit INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:__GNUC__>
    $<$<CXX_COMPILER_ID:Clang>:__clang__>
)

include(GNUInstallDirs)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS TurboKit
    EXPORT TurboKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT TurboKitTargets
    FILE TurboKitTargets.cmake
    NAMESPACE TurboKit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TurboKit
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    TurboKitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TurboKitConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TurboKitConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TurboKit
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TurboKitConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TurboKitConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TurboKit
)

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TurboKitConfig.cmake.in"
"@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
find_dependency(fmt REQUIRED)

include(\"\${CMAKE_CURRENT_LIST_DIR}/TurboKitTargets.cmake\")

check_required_components(TurboKit)
")

if(TURBOKIT_BUILD_TESTS)
    enable_testing()
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(TurboKitTests
        tests/test_serialization.cpp
        tests/test_vector.cpp
        tests/test_hash_map.cpp
        tests/test_buffer.cpp
        tests/test_sync.cpp
        tests/test_clock.cpp
        tests/test_freelist.cpp
        tests/test_logging.cpp
    )

    target_link_libraries(TurboKitTests PRIVATE TurboKit gtest gtest_main)

    add_test(NAME TurboKitTests COMMAND TurboKitTests)
endif()

if(TURBOKIT_BUILD_EXAMPLES)
    add_executable(TurboKitExamples
        examples/serialization_example.cpp
        examples/performance_example.cpp
        examples/logging_example.cpp
    )

    target_link_libraries(TurboKitExamples PRIVATE TurboKit)
endif()

message(STATUS "TurboKit Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Tests: ${TURBOKIT_BUILD_TESTS}")
message(STATUS "  Build Examples: ${TURBOKIT_BUILD_EXAMPLES}")
message(STATUS "  Enable Sanitizers: ${TURBOKIT_ENABLE_SANITIZERS}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}") 