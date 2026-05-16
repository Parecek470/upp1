#include "Analyzer.h"
#include "AppConfig.h"
#include <algorithm>
#include <cfloat>
#include <omp.h>
#include <algorithm>


struct MonthlyData {
	float sum = 0.0f;
	int count = 0;
};



static std::pair<float, float> computeGlobalMinMaxSerial(const std::vector<StationData>& dataset) {
	std::pair<float, float> result = { FLT_MAX, -FLT_MAX };
	for (const auto& sd : dataset) {
		for (const auto& m : sd.measurements) {
			result.first = std::min(m.value, result.first);
			result.second = std::max(m.value, result.second);
		}
	}
	return result;
}

static std::pair<float, float> computeGlobalMinMaxParallel(const std::vector<StationData>& dataset) {
	float globalMin = FLT_MAX;
	float globalMax = -FLT_MAX;

#pragma omp parallel
	{
		float localMin = FLT_MAX;
		float localMax = -FLT_MAX;

#pragma omp for schedule(static)
		for (int i = 0; i < (int)dataset.size(); i++) {
			for (const auto& m : dataset[i].measurements) {
				if (m.value < localMin) localMin = m.value;
				if (m.value > localMax) localMax = m.value;
			}
		}

#pragma omp critical
		{
			if (localMin < globalMin) globalMin = localMin;
			if (localMax > globalMax) globalMax = localMax;
		}
	}
	return { globalMin, globalMax };
}


std::pair<float, float> computeGlobalMinMax(const std::vector<StationData>& dataset, RunMode mode) {
	if (mode == RunMode::Parallel)
		return computeGlobalMinMaxParallel(dataset);
	else
		return computeGlobalMinMaxSerial(dataset);
}

static std::unordered_map<int, float> calculateMonthlyAverages(const StationData& sd) {
	std::unordered_map<int, MonthlyData> monthlyData;
	for (const auto& m : sd.measurements) {
		int key = m.year * 100 + m.month;
		monthlyData[key].sum += m.value;
		monthlyData[key].count++;
	}
	
	std::unordered_map<int, float> averages;
	for (const auto& pair : monthlyData) {
		averages[pair.first] = pair.second.sum / pair.second.count;
	}
	return averages;
}

void calculateMonthlyAverages(std::vector<StationData>& dataset, RunMode mode) {

	if (mode == RunMode::Parallel) {
		int n = (int)dataset.size();
#pragma omp parallel for schedule(static)
		for (int i = 0; i < n; i++) {
			auto& sd = dataset[i];
			sd.monthlyAvg = calculateMonthlyAverages(sd);
		}
	}
	else {
		for (auto& sd : dataset) {
			sd.monthlyAvg = calculateMonthlyAverages(sd);
		}
	}
}

std::vector<Anomaly> detectAnomalies(const StationData& sd, const std::unordered_map<int, float>& monthlyAvg) {

	// Find Min and Max avarage for each month
	std::unordered_map<int, std::pair<float, float>> bounds; // key = month

	for (const auto& [key, avg] : monthlyAvg) {
		int month = key % 100;
		if (bounds.find(month) == bounds.end()) bounds[month] = { FLT_MAX, -FLT_MAX }; // initialize
		bounds[month].first = std::min(bounds[month].first, avg);
		bounds[month].second = std::max(bounds[month].second, avg);
	}

	// Calculate 75% threshold for each month
	std::unordered_map<int, float> threshold; // key = month
	for (const auto& [month, bound] : bounds) {
		threshold[month] = (bound.second - bound.first) * 0.75f;
	}


	// Detect anomalies
	std::vector<Anomaly> anomalies;
	for (int month = 1; month <= 12; month++) {
		std::unordered_map<int, float> monthlyValues; // key = year
		for (const auto& [key, avg] : monthlyAvg) {
			if (key % 100 == month) {
				int year = key / 100;
				monthlyValues[year] = avg;
			}
		}
		std::vector<int> years;
		for (const auto& [year, _] : monthlyValues)
			years.push_back(year);
		std::sort(years.begin(), years.end());

		for (size_t i = 1; i < years.size(); i++) {
			if (years[i] == years[i - 1] + 1) { // consecutive years
				float diff = std::abs(monthlyValues[years[i]] - monthlyValues[years[i - 1]]);
				if (diff > threshold[month]) {
					anomalies.push_back({ sd.info.id, month, years[i], diff });
				}
			}
		}
	}
	return anomalies;
}

std::vector<Anomaly> detectAnomalies(const std::vector<StationData>& dataset, RunMode mode) {
	std::vector<Anomaly> allAnomalies;
	if (mode == RunMode::Parallel) {
		int n = (int)dataset.size();
		std::vector<std::vector<Anomaly>> threadResults(omp_get_max_threads());
#pragma omp parallel for schedule(static)
		for (int i = 0; i < n; i++) {
			auto& sd = dataset[i];
			auto anomalies = detectAnomalies(sd, sd.monthlyAvg);
			auto& local = threadResults[omp_get_thread_num()];
			local.insert(local.end(), anomalies.begin(), anomalies.end());
		}
		for (const auto& vec : threadResults) {
			allAnomalies.insert(allAnomalies.end(), vec.begin(), vec.end());
		}
	}
	else {
		
		for (auto& sd : dataset) {
			auto anomalies = detectAnomalies(sd, sd.monthlyAvg);
			allAnomalies.insert(allAnomalies.end(), anomalies.begin(), anomalies.end());
		}
	}
	return allAnomalies;
}

