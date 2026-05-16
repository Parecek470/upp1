#include <iostream>
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
	
	auto stations = loadStations(config.stationsFileName);
	auto measurements = loadMeasurements(config.measurementsFileName);
	auto dataset = buildDataset(stations, measurements);
	dataset = filterStations(dataset);

	std::vector<Anomaly> allAnomalies;
	for (auto& sd : dataset) {
		sd.monthlyAvg = calculateMonthlyAverages(sd);
		std::ranges::copy(detectAnomalies(sd, sd.monthlyAvg),std::back_inserter(allAnomalies));
	}

	writeAnomalies(allAnomalies, "vykyvy.csv");

	auto globalMinMax = computeGlobalMinMax(dataset);

	for (int month = 1; month <= 12; month++) {
		generateSVG(dataset,month, globalMinMax, monthNames[month] + ".svg");
	}

}