cmake_minimum_required(VERSION 3.16)

# Set the base directory for FetchContent BEFORE project()
# This ensures it's set before any other paths are initialized.
# We set it as a CACHE variable to make it "sticky".
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/_deps CACHE PATH "Base directory for FetchContent")
message(STATUS "Setting path for dependencies to ${FETCHCONTENT_BASE_DIR}")

project(SimpleVulkan LANGUAGES CXX)

# Set the C++ standard to 20, as you're learning C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Good practice

# --- Set standard output directories ---
# Put executables in ${CMAKE_BINARY_DIR}/bin
# (e.g., build/bin/simplevk)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Put libraries in ${CMAKE_BINARY_DIR}/lib
# (e.g., build/lib/libmylib.so)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# ---------------------------------------

# --- Find Vulkan SDK ---
# This finds the Vulkan loader (libvulkan.so) and the base C headers
find_package(Vulkan REQUIRED)

# --- Automatically determine vulkan.hpp tag ---
# find_package(Vulkan) sets ${Vulkan_VERSION} (e.g., "1.4.304")
# The vulkan.hpp tags are prefixed with 'v'
set(VULKAN_HPP_GIT_TAG "v${Vulkan_VERSION}")
message(STATUS "Found Vulkan SDK version: ${Vulkan_VERSION}")
message(STATUS "Setting vulkan.hpp Git tag to: ${VULKAN_HPP_GIT_TAG}")

# --- Fetch vulkan.hpp (The C++ Bindings) ---
# We use FetchContent to download the header-only library
# This makes your project self-contained and avoids system installs
include(FetchContent)

# We are no longer setting SOURCE_DIR/BINARY_DIR here,
# as FETCHCONTENT_BASE_DIR should handle it.
FetchContent_Declare(
    vulkan-hpp
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp.git
    # Use the automatically determined tag
    GIT_TAG        ${VULKAN_HPP_GIT_TAG}
)
# This makes the vh_SOURCE_DIR variable available
FetchContent_MakeAvailable(vulkan-hpp)

# Add our "src" directory where the executable lives
add_subdirectory(src)
