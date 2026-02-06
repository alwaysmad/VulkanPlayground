// src/main.cpp

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	try
	{
		return Application::instance().run(/* TODO pass args */); // returns EXIT_SUCCESS
	}
	catch ( const std::exception& e )
	{
		std::cerr << cLogger::COLOR_RED << "Error: " << e.what() << cLogger::COLOR_RESET << std::endl;
	}
	catch ( ... )
	{
		std::cerr << cLogger::COLOR_RED << "An unknown error occurred." << cLogger::COLOR_RESET << std::endl;
	}
	return EXIT_FAILURE; // If error occured 
}
