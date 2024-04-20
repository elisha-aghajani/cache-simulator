#include "cache.h"

// number of bytes that can be stored in each cache block
int DEFAULT_BLOCK_SIZE = 32;

ull Cache::extractBits(ull addr, int numBits, int from)
{
	return (((1ull << numBits) - 1ull) & (addr >> from));
}

ull Cache::getTag(ull addr)
{
	return addr >> (offsetBits + indexBits);
}

DirectCache::DirectCache(int size)
{
	blockSize = DEFAULT_BLOCK_SIZE;
	numBlocks = size / blockSize;

	blocks.resize(numBlocks);
	
	offsetBits = (int) std::log2((float) blockSize);
	indexBits = (int) std::log2((float) numBlocks);
	tagBits = blockSize - indexBits - offsetBits;
}

bool DirectCache::lookup(ull addr, std::string instr)
{
	ull cacheIndex = extractBits(addr, indexBits, offsetBits);
	ull addrTag = getTag(addr);

	bool found = false;
	if (blocks[cacheIndex].valid && blocks[cacheIndex].tag == addrTag)
	{
		found = true;
	}

	if (!found)
	{
		blocks[cacheIndex].tag = addrTag;
		blocks[cacheIndex].valid = true;
	}
	
	return found;
}

SetCache::SetCache(int size, int ways, SET_POLICY policy, bool allocOnWriteMiss)
{
	blockSize = DEFAULT_BLOCK_SIZE;
	numWays = ways;
	numSets = size / (numWays * blockSize);

	sets.resize(numSets);
	LRUMatrix.resize(numSets);

	for (int i = 0; i < numSets; i++)
	{
		sets[i].blocks.resize(numWays);
		LRUMatrix[i].resize(numWays);
	}

	this->policy = policy;
	this->allocOnWriteMiss = allocOnWriteMiss;

	offsetBits = (int) std::log2((float) blockSize);
	indexBits = (int) std::log2((float) numSets);
	tagBits = blockSize - indexBits - offsetBits;
}

bool SetCache::lookup(ull addr, std::string instr)
{
	ull cacheIndex = extractBits(addr, indexBits, offsetBits);
	ull addrTag = getTag(addr);

	bool found = false;
	for (int i = 0; i < numWays; i++)
	{
		if (sets[cacheIndex].blocks[i].valid && sets[cacheIndex].blocks[i].tag == addrTag)
		{
			update(cacheIndex, i);
			found = true;
			break;
		}
	}

	if (!found)
	{
		if (allocOnWriteMiss == false && instr == "S")
		{
			return found;
		}

		bool emptySpot = false;
		for (int i = 0; i < numWays; i++)
		{
			if (!sets[cacheIndex].blocks[i].valid)
			{
				sets[cacheIndex].blocks[i].tag = addrTag;
				sets[cacheIndex].blocks[i].valid = true;
				update(cacheIndex, i);
				emptySpot = true;
				break;
			}
		}

		if (!emptySpot)
		{
			int newIndex = getNewIndex(cacheIndex);
			sets[cacheIndex].blocks[newIndex].tag = addrTag;
			sets[cacheIndex].blocks[newIndex].valid = true;
			update(cacheIndex, newIndex);
		}
	}

	if (policy == ALL_PREFETCH_POLICY || (policy == MISS_PREFETCH_POLICY && !found))
	{
		prefetch(addr);
	}

	return found;
}

void SetCache::update(int cacheIndex, int block)
{
	for (int i = 0; i < numWays; i++)
	{
		if (i == block)
		{
			LRUMatrix[cacheIndex][i] = 0;
		}
		else
		{
			if (sets[cacheIndex].blocks[block].valid)
			{
				LRUMatrix[cacheIndex][i]++;
			}
		}
	}
}

int SetCache::getNewIndex(int cacheIndex)
{
	int max = -1;
	int LRUIndex;
	for (int i = 0; i < numWays; i++)
	{
		if (sets[cacheIndex].blocks[i].valid && LRUMatrix[cacheIndex][i] > max)
		{
			LRUIndex = i;
			max = LRUMatrix[cacheIndex][i];
		}
	}
	return LRUIndex;
}

void SetCache::prefetch(ull addr)
{
	ull nextAddr = addr + DEFAULT_BLOCK_SIZE;

	ull cacheIndex = extractBits(nextAddr, indexBits, offsetBits);
	ull addrTag = getTag(nextAddr);

	bool found = false;
	for (int i = 0; i < numWays; i++)
	{
		if (sets[cacheIndex].blocks[i].valid && sets[cacheIndex].blocks[i].tag == addrTag)
		{
			update(cacheIndex, i);
			found = true;
			break;
		}
	}

	if (!found)
	{
		bool emptySpot = false;
		for (int i = 0; i < numWays; i++)
		{
			if (!sets[cacheIndex].blocks[i].valid)
			{
				sets[cacheIndex].blocks[i].tag = addrTag;
				sets[cacheIndex].blocks[i].valid = true;
				update(cacheIndex, i);
				emptySpot = true;
				break;
			}
		}

		if (!emptySpot)
		{
			int newIndex = getNewIndex(cacheIndex);
			sets[cacheIndex].blocks[newIndex].tag = addrTag;
			sets[cacheIndex].blocks[newIndex].valid = true;
			update(cacheIndex, newIndex);
		}
	}
}

