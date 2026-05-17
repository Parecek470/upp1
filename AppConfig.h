#pragma once
#include <string>

enum class RunMode { Serial, Parallel };

struct AppConfig {
	std::string stationsFileName;
	std::string measurementsFileName;
	std::string mapFileName = "czmap.svg";
	RunMode mode = RunMode::Serial;

	

};
