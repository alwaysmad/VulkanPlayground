# src/CMakeLists.txt

# 1. Define the executable
add_executable(simplevk main.cpp)

# 2. Add sources
target_sources(simplevk PRIVATE
	VulkanApplication.cpp
	VulkanInstance.cpp
	VulkanDevice.cpp
	VulkanSwapchain.cpp
	VulkanLoader.cpp
	ComputePipeline.cpp
	Computer.cpp
	GraphicsPipeline.cpp
	Renderer.cpp
)

# 3. Compile Definitions
target_compile_definitions(simplevk PRIVATE
	$<$<CONFIG:Debug>:ENABLE_DEBUG_LOGGING>
	VULKAN_HPP_NO_CONSTRUCTORS
	GLFW_INCLUDE_VULKAN
	GLM_FORCE_CXX20
	GLM_FORCE_DEPTH_ZERO_TO_ONE
	GLM_FORCE_RADIANS
)

# 4. Include Directories (CRITICAL FIX)
target_include_directories(simplevk PRIVATE
	${vulkan-hpp_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} # <--- FIX: Allows PCH to find DebugOutput.hpp
)

# 5. Link Libraries
target_link_libraries(simplevk PRIVATE 
	Vulkan::Vulkan
	glfw
	glm::glm
)

# 6. Precompiled Headers (PCH)
target_precompile_headers(simplevk PRIVATE
	# STL
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
	<cstring>
	<atomic>
	<cmath>          # NEW: For sin/cos
	<cstddef>        # NEW: For offsetof

	# Third Party
	<vulkan/vulkan_raii.hpp>
	<GLFW/glfw3.h>
	<glm/glm.hpp>
	<glm/gtc/packing.hpp>
	<glm/gtc/matrix_transform.hpp> # NEW: For lookAt/perspective

	# Project Local
	"DebugOutput.hpp" # Works now because of step 4!
)

# 7. Shaders
add_slang_shader_target(
	CompileShaders
	SOURCES ${CMAKE_SOURCE_DIR}/shaders/triangle.slang
	SOURCES ${CMAKE_SOURCE_DIR}/shaders/solver.slang
)
add_dependencies(simplevk CompileShaders)
