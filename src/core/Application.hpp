// src/core/Application.hpp
#pragma once

class Application
{
	// 'Emperor' of the application
	// Is a singleton
public:
	// Delete copy/move to enforce singleton
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator= (const Application&) = delete;
	Application& operator= (Application&&) = delete;

	// Meyers' Singleton
	inline static Application& instance() { static Application app; return app; }
	
	// start the application
	// 'the reign of the Emperor'
	int run();

	// Access Logger / Emperor's Herald
	[[nodiscard]] inline const Logger& logger() { return m_logger; }

	// Access Settings / Emperor's rulebook
	[[nodiscard]] inline const Settings& settings() { return m_settings; }

private:
	Logger m_logger;
	Settings m_settings;

	// Private constructor, only instance() can create it
	Application() : m_logger(), m_settings()
	{
		m_logger.cInfo("Application started");
	}
	~Application()
	{
		m_logger.cInfo("Application ended");
	}
};
