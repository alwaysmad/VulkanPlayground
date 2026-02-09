// src/core/Application.hpp
#pragma once

class Application
{
	// 'Emperor' of the application
	// Is a singleton
private:
	// Passkey (THE CROWN)
    // A permission slip that only 'Application' can sign.
	// Only run() can crown an emperor
    class PassKey {
        friend class Application; // Only App can create this
        private: explicit PassKey() = default;
    };

	// The privy council
	static std::optional<Application> s_instance; // The throne
	Logger m_logger; // Herald and Scribe
	Settings m_settings; // Rulebook

	// The Imperial court
	// TODO more

	static Settings adjustSettings(/*TODO parse cli args*/);

public:
	// Delete copy/move to enforce singleton
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator= (const Application&) = delete;
	Application& operator= (Application&&) = delete;

	// start the application
	// 'the reign of the Emperor'
	static int run(/*TODO args */);

	// Access the instance / An audience
	[[nodiscard]] inline static const std::optional<Application>& instance() { return s_instance; }

	// Access logger / Emperor's Herald and Scribe
	[[nodiscard]] inline const Logger& logger() { return m_logger; }

	// Access Settings / Emperor's rulebook
	[[nodiscard]] inline const Settings& settings() { return m_settings; }

	// Public so std::optional can create and destroy
	// Since only 'run()' can make the Key, this is effectively private.
	explicit Application([[maybe_unused]] PassKey pk, Settings&& s) : m_settings(std::move(s)), m_logger(m_settings.logPath)
		{ m_logger.cInfo("Application started"); }
	~Application() { m_logger.cInfo("Application ended"); }
};
