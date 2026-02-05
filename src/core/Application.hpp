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

private:
	// Private constructor, only instance() can create it
	Application()
	{
		// log
	}
	~Application()
	{
		// log
	}
};
