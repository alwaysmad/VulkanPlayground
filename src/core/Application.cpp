// src/core/Application.cpp

int Application::run()
{
	m_logger.cInfo("Application name is {}", Settings::appName);
	m_logger.fInfo("blah blah in file");
	return EXIT_SUCCESS;
}
