#include <iostream>
#include <chrono>
#include "ArgParser.h"
#include "DataLoader.h"
#include "Filter.h"
#include "Analyzer.h"
#include "OutputWriter.h"

static const std::string monthNames[] = {
	"", "leden", "unor", "brezen", "duben", "kveten", "cerven",
	"cervenec", "srpen", "zari", "rijen", "listopad", "prosinec"
};

int main(int argc, char* argv[])
{
	AppConfig config = parseArgs(argc, argv);
	

	auto start = std::chrono::high_resolution_clock::now();
	auto stations = loadStations(config.stationsFileName);
	auto end = std::chrono::high_resolution_clock::now();
	if (stations.empty()) {
		std::cout << "no stations";
		exit(1);
	}
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Loading stations took: " << ms << " ms\n";


	start = std::chrono::high_resolution_clock::now();
	auto measurements = loadMeasurements(config.measurementsFileName);
	end = std::chrono::high_resolution_clock::now();
	
	if (stations.empty()) {
		std::cout << "no measurements";
		exit(1);
	}
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Loading measurements took: " << ms << " ms\n";



	start = std::chrono::high_resolution_clock::now();
	auto dataset = buildDataset(stations, measurements);
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "DataSet building took: " << ms << " ms\n";


	start = std::chrono::high_resolution_clock::now();
	dataset = filterStations(dataset);
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Filtering took: " << ms << " ms\n";



	start = std::chrono::high_resolution_clock::now();
	std::vector<Anomaly> allAnomalies;
	for (auto& sd : dataset) {
		sd.monthlyAvg = calculateMonthlyAverages(sd);
		std::ranges::copy(detectAnomalies(sd, sd.monthlyAvg),std::back_inserter(allAnomalies));
	}
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Anomaly identification took: " << ms << " ms\n";

	start = std::chrono::high_resolution_clock::now();
	writeAnomalies(allAnomalies, "vykyvy.csv");
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Anomaly writing took: " << ms << " ms\n";

	start = std::chrono::high_resolution_clock::now();
	auto globalMinMax = computeGlobalMinMax(dataset);
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Global min and max calculation took: " << ms << " ms\n";

	start = std::chrono::high_resolution_clock::now();
	for (int month = 1; month <= 12; month++) {
		generateSVG(dataset,month, globalMinMax, monthNames[month] + ".svg");
	}
	end = std::chrono::high_resolution_clock::now();
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "generation of svg took: " << ms << " ms\n";
}