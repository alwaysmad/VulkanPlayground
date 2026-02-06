// src/main.cpp
#include "core/Application.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	try
	{
		return Application::instance().run(); // returns EXIT_SUCCESS
	}
	catch ( const std::exception& e )
	{
		std::cerr << Logger::COLOR_RED << "Error: " << e.what() << Logger::COLOR_RESET << std::endl;
	}
	catch ( ... )
	{
		std::cerr << "An unknown error occurred." << std::endl;
	}
	return EXIT_FAILURE; // If error occured 
}
