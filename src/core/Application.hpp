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

	// Access console logger / Emperor's Herald
	[[nodiscard]] inline const cLogger& Clogger() { return m_cLogger; }

	// Access Settings / Emperor's rulebook
	[[nodiscard]] inline const Settings& settings() { return m_settings; }

	// Access file logger / Emperor's Scribe
	[[nodiscard]] inline const fLogger& Flogger() { return *m_fLogger; }

private:
	// The privy council
	cLogger m_cLogger; // Herald
	Settings m_settings; // Rulebook

	// The Imperial court
	std::optional<fLogger> m_fLogger; // Scribe
	// TODO more

	void adjustSettings(/*TODO parse cli args*/);

	// Private constructor, only instance() can create it
	Application() { m_cLogger.info("Application started"); }
	~Application() { m_cLogger.info("Application ended"); }
};
