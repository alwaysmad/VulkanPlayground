// src/core/Application.cpp
#include <filesystem> // for std::filesystem::temp_directory_path

// Define the static Throne
std::optional<Application> Application::s_instance;

int Application::run(/*TODO parse cli args*/)
{
	// Crown a new emperor with a new rulebook
	s_instance.emplace(adjustSettings(/*TODO args */));

	// Declare start of the reign
	s_instance->m_logger.cInfo("Application name is {}", Settings::appName);

	// test the file output
	s_instance->m_logger.fInfo("blah blah in file");
	
	// end the reign gracefully
	s_instance.reset();

	// Report success to god
	return EXIT_SUCCESS;
}

Settings Application::adjustSettings(/*TODO parse cli args*/)
{
	// Create rulebook
	Settings s;
	
	// Adjust it according to god's will
		// Set a path for log file
		// 'give Scribe the parchment'
		// Cross-platform temporary directory
		// e.g., Linux: /tmp/SimpleVK.log
		// e.g., Windows: C:\Users\User\AppData\Local\Temp\SimpleVK.log
	s.logPath = std::filesystem::temp_directory_path() / ("rso.log");
	
	return s;
}