FullCache::FullCache(int size, FULL_POLICY policy)
{
	blockSize = DEFAULT_BLOCK_SIZE;
	numWays = size / blockSize;
	numSets = 1;

	sets.resize(numSets);
	LRUMatrix.resize(numSets);
	HCMatrix.resize(numSets);
	sets[0].blocks.resize(numWays);
	LRUMatrix[0].resize(numWays);
	HCMatrix[0].resize(numWays - 1);

	// initial state of all hot cold bits is 0
	// 0 represents the case where the left child is hot and the right cchild is cold
	// 1 represents the case where the right child is hot and left child is cold
	for (int i = 0; i < numWays - 1; i++)
	{
		HCMatrix[0][i] = 0;
	}

	this->policy = policy;

	offsetBits = (int) std::log2((float) blockSize);
	indexBits = (int) std::log2((float) numSets);
	tagBits = blockSize - indexBits - offsetBits;
}

bool FullCache::lookup(ull addr, std::string instr)
{
	ull addrTag = getTag(addr);
	
	bool found = false;
	for (int i = 0; i < numWays; i++)
	{
		if (sets[0].blocks[i].valid && sets[0].blocks[i].tag == addrTag)
		{
			update(i);
			found = true;
			break;
		}
	}

	if (!found)
	{
		if (policy == LRU_POLICY)
		{
			bool emptySpot = false;
			for (int i = 0; i < numWays; i++)
			{
				if (!sets[0].blocks[i].valid)
				{
					sets[0].blocks[i].tag = addrTag;
					sets[0].blocks[i].valid = true;
					update(i);
					emptySpot = true;
					break;
				}
			}

			if (!emptySpot)
			{
				int newIndex = getNewIndex();
				sets[0].blocks[newIndex].tag = addrTag;
				sets[0].blocks[newIndex].valid = true;
				update(newIndex);
			}
		}
		else if (policy == HC_POLICY)
		{
			int newIndex = getNewIndex();
			sets[0].blocks[newIndex].tag = addrTag;
			sets[0].blocks[newIndex].valid = true;
			return found;
		}
	}

	return found;
}

void FullCache::update(int block)
{
	if (policy == LRU_POLICY)
	{
		for (int i = 0; i < numWays; i++)
		{
			if (i == block)
			{
				LRUMatrix[0][i] = 0;
			}
			else
			{
				if (sets[0].blocks[block].valid)
				{
					LRUMatrix[0][i]++;
				}
			}
		}
	}
	else if (policy == HC_POLICY)
	{
		int pos = numWays + block - 1;
		// traverse the tree from the bottom and update along the way
		while (pos != 0)
		{
			// left child
			if (pos % 2 != 0)
			{
				pos = (pos - 1) / 2; // get parent
				HCMatrix[0][pos] = 0; // update parent to indicate left child is hot 
			}
			// right child
			else if (pos % 2 == 0)
			{
				pos = (pos - 2) / 2; // get parent
				HCMatrix[0][pos] = 1; // update parent to indicate right child is hot
			}
		}
	}
}

int FullCache::getNewIndex()
{
	if (policy == LRU_POLICY)
	{
		int max = -1;
		int LRUIndex;
		for (int i = 0; i < numWays; i++)
		{
			if (sets[0].blocks[i].valid && LRUMatrix[0][i] > max)
			{
				LRUIndex = i;
				max = LRUMatrix[0][i];
			}
		}
		return LRUIndex;
	}
	else if (policy == HC_POLICY)
	{
		int pos = 0;
		int height = (int) std::log2((float) numWays);
		for (int i = 0; i < height; i++)
		{
			// right child is hot -> LRU should come from left half
			if (HCMatrix[0][pos] == 1)
			{
				HCMatrix[0][pos] = 0; // update parent to 0 indicating left child is now hot
				pos = pos * 2 + 1; // get left child
			}
			// left child is hot -> LRU should come from right half
			else if (HCMatrix[0][pos] == 0)
			{
				HCMatrix[0][pos] = 1; // update parent to 1 indicating right child is now hot
				pos = pos * 2 + 2; // get right child
			}
		}
		// pos gets updated one extra time so we need to fix
		return pos - (numWays - 1);
	}
}
