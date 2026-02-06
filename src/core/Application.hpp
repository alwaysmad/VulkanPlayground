// src/core/Application.hpp
#pragma once

#include "Logger.hpp"

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
	inline Logger& logger() { return m_logger; }

private:
	Logger m_logger;

	// Private constructor, only instance() can create it
	Application() : m_logger()
	{
		m_logger.cInfo("Application started");
	}
	~Application()
	{
		m_logger.cInfo("Application ended");
	}
};
