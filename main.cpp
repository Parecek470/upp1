#include <iostream>
#include <chrono>
#include <vector>
#include <omp.h>
#include "ArgParser.h"
#include "DataLoader.h"
#include "Filter.h"
#include "Analyzer.h"
#include "OutputWriter.h"

static long long measure(auto&& fn) {
    auto start = std::chrono::high_resolution_clock::now();
    fn();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char* argv[])
{
    AppConfig config = parseArgs(argc, argv);

    std::vector<Station> stations;
    std::cout << "Loading stations took: "
        << measure([&] { stations = loadStations(config.stationsFileName); })
        << " ms\n";
    if (stations.empty()) { std::cout << "no stations\n"; return 1; }

    
    std::vector<Measurement> measurements;
    std::cout << "Loading measurements took: "
        << measure([&] { measurements = loadMeasurements(config.measurementsFileName, config.mode); })
        << " ms\n";
    if (measurements.empty()) { std::cout << "no measurements\n"; return 1; }


    std::vector<StationData> dataset;
    std::cout << "DataSet building took: "
        << measure([&] { dataset = buildDataset(stations, measurements, config.mode); })
        << " ms\n";


    std::cout << "Filtering took: "
        << measure([&] { dataset = filterStations(dataset, config.mode); })
        << " ms\n";


    std::cout << "Calculating averages took: "
        << measure([&] { calculateMonthlyAverages(dataset, config.mode); })
        << " ms\n";


    std::vector<Anomaly> allAnomalies;
    std::cout << "Anomaly identification took: "
        << measure([&] { allAnomalies = detectAnomalies(dataset, config.mode); })
        << " ms\n";


    std::cout << "Anomaly writing took: "
        << measure([&] { writeAnomalies(allAnomalies, "vykyvy.csv"); })
        << " ms\n";


    std::pair<float,float> globalMinMax;
    std::cout << "Global min and max calculation took: "
        << measure([&] { globalMinMax = computeGlobalMinMax(dataset, config.mode); })
        << " ms\n";


    std::cout << "Generation of SVGs took: "
        << measure([&] { generateSVGs(dataset, globalMinMax, config.mode); })
        << " ms\n";

    return 0;
}