#include "Filter.h"
#include <algorithm>
#include <map>
#include <set>
#include <omp.h>

static bool passesFilter(const StationData& sd) {
	if (sd.measurements.empty()) return false;

	std::map<int, int> yearCount;
	for (const auto& m : sd.measurements)
		yearCount[m.year]++;

	// check average readings per year >= 100
	int totalReadings = (int)sd.measurements.size();
	if ((double)totalReadings / (double)yearCount.size() < 100)
		return false;

	// check 5 continuous years
	int count = 1;
	for (auto it = std::next(yearCount.begin()); it != yearCount.end(); ++it)
		if (it->first == std::prev(it)->first + 1) {
			if (++count >= 5) return true;
		}
		else {
			count = 1;
		}

	return false;
}

static std::vector<StationData> filterStationsSerial(std::vector<StationData> dataset) {
	std::erase_if(dataset, [](const StationData& sd) {
		return !passesFilter(sd);
		});
	return dataset;
}

static std::vector<StationData> filterStationsParallel(const std::vector<StationData>& dataset) {
	int n = (int)dataset.size();
	std::vector<std::vector<StationData>> threadResults(omp_get_max_threads());

#pragma omp parallel for schedule(static)
	for (int i = 0; i < n; i++) {
		if (passesFilter(dataset[i]))
			threadResults[omp_get_thread_num()].push_back(dataset[i]);
	}

	size_t total = 0;
	for (auto& vec : threadResults) total += vec.size();

	std::vector<StationData> result;
	result.reserve(total);
	for (auto& vec : threadResults)
		result.insert(result.end(),
			std::make_move_iterator(vec.begin()),
			std::make_move_iterator(vec.end()));

	return result;
}

std::vector<StationData> filterStations(const std::vector<StationData>& dataset, RunMode mode) {
	if (mode == RunMode::Parallel)
		return filterStationsParallel(dataset);
	else
		return filterStationsSerial(dataset);
}