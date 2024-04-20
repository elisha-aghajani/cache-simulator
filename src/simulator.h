#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "cache.h"

struct Trace
{
	std::string instr;
	ull addr;
};

class Simulator
{
	public:
		int simDirectCache(int size);
		int simSetCache(int size, int ways, SET_POLICY policy, bool allocOnWriteMiss);
		int simFullCache(int size, FULL_POLICY policy);
		std::vector<Trace> traces;
};

#endif // SIMULATOR_H
