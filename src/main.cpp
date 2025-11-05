// No VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1 needed
// We will use the functions linked from the Vulkan SDK (Vulkan::Vulkan)

#include <vulkan/vulkan.hpp>
// #include <vulkan/vulkan_dynamic.hpp>  // <--- REMOVE THIS LINE
#include <iostream>
#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE

int main() {
    try {
        // --- 1. Initialize the Dynamic Loader ---
        // We no longer need this section!
        // vk::DynamicLoader dl;
        // VULKAN_HPP_DEFAULT_DISPATCHER.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        // --- 2. Define Application Info ---
        // This tells Vulkan about our application
        vk::ApplicationInfo appInfo(
            "SimpleVK",             // Application Name
            VK_MAKE_VERSION(1, 0, 0), // Application Version
            "No Engine",            // Engine Name
            VK_MAKE_VERSION(1, 0, 0), // Engine Version
            VK_API_VERSION_1_3        // Vulkan API Version
        );

        // --- 3. Define Instance Create Info ---
        // This tells Vulkan which extensions and layers we want
        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &appInfo;

        // (We aren't enabling any extensions or layers for this simple example)

        // --- 4. Create the Vulkan Instance ---
        // We use createInstanceUnique to get an RAII-style handle.
        // vk::UniqueInstance will automatically call vkDestroyInstance
        // when it goes out of scope at the end of the 'try' block.
        // This is the core of the C++ wrapper's benefit!
        vk::UniqueInstance instance = vk::createInstanceUnique(createInfo);

        // If vk::createInstanceUnique fails, it throws an exception which
        // we will catch below. If it succeeds, we print a message.
        std::cout << "Vulkan Instance created successfully!" << std::endl;

    } catch (const vk::Error& e) {
        // Catch any Vulkan-specific exceptions
        std::cerr << "Vulkan Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        // Catch other standard C++ exceptions
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        // Catch any other unknown errors
        std::cerr << "An unknown error occurred." << std::endl;
        return EXIT_FAILURE;
    }

    // If the 'try' block completes without errors, we return success
    return EXIT_SUCCESS;
}
