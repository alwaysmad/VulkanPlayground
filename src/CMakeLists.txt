# 1. Define the executable with its "main" file
add_executable(simplevk main.cpp)

# 2. Add other source files to the target
target_sources(simplevk PRIVATE
    VulkanApplication.cpp
    # Add new .cpp files here in the future
    # MyNewFile.cpp
    # MyOtherFile.cpp
)

target_include_directories(simplevk PRIVATE
    ${vulkan-hpp_BINARY_DIR}
)

target_link_libraries(simplevk PRIVATE 
    Vulkan::Vulkan
)
