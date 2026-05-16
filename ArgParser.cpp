#include <iostream>

#include "ArgParser.h"

void printHelp() {
	std::cout << "Usage: ./upp_sp1 <stations.csv> <measurements.csv> <options>" << std::endl;
	std::cout << "	options: --parallel" << std::endl;
	std::cout << "			 --serial" << std::endl;


}

AppConfig parseArgs(int argc, char* argv[]) {
	RunMode mode;
	if (argc != 4) {
		printHelp();
		exit(1);
	}

	std::string flag = argv[3];
	if (flag == "--parallel") {
		mode = RunMode::Parallel;
	}
	else if (flag == "--serial") {
		mode = RunMode::Serial;
	}
	else {
		std::cerr << "unknown option" << std::endl;
		exit(1);
	}

	return AppConfig{ argv[1], argv[2],"czmap.svg", mode };
}

