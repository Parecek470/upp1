#pragma once
#include <string>
#include <vector>
#include "Analyzer.h"


void writeAnomalies(const std::vector<Anomaly>& anomalies, const std::string& fileName);
void generateSVG(const std::vector<StationData>& data, int month, std::pair<float, float> minMax, const std::string& outputPath);