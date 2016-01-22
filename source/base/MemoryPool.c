/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MemoryPool.h>
#include <Game.h>
#include <Utilities.h>
#include <Types.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

// it is necessary for the object to be aligned to 2's multiples
#define __MEMORY_ALIGNMENT			0
#define __MEMORY_USED_BLOCK_FLAG	0xFFFFFFFF
#define __MEMORY_FREE_BLOCK_FLAG	0x00000000

#define __MEMORY_POOLS							12

#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(188, 1)																			\
	__BLOCK_DEFINITION(156, 4)																			\
	__BLOCK_DEFINITION(140, 26)																			\
	__BLOCK_DEFINITION(128, 6)																			\
	__BLOCK_DEFINITION(116, 36)																			\
	__BLOCK_DEFINITION(100, 46)																			\
	__BLOCK_DEFINITION(84, 32)																			\
	__BLOCK_DEFINITION(76, 6)																			\
	__BLOCK_DEFINITION(68, 72)																			\
	__BLOCK_DEFINITION(28, 270)																			\
	__BLOCK_DEFINITION(20, 600)																			\
	__BLOCK_DEFINITION(16, 240)																		\

#define __SET_MEMORY_POOL_ARRAYS																		\
	__SET_MEMORY_POOL_ARRAY(188)																		\
	__SET_MEMORY_POOL_ARRAY(156)																		\
	__SET_MEMORY_POOL_ARRAY(140)																		\
	__SET_MEMORY_POOL_ARRAY(128)																		\
	__SET_MEMORY_POOL_ARRAY(116)																		\
	__SET_MEMORY_POOL_ARRAY(100)																			\
	__SET_MEMORY_POOL_ARRAY(84)																			\
	__SET_MEMORY_POOL_ARRAY(76)																			\
	__SET_MEMORY_POOL_ARRAY(68)																			\
	__SET_MEMORY_POOL_ARRAY(28)																			\
	__SET_MEMORY_POOL_ARRAY(20)																			\
	__SET_MEMORY_POOL_ARRAY(16)	
//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MemoryPool_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* dynamic memory area */																			\
	/* must always put together the pools! */															\
	/* first byte is used as a usage flag */															\
	__MEMORY_POOL_ARRAYS																				\
																										\
	/* pointer to the beginning of each memory pool */													\
	BYTE* poolLocation[__MEMORY_POOLS];																	\
																										\
	/* pool's size and pool's block size */																\
	int poolSizes[__MEMORY_POOLS][2];																	\

__CLASS_DEFINITION(MemoryPool, Object);

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MemoryPool_constructor(MemoryPool this);
static void MemoryPool_reset(MemoryPool this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(MemoryPool);

// class constructor
static void MemoryPool_constructor(MemoryPool this)
{
	__CONSTRUCT_BASE();

	MemoryPool_reset(this);
	MemoryPool_cleanUp(this);
}

// class's destructor
void MemoryPool_destructor(MemoryPool this)
{
	ASSERT(this, "MemoryPool::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// allocate memory for data
BYTE* MemoryPool_allocate(MemoryPool this, int numBytes)
{
	ASSERT(this, "MemoryPool::allocate: null this");

	int i = 0;
	int numberOfOjects = 0;
	int pool = __MEMORY_POOLS - 1;
	int blockSize = this->poolSizes[pool][eBlockSize];
	int displacement = 0;
	int displacementStep = 0;

	// the first 4 bytes are for memory block usage flag
	numBytes += __MEMORY_ALIGNMENT;

	// seach for the shortest pool which can hold the data
	for(; numBytes > blockSize && pool--; blockSize = this->poolSizes[pool][eBlockSize]);

	if(0 > pool)
	{
		Printing_clear(Printing_getInstance());
		Printing_text(Printing_getInstance(), "Block's size requested: ", 20, 12, NULL);
		Printing_int(Printing_getInstance(), numBytes, 44, 12, NULL);

		NM_ASSERT(pool >= 0, "MemoryPool::allocate: object size overflow");
	}

	// get the number of allocable objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / blockSize;

	// how much must displace on each iteration
	displacementStep = blockSize;

	// look for a free block
	for(i = 0, displacement = 0;
	    i < numberOfOjects && __MEMORY_FREE_BLOCK_FLAG != *((u32*)&this->poolLocation[pool][displacement]);
	    i++, displacement += displacementStep);

	if(i >= numberOfOjects)
	{
		Printing_clear(Printing_getInstance());
		MemoryPool_printDetailedUsage(this, 1, 8);
		Printing_text(Printing_getInstance(), "Block's size requested: ", 20, 12, NULL);
		Printing_int(Printing_getInstance(), numBytes, 44, 12, NULL);

		NM_ASSERT(false, "MemoryPool::allocate: pool exhausted");
	}

	// mark address as allocated
	*((u32*)&this->poolLocation[pool][displacement]) = __MEMORY_USED_BLOCK_FLAG;

	// return designed address
	return &this->poolLocation[pool][displacement + __MEMORY_ALIGNMENT];
};

// free memory when an object is no longer used
// remove an object from heap
void MemoryPool_free(MemoryPool this, BYTE* object)
{
	ASSERT(this, "MemoryPool::free: null this");

	if(!(object >= &this->poolLocation[0][0] && object < &this->poolLocation[__MEMORY_POOLS - 1][0] + this->poolSizes[__MEMORY_POOLS - 1][ePoolSize]))
	{
		return;
	}
	
#ifdef __DEBUG

	int i;
	int pool = 0;
	int displacement = 0;
	int numberOfOjects = 0;

	if(!object)
	{
		return;
	}

	// look for the pool containing the object
	for(pool = 0; object >= &this->poolLocation[pool][0 + __MEMORY_ALIGNMENT] && pool < __MEMORY_POOLS; pool++);

	// look for the registry in which the object is
	ASSERT(pool <= __MEMORY_POOLS , "MemoryPool::free: deleting something not allocated");

	// move one pool back since the above loop passed the target by one
	pool--;

	// get the total objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	// search for the pool in which it is allocated
	for(i = 0, displacement = 0; i < numberOfOjects; i++, displacement += this->poolSizes[pool][eBlockSize])
	{
		// if the object has been found
		if(object == &this->poolLocation[pool][displacement + __MEMORY_ALIGNMENT])
		{
			// free the block
			*((u32*)&this->poolLocation[pool][displacement]) = __MEMORY_FREE_BLOCK_FLAG;

			return;
		}
	}

	// thrown exception
	ASSERT(false, "MemoryPool::free: deleting something not allocated");

#endif

	// set address as free
	*((u32*)&object[-__MEMORY_ALIGNMENT]) = __MEMORY_FREE_BLOCK_FLAG;
}

// clear all dynamic memory
static void MemoryPool_reset(MemoryPool this)
{
	ASSERT(this, "MemoryPool::reset: null this");

	int pool = 0;
	int i;

	// initialize pool's sizes and pointers
	__SET_MEMORY_POOL_ARRAYS
	
	// clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for(i = 0; i < this->poolSizes[pool][ePoolSize]; i++)
		{
			*((u32*)&this->poolLocation[pool][i]) = __MEMORY_FREE_BLOCK_FLAG;
		}
	}
}

// clear all dynamic memory
void MemoryPool_cleanUp(MemoryPool this)
{
	ASSERT(this, "MemoryPool::reset: null this");

	int pool = 0;

	// clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		int i = 0;
		for (; i < this->poolSizes[pool][ePoolSize]; i += this->poolSizes[pool][eBlockSize])
		{
			if(!*((u32*)&this->poolLocation[pool][i]))
			{
				int j = i;
				for(; j < this->poolSizes[pool][eBlockSize]; j++)
				{
					this->poolLocation[pool][j] = 0;
				}
			}
		}
	}
}

// retrieve pool size
int MemoryPool_getPoolSize(MemoryPool this)
{
	ASSERT(this, "MemoryPool::reset: null this");

	int size = 0;
	int pool = 0;

	// clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		size += this->poolSizes[pool][ePoolSize];
	}

	return size;
}

