#include "simulator.h"

int Simulator::simDirectCache(int size)
{
	int numCorrect = 0;
	DirectCache cache(size);

	for (ull i = 0; i < traces.size(); i++)
	{
		bool found = cache.lookup(traces[i].addr, traces[i].instr);
		if (found) numCorrect++;
	}

	return numCorrect;
}

int Simulator::simSetCache(int size, int ways, SET_POLICY policy, bool allocOnWriteMiss)
{
	int numCorrect = 0;
	SetCache cache(size, ways, policy, allocOnWriteMiss);

	for (ull i = 0; i < traces.size(); i++)
	{
		bool found = cache.lookup(traces[i].addr, traces[i].instr);
		if (found) numCorrect++;
	}

	return numCorrect;
}

int Simulator::simFullCache(int size, FULL_POLICY policy)
{
	int numCorrect = 0;
	FullCache cache(size, policy);

	for (ull i = 0; i < traces.size(); i++)
	{
		bool found = cache.lookup(traces[i].addr, traces[i].instr);
		if (found) numCorrect++;
	}

	return numCorrect;
}
