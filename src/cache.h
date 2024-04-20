#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

typedef unsigned long long ull;
typedef unsigned long ul;

struct Block
{
	bool valid = false;
	ull tag;
};

struct Set
{
	std::vector<Block> blocks;
};

// set associative prefetch policies
enum SET_POLICY {NO_PREFETCH_POLICY, ALL_PREFETCH_POLICY, MISS_PREFETCH_POLICY};

// fully associative replacement policies
enum FULL_POLICY {LRU_POLICY, HC_POLICY};

class Cache
{
	public:
		virtual bool lookup(ull addr, std::string instr) = 0;
	protected:
		ull extractBits(ull addr, int numBits, int from);
		ull getTag(ull addr);
		int blockSize;
		int offsetBits;
		int indexBits;
		int tagBits;
};

class DirectCache : public Cache
{
	public:
		DirectCache(int size);
		bool lookup(ull addr, std::string instr);
	private:
		std::vector<Block> blocks;
		int numBlocks;
};

class SetCache : public Cache
{
	public:
		SetCache(int size, int ways, SET_POLICY policy, bool allocOnWriteMiss);
		bool lookup(ull addr, std::string instr);
	private:
		void update(int cacheIndex, int way);
		int getNewIndex(int cacheIndex);
		void prefetch(ull addr);
		std::vector<Set> sets;
		std::vector< std::vector<int> > LRUMatrix;
		int numSets;
		int numWays;
		SET_POLICY policy;
		bool allocOnWriteMiss;
};

class FullCache : public Cache
{
	public:
		FullCache(int size, FULL_POLICY policy);
		bool lookup(ull addr, std::string instr);
	private:
		void update(int block);
		int getNewIndex();
		std::vector<Set> sets;
		std::vector< std::vector<int> > LRUMatrix;
		std::vector< std::vector<int> > HCMatrix;
		int numSets;
		int numWays;
		FULL_POLICY policy;
};

#endif // CACHE_H
