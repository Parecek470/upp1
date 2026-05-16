#pragma once
#include <string>

enum class RunMode { Serial, Parallel };

struct AppConfig {
	std::string measurementsFileName;
	std::string stationsFileName;
	std::string mapFileName = "czmap.svg";
	RunMode mode = RunMode::Serial;

	

};