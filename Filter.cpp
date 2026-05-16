#include "Filter.h"
#include <algorithm>
#include <set>
#include <omp.h>

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

static std::vector<StationData> filterStationsSerial(std::vector<StationData> dataset) {
	std::erase_if(dataset, [](const StationData& sd) {
		return !hasFiveContinuousYears(sd) || !hasEnoughReadingsPerYear(sd);
		});
	return dataset;
}

static std::vector<StationData> filterStationsParallel(const std::vector<StationData>& dataset) {
	int n = (int)dataset.size();
	std::vector<std::vector<StationData>> threadResults(omp_get_max_threads());

#pragma omp parallel for schedule(static)
	for (int i = 0; i < n; i++) {
		if (hasFiveContinuousYears(dataset[i]) && hasEnoughReadingsPerYear(dataset[i])) {
			threadResults[omp_get_thread_num()].push_back(dataset[i]);
		}
	}

	std::vector<StationData> result;
	for (auto& vec : threadResults)
		result.insert(result.end(), vec.begin(), vec.end());

	return result;
}

std::vector<StationData> filterStations(const std::vector<StationData>& dataset, RunMode mode) {
	if (mode == RunMode::Parallel)
		return filterStationsParallel(dataset);
	else
		return filterStationsSerial(dataset);
}