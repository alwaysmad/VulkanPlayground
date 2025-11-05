add_executable(simplevk main.cpp)

# 1. Manually add the include directory where vulkan.hpp is generated.
#    This is provided by the FetchContent variable ${vulkan-hpp_BINARY_DIR}.
target_include_directories(simplevk PRIVATE
    ${vulkan-hpp_BINARY_DIR}
)

# 2. Link *only* against the system Vulkan library.
#    The 'vulkan-hpp' target is INTERFACE and provides headers,
#    not a library file to link against.
target_link_libraries(simplevk PRIVATE 
    Vulkan::Vulkan
)