// print dynamic memory usage
void MemoryPool_printDetailedUsage(MemoryPool this, int x, int y)
{
	ASSERT(this, "MemoryPool::printMemUsage: null this");

	int i;
	int totalUsedBlocks = 0;
	int totalUsedBytes = 0;
	int pool;
	int displacement = 0;

	Printing_text(Printing_getInstance(), "MEMORY STATUS", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Pool", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "Free", x + 5, y, NULL);
	Printing_text(Printing_getInstance(), "Used", x + 10, y++, NULL);

	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		int totalBlocks = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if(this->poolLocation[pool][displacement])
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolSizes[pool][eBlockSize];

		Printing_int(Printing_getInstance(), this->poolSizes[pool][eBlockSize],  x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
		Printing_text(Printing_getInstance(), "      ", x + 14, y, NULL);
		Printing_int(Printing_getInstance(), totalUsedBlocks, x + 10, y, NULL);
		
		int usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		Printing_int(Printing_getInstance(), usedBlocksPercentage, x + 17 - Utilities_intLength(usedBlocksPercentage), y, NULL);
		Printing_text(Printing_getInstance(), "% ", x + 17, y, NULL);

		totalUsedBlocks = 0 ;
	}
	
	++y;
	int poolSize = MemoryPool_getPoolSize(this);
	Printing_text(Printing_getInstance(), "Pool size: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), poolSize, x + 18 - Utilities_intLength(poolSize), y, NULL);

	Printing_text(Printing_getInstance(), "Pool usage: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), totalUsedBytes, x + 18 - Utilities_intLength(totalUsedBytes), y++, NULL);
	
	int usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing_int(Printing_getInstance(), usedBytesPercentage, x + 17 - Utilities_intLength(usedBytesPercentage), y, NULL);
	Printing_text(Printing_getInstance(), "% ", x + 17, y++, NULL);
}

// print dynamic memory usage
void MemoryPool_printResumedUsage(MemoryPool this, int x, int y)
{
	ASSERT(this, "MemoryPool::printMemUsage: null this");

	int originalY = y;
	int i;
	int totalUsedBlocks = 0;
	int totalUsedBytes = 0;
	int pool;
	int displacement = 0;
	
	Printing_text(Printing_getInstance(), "MEM", x, y++, NULL);

	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		int totalBlocks = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if(this->poolLocation[pool][displacement])
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolSizes[pool][eBlockSize];

		int usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		
		Printing_text(Printing_getInstance(), "           ", x, originalY + 1 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printing_int(Printing_getInstance(), this->poolSizes[pool][eBlockSize],  x, y, NULL);
			Printing_int(Printing_getInstance(), usedBlocksPercentage, x + 7 - Utilities_intLength(usedBlocksPercentage), y, NULL);
			Printing_text(Printing_getInstance(), "% ", x + 7, y++, NULL);
		}

		totalUsedBlocks = 0 ;
	}
	
	y = originalY;
	int poolSize = MemoryPool_getPoolSize(MemoryPool_getInstance());
	int usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing_int(Printing_getInstance(), usedBytesPercentage, x + 7 - Utilities_intLength(usedBytesPercentage), y, NULL);
	Printing_text(Printing_getInstance(), "% ", x + 7, y++, NULL);
}