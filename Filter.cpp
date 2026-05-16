#include "Filter.h"
#include <algorithm>
#include <set>

bool hasFiveContinuousYears(const StationData& sd) {
	if (sd.measurements.empty()) return false;
	
	std::set<int> years;
	for (const auto& m : sd.measurements) {
		years.insert(m.year);
	}

	int count = 1;
	for (auto it = years.begin(); it != years.end(); ++it) {
		if (it != years.begin() && *it == *(std::prev(it)) + 1)
		{
			count++;
			if (count >= 5) return true;
		}
		else {
			count = 1;
		}
	}

	return false;
}

bool hasEnoughReadingsPerYear(const StationData& sd) {
	if (sd.measurements.empty()) return false;

	std::set<int> years;
	for (const auto& m : sd.measurements) {
		years.insert(m.year);
	}

	return (double)sd.measurements.size() / (double)years.size() >= 100;
}

std::vector<StationData> filterStations(std::vector<StationData> dataset) {
	std::erase_if(dataset, [](const StationData& sd) {
		return !hasFiveContinuousYears(sd) || !hasEnoughReadingsPerYear(sd);
		});
	return dataset;
}