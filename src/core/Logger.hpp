// src/core/Logger.hpp
#pragma once

class Logger
{
	// Is owned by Application and thus provides logging globally
	// 'Emperor's own Herald'
public:
	// No copy
	Logger(const Logger&) = delete;
	Logger& operator= (const Logger&) = delete;
	// Move is allowed
	Logger(Logger&&) noexcept = default;
	Logger& operator= (Logger&&) noexcept = default;

	// =========================================================================
	//  Logging functions: output to console
	// =========================================================================
	template<typename... Args> inline void cDebug (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_GRAY << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void cInfo (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_WHITE << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void cWarn (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_YELLOW << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void cError (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cerr << COLOR_RED << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	// =========================================================================
	//  Logging functions: output to file
	// =========================================================================
	template<typename... Args> inline void fDebug (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		m_file << "[Debug]" << std::format(fmt, std::forward<Args>(args)...) << "\n";
	}
	template<typename... Args> inline void fInfo (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		m_file << "[Info]" << std::format(fmt, std::forward<Args>(args)...) << "\n";
	}
	template<typename... Args> inline void fWarn (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		m_file << "[Warn]" << std::format(fmt, std::forward<Args>(args)...) << "\n";
	}
	template<typename... Args> inline void fError (std::format_string<Args...> fmt, Args&&... args)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		m_file << "[Error]" << std::format(fmt, std::forward<Args>(args)...) << "\n";
	}

	// --- Constructors ---
	explicit Logger()
	{
		// Cross-platform temporary directory
		// e.g., Linux: /tmp/SimpleVK.log
		// e.g., Windows: C:\Users\User\AppData\Local\Temp\SimpleVK.log
		const std::string appName = "SimpleVK"; //TODO get from Settings manager
		const auto logPath = std::filesystem::temp_directory_path() / (appName + ".log");

		m_file.open(logPath);
		cInfo("Outputting additional logs to '{}'", logPath.string());
	}
	~Logger() noexcept = default;

	// ANSI Color Codes
	static constexpr std::string_view COLOR_RED       = "\033[1;31m"; // for errors
	static constexpr std::string_view COLOR_YELLOW    = "\033[1;33m"; // for warnings
	static constexpr std::string_view COLOR_WHITE     = "\033[1;37m"; // for info
	static constexpr std::string_view COLOR_GRAY      = "\033[90m";	  // for debug 
	
	//static constexpr std::string_view STYLE_BOLD      = "\033[1m";
	//static constexpr std::string_view STYLE_UNDERLINE = "\033[4m";

	static constexpr std::string_view COLOR_RESET     = "\033[0m";  

private:
	std::ofstream m_file;
	std::mutex m_mutex;
};
