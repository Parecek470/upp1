#pragma once


#include <vector>
#include "Structs.h"
#include "AppConfig.h"
#include <string>

std::vector<Station> loadStations(const std::string& fileName);
std::vector<Measurement> loadMeasurements(const std::string& fileName, RunMode mode);
std::vector<StationData> buildDataset(const std::vector<Station>& stations, const std::vector<Measurement>& measurements, RunMode mode);

