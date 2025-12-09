# 1. Define the executable with its "main" file
add_executable(simplevk main.cpp)

# 2. Add other source files to the target
target_sources(simplevk PRIVATE
	VulkanApplication.cpp
	GlfwContext.cpp
	VulkanInstance.cpp
	VulkanWindow.cpp
	VulkanDevice.cpp
	VulkanSync.cpp
	VulkanSwapchain.cpp
	VulkanPipeline.cpp
	VulkanRender.cpp
	# Add new .cpp files here in the future
	# MyNewFile.cpp
)

# 3. Add the compile definition
target_compile_definitions(simplevk PRIVATE
	# This line adds -DENABLE_DEBUG_LOGGING *only* when 
	# CMAKE_BUILD_TYPE is "Debug"
	$<$<CONFIG:Debug>:ENABLE_DEBUG_LOGGING>
	# These will now apply to PCH and every source file automatically
    	VULKAN_HPP_NO_CONSTRUCTORS
    	GLFW_INCLUDE_VULKAN
)

target_include_directories(simplevk PRIVATE
	${vulkan-hpp_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(simplevk PRIVATE 
	Vulkan::Vulkan
	glfw
)

target_precompile_headers(simplevk PRIVATE
	<vulkan/vulkan_raii.hpp>
	<GLFW/glfw3.h>
	<vector>
	<array>
	<iostream>
	<stdexcept>
	<cstdlib>
	<set>
	<string>
	<fstream>
	<limits>
	<algorithm>
)

# Compile the shaders
add_slang_shader_target(
	CompileShaders
	SOURCES ${CMAKE_SOURCE_DIR}/shaders/triangle.slang
)

# Force 'simplevk' to wait for 'CompileShaders' to finish
add_dependencies(simplevk CompileShaders)
# ------------------------
