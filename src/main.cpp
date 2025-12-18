#include "VulkanApplication.hpp"

constexpr auto AppName = "SimpleVK";

constexpr auto DeviceName = "Intel(R) Iris(R) Xe Graphics (ADL GT2)";

static constexpr uint32_t WIDTH  = 800;
static constexpr uint32_t HEIGHT = 600;

int main(/* int argc, char** argv*/ )
{
	/*try
	{
		std::string DeviceName;

		// Check if user provided a device name via CLI
		if (argc > 1)
		{
			DeviceName = argv[1];
			LOG_DEBUG("User requested device: " << DeviceName);
		}
		else
			{ throw std::runtime_error("Wrong CLI argumnets -- device name expected."); }

		// Pass it to the application
		return VulkanApplication(AppName, DeviceName).run();
	}*/
	try
	{
		// Create an VulkanApplication instance 
		// execute run() method 
		// then destroy the instance as it's an anonymous object
		return VulkanApplication(AppName, DeviceName, WIDTH, HEIGHT).run();
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
