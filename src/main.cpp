#include <vulkan/vulkan.hpp>
#include <iostream>
#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE

int main( int /*argc*/, char ** /*argv*/ )
{
	try
	{
		// create an ApplicationInfo object that is required for Instance
		vk::ApplicationInfo appInfo (
			"SimpleVK",             // Application Name
			VK_MAKE_VERSION( 1, 0, 0 ), // Application Version
			"No Engine",            // Engine Name
			VK_MAKE_VERSION( 1, 0, 0 ), // Engine Version
			VK_API_VERSION_1_3 // Vulkan API Version
		);

		// create Instance
		vk::UniqueInstance instance = vk::createInstanceUnique (
			// Pass an anonymous vk::InstanceCreateInfo struct
			vk::InstanceCreateInfo (
				{}, // flags
				&appInfo
				// No extensions or layers for this simple example
			)
		);

		// If vk::createInstanceUnique fails, it throws an exception which
		std::cout << "Vulkan Instance created successfully!" << std::endl;
	}
	catch ( const vk::Error &e )
	{
		// Catch any Vulkan-specific exceptions
		std::cerr << "Vulkan Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch ( const std::exception &e )
	{
		// Catch other standard C++ exceptions
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch ( ... )
	{
		// Catch any other unknown errors
		std::cerr << "An unknown error occurred." << std::endl;
		return EXIT_FAILURE;
	}

	// If the 'try' block completes without errors, return success
	return EXIT_SUCCESS;
}
