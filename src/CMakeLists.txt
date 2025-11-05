add_executable(simplevk main.cpp)

target_include_directories(simplevk PRIVATE
    ${vulkan-hpp_BINARY_DIR}
)

target_link_libraries(simplevk PRIVATE 
    Vulkan::Vulkan
)
