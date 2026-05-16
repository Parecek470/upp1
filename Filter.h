#pragma once
#include <vector>
#include "Structs.h"

bool hasFiveContinuousYears(const StationData& sd);
bool hasEnoughReadingsPerYear(const StationData& sd);

std::vector<StationData> filterStations(const std::vector<StationData> dataset);