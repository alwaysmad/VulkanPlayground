// src/core/ConsoleLogger.hpp
#pragma once

class cLogger
{
	// Is owned by Application and thus provides logging globally
	// 'Emperor's own Herald'
public:
	// No copy
	cLogger(const cLogger&) = delete;
	cLogger& operator= (const cLogger&) = delete;
	// Move is allowed
	cLogger(cLogger&&) noexcept = default;
	cLogger& operator= (cLogger&&) noexcept = default;

	// =========================================================================
	//  Logging functions: output to console
	// =========================================================================
	template<typename... Args> inline void debug (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_GRAY << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void info (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_WHITE << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void warn (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << COLOR_YELLOW << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}
	template<typename... Args> inline void error (std::format_string<Args...> fmt, Args&&... args) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		std::cerr << COLOR_RED << std::format(fmt, std::forward<Args>(args)...) << COLOR_RESET << "\n";
	}

	// --- Constructors ---
	explicit cLogger() = default;
	~cLogger() = default;

	// ANSI Color Codes
	static constexpr std::string_view COLOR_RED       = "\033[1;31m"; // for errors
	static constexpr std::string_view COLOR_YELLOW    = "\033[1;33m"; // for warnings
	static constexpr std::string_view COLOR_WHITE     = "\033[1;37m"; // for info
	static constexpr std::string_view COLOR_GRAY      = "\033[90m";	  // for debug 
	
	//static constexpr std::string_view STYLE_BOLD      = "\033[1m";
	//static constexpr std::string_view STYLE_UNDERLINE = "\033[4m";

	static constexpr std::string_view COLOR_RESET     = "\033[0m";  

private:
	mutable std::mutex m_mutex;
};
