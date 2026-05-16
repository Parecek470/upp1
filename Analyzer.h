#pragma once
#include <vector>
#include <unordered_map>
#include "Structs.h"


struct Anomaly {
	int stationId;
	int month;
	int year;
	float difference;
};

std::pair<float, float> computeGlobalMinMax(const std::vector<StationData>& dataset);
std::unordered_map<int, float> calculateMonthlyAverages(const StationData& sd);
std::vector<Anomaly> detectAnomalies(const StationData& sd, const std::unordered_map<int, float>& monthlyAvg);