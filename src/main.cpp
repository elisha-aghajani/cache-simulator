#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cache.h"
#include "simulator.h"

static void showUsage(std::string name)
{
	std::cerr << "Usage: " << name
	<< " <input file> <output file>"
	<< std::endl;
}

static bool isFile(std::string name)
{
	std::ifstream f(name.c_str());
	bool success = f.good();
	f.close();

	return success;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		showUsage(argv[0]);
		return 1;
	}

	std::string inputFileName = argv[1];
	std::string outputFileName = argv[2];

	if (isFile(inputFileName) != true)
	{
		std::cout << "Input file " <<  inputFileName << " does not exist" << std::endl;
		return 1;
	}

	Simulator simulator;

	std::ifstream inputFile(inputFileName);
	std::string line;
	int numTraces = 0;
	while (std::getline(inputFile, line))
	{
		std::stringstream ss(line);
		Trace trace;
		ss >> trace.instr >> std::hex >> trace.addr;
		simulator.traces.push_back(trace);
		numTraces++;
	}
	inputFile.close();

	std::ofstream outputFile(outputFileName);

	std::vector<int> cacheSizes{1024, 4096, 16384, 32768};
	std::vector<int> waySizes{2, 4, 8, 16};
	int numCorrect;

	// simulate direct mapped cache
	for (ul i = 0; i < cacheSizes.size(); i++)
	{
		numCorrect = simulator.simDirectCache(cacheSizes[i]);
		outputFile << numCorrect << "," << numTraces << ";";
		if (i + 1 != cacheSizes.size()) outputFile << " ";
	}
	outputFile << std::endl;

	// simulate set associative cache with no prefetch and allocation on write miss
	for (ul i = 0; i < waySizes.size(); i++)
	{
		numCorrect = simulator.simSetCache(16384, waySizes[i], NO_PREFETCH_POLICY, true);
		outputFile << numCorrect << "," << numTraces << ";";
		if (i + 1 != waySizes.size()) outputFile << " ";
	}
	outputFile << std::endl;

	// simulate fully associative cache with LRU policy
	numCorrect = simulator.simFullCache(16384, LRU_POLICY);
	outputFile << numCorrect << "," << numTraces << ";";
	outputFile << std::endl;

	// simulate fully associative cache with Hot Cold policy
	numCorrect = simulator.simFullCache(16384, HC_POLICY);
	outputFile << numCorrect << "," << numTraces << ";";
	outputFile << std::endl;

	// simulate set associative cache with no prefetch and no allocation on write miss
	for (ul i = 0; i < waySizes.size(); i++)
	{
		numCorrect = simulator.simSetCache(16384, waySizes[i], NO_PREFETCH_POLICY, false);
		outputFile << numCorrect << "," << numTraces << ";";
		if (i + 1 != waySizes.size()) outputFile << " ";
	}
	outputFile << std::endl;

	// simulate set associative cache with next line prefetch and allocation on write miss
	for (ul i = 0; i < waySizes.size(); i++)
	{
		numCorrect = simulator.simSetCache(16384, waySizes[i], ALL_PREFETCH_POLICY, true);
		outputFile << numCorrect << "," << numTraces << ";";
		if (i + 1 != waySizes.size()) outputFile << " ";
	}
	outputFile << std::endl;

	// simulate set associative cache with prefetch on miss only and allocation on write miss
	for (ul i = 0; i < waySizes.size(); i++)
	{
		numCorrect = simulator.simSetCache(16384, waySizes[i], MISS_PREFETCH_POLICY, true);
		outputFile << numCorrect << "," << numTraces << ";";
		if (i + 1 != waySizes.size()) outputFile << " ";
	}
	outputFile << std::endl;

	return 0;
}
