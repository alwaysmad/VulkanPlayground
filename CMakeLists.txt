cmake_minimum_required(VERSION 3.16)

# Set the base directory for FetchContent BEFORE project()
# This ensures it's set before any other paths are initialized.
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/_deps CACHE PATH "Base directory for FetchContent")

project(SimpleVulkan LANGUAGES CXX)

# --- Set Build Type and Flags ---
# Set a default build type if none is specified (e.g., Debug)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type: Debug, Release, RelWithDebInfo, MinSizeRel")
endif()

# Set standard compiler flags for Release and Debug builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Print the build type being used
message(STATUS "Building project with build type: ${CMAKE_BUILD_TYPE}")
# --------------------------------

# Set the C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Good practice

# --- Add Project-Wide Compiler Warnings ---
# These flags are great for catching bugs and ensuring code quality.
# -Wall: Enable all standard warnings
# -Wextra: Enable extra (but still useful) warnings
# -Wpedantic: Warn on non-standard C++ extensions
add_compile_options(-Wall -Wextra -Wpedantic)
# ------------------------------------------

# --- Set standard output directories ---
# Put executables in ${CMAKE_BINARY_DIR}/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Put libraries in ${CMAKE_BINARY_DIR}/lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# ---------------------------------------

# --- Find Vulkan SDK ---
find_package(Vulkan REQUIRED)

# --- Automatically determine vulkan.hpp tag ---
set(VULKAN_HPP_GIT_TAG "v${Vulkan_VERSION}")
message(STATUS "Found Vulkan SDK version: ${Vulkan_VERSION}")
message(STATUS "Setting vulkan.hpp Git tag to: ${VULKAN_HPP_GIT_TAG}")

# --- Fetch vulkan.hpp (The C++ Bindings) ---
include(FetchContent)

FetchContent_Declare(
    vulkan-hpp
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp.git
    GIT_TAG        ${VULKAN_HPP_GIT_TAG}
)
FetchContent_MakeAvailable(vulkan-hpp)

# Add our "src" directory where the executable lives
add_subdirectory(src)
