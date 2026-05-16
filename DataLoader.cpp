#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <unordered_map>


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
        std::stringstream ss(line);
        Station s;
        std::string token;

		std::getline(ss, token, ';');
		s.id = std::stoi(token);

		std::getline(ss, s.name, ';');

		std::getline(ss, token, ';');
		s.latitude = std::stod(token);

		std::getline(ss, token, ';');
		s.longitude = std::stod(token);

        stations.push_back(s);
    }
    return stations;
}

std::vector<Measurement> loadMeasurements(const std::string& fileName) {
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
		std::stringstream ss(line);
		Measurement m;
		std::string token;

		std::getline(ss, token, ';');
		m.stationId = std::stoi(token);

		std::getline(ss, token, ';');
		m.ordinal = std::stoi(token);

		std::getline(ss, token, ';');
		m.year = std::stoi(token);

		std::getline(ss, token, ';');
		m.month = std::stoi(token);

		std::getline(ss, token, ';');
		m.day = std::stoi(token);

		std::getline(ss, token, ';');
		m.value = std::stof(token);

		measurements.push_back(m);
	}
	return measurements;
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