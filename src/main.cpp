// enable debug output
#define ENABLE_DEBUG_LOGGING

#include "VulkanApplication.hpp"
#include "DebugOutput.hpp"

int main( int /*argc*/, char ** /*argv*/ )
{
	try
	{
		// Create an VulkanApplication instance 
		// execute run() method 
		// then destroy instance as it's an anonymous object
		return VulkanApplication().run();
	}
	// The exception type for vulkan_raii.hpp is vk::SystemError
	catch ( const vk::SystemError& e )
	{
		std::cerr 
			<< DBG_COLOR_RED
			<< "Vulkan Error: "
			<< DBG_COLOR_RESET
		       	<< e.what() << std::endl;
	}
	catch ( const std::exception& e )
	{
		std::cerr
			<< DBG_COLOR_RED
			<< "Error: " 
			<< DBG_COLOR_RESET
			<< e.what() << std::endl;
	}
	catch ( ... )
	{
		std::cerr
			<< DBG_COLOR_RED
			<< "An unknown error occurred."
			<< DBG_COLOR_RESET
			<< std::endl;
	}

	return EXIT_FAILURE;
}
