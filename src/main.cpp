// src/main.cpp
#include "core/Application.hpp"

int main(int argc, char** argv)
{
	try
	{
		return Application::instance().run(); // returns EXIT_SUCCESS
	}
	// The exception type for vulkan_raii.hpp is vk::SystemError
	catch ( const vk::SystemError& e )
	{
		std::cerr << "Vulkan Error: " << e.what() << std::endl;
	}
	// catch stl exceptions
	catch ( const std::exception& e )
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	// catch other exceptions
	catch ( ... )
	{
		std::cerr << "An unknown error occurred." << std::endl;
	}
	return EXIT_FAILURE; // If error occured 
}
