# 1. Define the executable with its "main" file
add_executable(simplevk main.cpp)

# 2. Add other source files to the target
target_sources(simplevk PRIVATE
	VulkanApplication.cpp
	GlfwContext.cpp
	VulkanInstance.cpp
	VulkanWindow.cpp
	VulkanDevice.cpp
	VulkanSwapchain.cpp
	VulkanPipeline.cpp
	# Add new .cpp files here in the future
	# MyNewFile.cpp
	# MyOtherFile.cpp
)

# 3. Add the compile definition
target_compile_definitions(simplevk PRIVATE
    # This line adds -DENABLE_DEBUG_LOGGING *only* when 
    # CMAKE_BUILD_TYPE is "Debug"
    $<$<CONFIG:Debug>:ENABLE_DEBUG_LOGGING>
)

target_include_directories(simplevk PRIVATE
	${vulkan-hpp_BINARY_DIR}
    # Add this! This tells the compiler to look in 'build/src' for headers
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(simplevk PRIVATE 
	Vulkan::Vulkan
	glfw
)

# --- ADD THIS SECTION ---
# Compile the shaders
add_slang_shader_target(
    CompileShaders
    SOURCES ${CMAKE_SOURCE_DIR}/shaders/triangle.slang
)

# Force 'simplevk' to wait for 'CompileShaders' to finish
add_dependencies(simplevk CompileShaders)
# ------------------------
