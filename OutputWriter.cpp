#include <fstream>
#include <iostream>
#include <format>
#include <array>
#include "OutputWriter.h"

static const std::string mapPath = "czmap.svg";
static const double latMin = 48.521003814763994;
static const double latMax = 51.03806105663445;
static const double lonMin = 12.102209054269062;
static const double lonMax = 18.866923511078615;
static const int svgWidth = 5338;
static const int svgHeight = 3056;

static std::string toString(const Anomaly& anomaly) {
	return std::format("{};{};{};{:.3f}", anomaly.stationId, anomaly.month, anomaly.year, anomaly.difference);
}

static std::string svgHeaderOpenElement(float min, float max) {
	return std::format(
		"<svg width=\"{}\" height=\"{}\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		" <image href=\"{}\" width=\"{}\" height=\"{}\" />\n"
		" <defs>\n"
		"  <linearGradient id=\"legendGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">\n"
		"   <stop offset=\"0%\" stop-color=\"rgb(0,0,255)\"/>\n"
		"   <stop offset=\"50%\" stop-color=\"rgb(255,255,0)\"/>\n"
		"   <stop offset=\"100%\" stop-color=\"rgb(255,0,0)\"/>\n"
		"  </linearGradient>\n"
		" </defs>\n"
		" <rect x=\"50\" y=\"50\" width=\"300\" height=\"30\" fill=\"url(#legendGrad)\"/>\n"
		" <text x=\"50\" y=\"100\">{:.1f}</text>\n"
		" <text x=\"335\" y=\"100\">{:.1f}</text>",
		svgWidth, svgHeight, mapPath, svgWidth, svgHeight, min, max
	);
}

static std::string stationToSvgElement(float x, float y, std::array<int, 3> colors) {
	return std::format("<circle cx =\"{}\" cy=\"{}\" r=\"20\" fill=\"rgb({}, {}, {})\"/>",x,y,colors[0], colors[1], colors[2]);
}

static std::array<int,3> rgbCalculation(float temp, float min, float max) {
	float t = (temp - min) / (max - min);
	int r, g, b;
	if (t <= 0.5f) {
		float s = t / 0.5f;  
		r = (int)std::lerp(0.0f, 255.0f, s);
		g = (int)std::lerp(0.0f, 255.0f, s);
		b = (int)std::lerp(255.0f, 0.0f, s);
	}
	else {
		float s = (t - 0.5f) / 0.5f;  
		r = 255;
		g = (int)std::lerp(255.0f, 0.0f, s);
		b = 0;
	}
	return { r, g, b };
}

void writeAnomalies(const std::vector<Anomaly>& anomalies, const std::string& fileName) {
	std::ofstream file(fileName);
	if (!file.is_open()) {
		std::cerr << "cannot write anomalies into file" << std::endl;
		exit(1);
	}

	file << "stationId;month;year;difference" << std::endl;
	for (const auto& a : anomalies) {
		file << toString(a) << '\n';
	}
}

void generateSVG(const std::vector<StationData>& data, int month, std::pair<float, float> minMax, const std::string& outputPath) {
	std::ofstream file(outputPath);
	if (!file.is_open()) {
		std::cerr << "cannot write map" << std::endl;
		exit(1);
	}

	file << svgHeaderOpenElement(minMax.first, minMax.second);
	for (const auto& sd : data) {
		float normX = (float)((sd.info.longitude - lonMin) / (lonMax - lonMin));
		float normY = (float)((latMax - sd.info.latitude) / (latMax - latMin));
		float x = std::lerp(0.0f, (float)svgWidth, normX);
		float y = std::lerp(0.0f, (float)svgHeight, normY);

		//avaratge across all years
		float sum = 0; int count = 0;
		for (const auto& [key, avg] : sd.monthlyAvg) {
			if (key % 100 == month) { sum += avg; count++; }
		}
		float monthTemp = count > 0 ? sum / count : 0.0f;

		file << stationToSvgElement(x, y, rgbCalculation(monthTemp, minMax.first, minMax.second)) << '\n';

	}
	file << "</svg>";
}

