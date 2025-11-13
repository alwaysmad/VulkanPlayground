#pragma once
#include <iostream>

constexpr auto DBG_COLOR_RED         = "\033[1;31m";
constexpr auto DBG_COLOR_YELLOW      = "\033[1;33m";
constexpr auto DBG_COLOR_WHITE       = "\033[1;37m";
constexpr auto DBG_STYLE_BOLD        = "\033[1m";
constexpr auto DBG_STYLE_UNDERLINE   = "\033[4m";

constexpr auto DBG_COLOR_RESET  = "\033[0m";

#ifdef ENABLE_DEBUG_LOGGING
	#define LOG_DEBUG(...) do { \
		std::cout \
			<< DBG_COLOR_WHITE \
			/* << "[DEBUG] " */ \
			<< __VA_ARGS__ \
			<< DBG_COLOR_RESET \
			<< std::endl; \
		} while (0)
#else
	#define LOG_DEBUG(...) ((void)0) // does nothing
#endif
