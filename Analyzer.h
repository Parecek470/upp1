#pragma once
#include <vector>
#include <unordered_map>
#include "Structs.h"
#include "AppConfig.h"


struct Anomaly {
	int stationId;
	int month;
	int year;
	float difference;
};


std::pair<float, float> computeGlobalMinMax(const std::vector<StationData>& dataset, RunMode mode);
void calculateMonthlyAverages(std::vector<StationData>& dataset, RunMode mode);
std::vector<Anomaly> detectAnomalies(const std::vector<StationData>& dataset, RunMode mode);