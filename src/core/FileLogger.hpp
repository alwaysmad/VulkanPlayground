// src/core/FileLogger.hpp
#pragma once

class fLogger
{
	// Is owned by Application and thus provides logging globally
	// 'Emperor's own Scribe'
public:
	// No copy
	fLogger(const fLogger&) = delete;
	fLogger& operator= (const fLogger&) = delete;
	// Move is allowed
	fLogger(fLogger&&) noexcept = default;
	fLogger& operator= (fLogger&&) noexcept = default;

	// =========================================================================
	//  Logging functions: output to file
	// =========================================================================
	template<typename... Args> inline void debug (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		if (m_file.is_open())
			{ m_file << "[Debug]" << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
		else
			{ std::cout << cLogger::COLOR_GRAY << std::format(fmt, std::forward<Args>(args)...) << cLogger::COLOR_RESET << "\n"; }
	}
	template<typename... Args> inline void info (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		if (m_file.is_open())
			{ m_file << "[Info]" << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
		else
			{ std::cout << cLogger::COLOR_WHITE << std::format(fmt, std::forward<Args>(args)...) << cLogger::COLOR_RESET << "\n"; }
	}
	template<typename... Args> inline void warn (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		if (m_file.is_open())
			{ m_file << "[Warn]" << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
		else
			{ std::cout << cLogger::COLOR_YELLOW << std::format(fmt, std::forward<Args>(args)...) << cLogger::COLOR_RESET << "\n"; }
	}
	template<typename... Args> inline void error (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		if (m_file.is_open())
			{ m_file << "[Error]" << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
		else
			{ std::cerr << cLogger::COLOR_RED << std::format(fmt, std::forward<Args>(args)...) << cLogger::COLOR_RESET << "\n"; }
	}

	// --- Constructors ---
	explicit fLogger(const std::string& logPath)
	{
		try { m_file.open(logPath); }
		catch (...)
		{
			std::cout << cLogger::COLOR_YELLOW << "Failed to open " << logPath << " for file logs." << cLogger::COLOR_RESET << "\n";
			std::cout << cLogger::COLOR_YELLOW << "File logs will be directed to console." << cLogger::COLOR_RESET << "\n";
		}
	}
	~fLogger() = default;

private:
	mutable std::ofstream m_file;
	mutable std::mutex m_mutex;
};
