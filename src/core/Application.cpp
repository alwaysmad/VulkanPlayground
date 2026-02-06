// src/core/Application.cpp
#include <filesystem> // for std::filesystem::temp_directory_path

int Application::run(/*TODO parse cli args*/)
{
	// Declare start of the reign
	m_cLogger.info("Application name is {}", Settings::appName);

	// Recieve divine revelation
	adjustSettings(/* */);

	// appoint Scribe
	m_fLogger.emplace(m_settings.logPath);

	// test the Scribe
	m_fLogger->info("blah blah in file");
	
	// Report success to god
	return EXIT_SUCCESS;
}

void Application::adjustSettings(/*TODO parse cli args*/)
{
	// Set a path for log file
	// 'give Scribe the parchment'
	// Cross-platform temporary directory
	// e.g., Linux: /tmp/SimpleVK.log
	// e.g., Windows: C:\Users\User\AppData\Local\Temp\SimpleVK.log
	m_settings.logPath = std::filesystem::temp_directory_path() / ("rso.log");
	
}
