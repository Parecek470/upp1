#pragma once
#include <string>
#include <unordered_map>

struct Station
{
	int id;
	std::string name;
	double latitude;
	double longitude;
};

struct Measurement
{
	int stationId;
	int ordinal;
	int year, month, day;
	float value;
};

struct StationData {
	Station info;
	std::vector<Measurement> measurements;
	std::unordered_map<int, float> monthlyAvg; // key: year*100 + month
};
