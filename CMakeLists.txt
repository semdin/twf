cmake_minimum_required(VERSION 3.25)
project(twf VERSION 0.1.0 LANGUAGES C CXX)

# Enforce Modern C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler Warnings and Flags for Clang / GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-c++26-extensions
    )
endif()

# Automated CPM (CMake Package Manager) Setup
set(CPM_DOWNLOAD_VERSION 0.40.2)
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake"
        STATUS download_status
    )
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(WARNING "Could not download CPM.cmake. Local packages will be used if available.")
    endif()
endif()
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    include("${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

# -------------------------------------------------------------
# Core Framework Library: twf
# -------------------------------------------------------------
add_library(twf STATIC
    src/core/buffer.cpp
    src/server/server.cpp
)

target_include_directories(twf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# On Windows (MinGW/Clang), link Windows Sockets (ws2_32)
if(WIN32)
    target_link_libraries(twf PUBLIC ws2_32 mswsock)
endif()

# -------------------------------------------------------------
# Optional Third-Party: Glaze (High-speed C++23 JSON reflection)
# -------------------------------------------------------------
if(COMMAND CPMAddPackage)
    CPMAddPackage(
        NAME glaze
        GITHUB_REPOSITORY stephenberry/glaze
        GIT_TAG v4.3.0
        OPTIONS "glaze_ENABLE_AVX2 OFF"
    )
    if(TARGET glaze::glaze)
        target_link_libraries(twf PUBLIC glaze::glaze)
    endif()
endif()

# -------------------------------------------------------------
# Examples & Applications
# -------------------------------------------------------------
add_subdirectory(examples/hello_server)

# -------------------------------------------------------------
# Test Suite
# -------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

