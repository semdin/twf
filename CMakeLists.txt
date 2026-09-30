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

# -------------------------------------------------------------
# Core Framework Library: twf
# -------------------------------------------------------------
add_library(twf STATIC
    src/core/buffer.cpp
    src/server/server.cpp
)

# Standard alias for consumer targets (twf::twf)
add_library(twf::twf ALIAS twf)

target_include_directories(twf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# On Windows (MinGW/Clang), link Windows Sockets (ws2_32)
if(WIN32)
    target_link_libraries(twf PUBLIC ws2_32 mswsock)
endif()

# -------------------------------------------------------------
# Installation & Package Export (find_package / CPM / vcpkg)
# -------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install Library Target
install(TARGETS twf
    EXPORT twfTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install Header Files
install(DIRECTORY include/twf
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export Set
install(EXPORT twfTargets
    FILE twfTargets.cmake
    NAMESPACE twf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/twf
)

# Generate Version File with Semantic Versioning (SameMajorVersion)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/twfConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate Config File
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/twfConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/twfConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/twf
)

# Install Config and Version Files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/twfConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/twfConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/twf
)

# -------------------------------------------------------------
# Third-Party: Glaze (High-speed C++23 JSON reflection)
# -------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG v4.3.0
)
FetchContent_MakeAvailable(glaze)
target_link_libraries(twf PUBLIC glaze::glaze)

# -------------------------------------------------------------
# Examples & Applications (Only when built as root project)
# -------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(examples/hello_server)

    # Test Suite
    enable_testing()
    add_subdirectory(tests)
endif()

