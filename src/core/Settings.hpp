// src/core/Settings.hpp
#pragma once

struct Settings
{
public:
	// Emperor's rulebook
	static constexpr std::string_view appName = "RSO";
	static constexpr std::string_view logName = "rso.log";

	Settings() noexcept = default;
	~Settings() noexcept = default;

	// Both copy and move are allowed
	Settings(const Settings&) noexcept = default;
	Settings& operator= (const Settings&) noexcept = default;
	Settings(Settings&&) noexcept = default;
	Settings& operator= (Settings&&) noexcept = default;
};
