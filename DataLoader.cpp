#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <charconv>
#include <omp.h>

std::vector<Station> loadStations(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return {};
    }

	std::vector<Station> stations;
	stations.reserve(10'000);
    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
		if (line.empty()) continue;
		size_t pos = 0, next;
        Station s;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, s.id);
		pos = next + 1;

		next = line.find(';', pos);
		s.name = std::string_view(line).substr(pos, next - pos);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, s.latitude);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, s.longitude);
		stations.push_back(s);
    }
    return stations;
}




static std::vector<Measurement> loadMeasurementsSerial(const std::string& fileName) {
	std::ifstream file(fileName);
	if (!file.is_open()) {
		return {};
	}
    
	std::vector<Measurement> measurements;
	measurements.reserve(5'000'000);
    std::string line;
	std::getline(file, line); // skip header

	while (std::getline(file, line)) {
		if (line.empty()) continue;
		size_t pos = 0, next;
		Measurement m;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, m.stationId);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, m.ordinal);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, m.year);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, m.month);
		pos = next + 1;

		next = line.find(';', pos);
		std::from_chars(line.data() + pos, line.data() + next, m.day);
		pos = next + 1;

		std::from_chars(line.data() + pos, line.data() + line.size(), m.value);

		measurements.push_back(m);
	}
	return measurements;
}

static std::vector<Measurement> loadMeasurementsParallel(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::binary);
	std::string buffer(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>{}
	);

	// 2. Find chunk boundaries (serial)
	int nThreads = omp_get_max_threads();
	std::vector<size_t> starts(nThreads), ends(nThreads);
	size_t headerEnd = buffer.find('\n'); // skip header

	for (int t = 0; t < nThreads; t++) {
		size_t approx = headerEnd + 1 + (buffer.size() - headerEnd) * t / nThreads;
		starts[t] = (t == 0) ? headerEnd + 1 : buffer.find('\n', approx) + 1;
	}

	for (int t = 0; t < nThreads - 1; t++) {
		ends[t] = starts[t + 1]; // end of chunk t = start of chunk t+1
	}
	ends[nThreads - 1] = buffer.size();

	// 3. Parallel parse
	std::vector<std::vector<Measurement>> threadResults(nThreads);
#pragma omp parallel for schedule(static)
	for (int t = 0; t < nThreads; t++) {
		size_t pos = starts[t];
		while (pos < ends[t]) {
			size_t lineEnd = buffer.find('\n', pos);
			if (lineEnd == std::string::npos || lineEnd > ends[t])
				lineEnd = ends[t];

			// Handle possible \r\n line endings because we read in binary mode
			size_t actualEnd = lineEnd;
			if (actualEnd > pos && buffer[actualEnd - 1] == '\r')
				actualEnd--;

			const char* p = buffer.data() + pos;
			const char* end = buffer.data() + actualEnd;

			Measurement m;
			auto next = std::find(p, end, ';');
			std::from_chars(p, next, m.stationId);
			p = next + 1;

			next = std::find(p, end, ';');
			std::from_chars(p, next, m.ordinal);
			p = next + 1;

			next = std::find(p, end, ';');
			std::from_chars(p, next, m.year);
			p = next + 1;

			next = std::find(p, end, ';');
			std::from_chars(p, next, m.month);
			p = next + 1;

			next = std::find(p, end, ';');
			std::from_chars(p, next, m.day);
			p = next + 1;

			std::from_chars(p, end, m.value);

			threadResults[t].push_back(m);
			pos = lineEnd + 1; 
		}
	}

	// 4. Merge results
	size_t total = 0;
	for (const auto& v : threadResults)
		total += v.size();

	std::vector<Measurement> measurements;
	measurements.reserve(total);  

	for (auto& v : threadResults)
		measurements.insert(measurements.end(), v.begin(), v.end());
	return measurements;
}

// Dispatcher function to choose loading method based on mode
std::vector<Measurement> loadMeasurements(const std::string& fileName, RunMode mode) {
	if (mode == RunMode::Parallel) {
		return loadMeasurementsParallel(fileName);
	}
	else {
		return loadMeasurementsSerial(fileName);
	}
}

std::vector<StationData> buildDataset(const std::vector<Station>& stations, const std::vector<Measurement>& measurements) {
	std::unordered_map<int, std::vector<Measurement>> byStation;
	for (const auto& m : measurements) {
		byStation[m.stationId].push_back(m);
	}

	std::vector<StationData> dataset;
	dataset.reserve(stations.size());
	for (const auto& s : stations) {
		StationData sd;
		sd.info = s;
		auto it = byStation.find(s.id);
		if (it != byStation.end()) {
			sd.measurements = std::move(it->second); 
		}
		dataset.push_back(std::move(sd));
	}
	return dataset;
}