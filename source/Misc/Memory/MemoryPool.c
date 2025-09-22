/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with memoryPool source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Printer.h>
#include <Singleton.h>
#include <Utilities.h>

#include "MemoryPool.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Have to undefine these in order for the lp to not get corrupted by the checks on the memoryPool pointer
#undef ASSERT
#undef NM_ASSERT
#undef __SAFE_CAST

#define ASSERT(Statement, ...)
#define NM_ASSERT(Statement, ...)
#define __SAFE_CAST(ClassName, object) (ClassName)object

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint8* MemoryPool::allocate(int32 numberOfBytes)
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	NM_ASSERT(__SINGLETON_NOT_CONSTRUCTED != _singletonConstructed, "MemoryPool::allocate: no properly constructed yet");

#ifndef __SHIPPING
#ifndef __RELEASE
	int32 lp = HardwareManager::getLinkPointer();
#endif
#endif

	static uint16 pool = __MEMORY_POOLS >> 1;

	uint32 blockSize = memoryPool->poolSizes[pool][eBlockSize];

	if(blockSize > (uint32)numberOfBytes)
	{
		pool = __MEMORY_POOLS;
	}
	else
	{
		pool++;
	}

	HardwareManager::suspendInterrupts();

	while(0 != pool--)
	{
		blockSize = memoryPool->poolSizes[pool][eBlockSize];

		if((uint32)numberOfBytes > blockSize)
		{
			continue;
		}

		uint8* poolLocationStart = &memoryPool->poolLocation[pool][0];
		uint8* poolLocationLeft = memoryPool->poolLastFreeBlock[pool];
		uint8* poolLocationRight = poolLocationLeft + blockSize;
		uint8* poolLocationEnd = poolLocationStart + memoryPool->poolSizes[pool][ePoolSize] - blockSize;
		uint8* poolLocation = NULL;

		if(100 < memoryPool->poolSizes[pool][ePoolSize] / blockSize)
		{
			CACHE_RESET;
		}

		do
		{
			if(poolLocationRight <= poolLocationEnd)
			{
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationRight))
				{
					poolLocation = poolLocationRight;
					break;
				}
				
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationEnd))
				{
					poolLocation = poolLocationEnd;
					break;
				}

				poolLocationRight += blockSize;
				poolLocationEnd -= blockSize;
			}
			else
			{
				while(poolLocationLeft >= poolLocationStart)
				{
					if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationLeft))
					{
						poolLocation = poolLocationLeft;
						break;
					}
					
					if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationStart))
					{
						poolLocation = poolLocationStart;
						break;
					}

					poolLocationLeft -= blockSize;
					poolLocationStart += blockSize;
				}

				break;
			}

			if(poolLocationLeft >= poolLocationStart)
			{
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationLeft))
				{
					poolLocation = poolLocationLeft;
					break;
				}
				
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationStart))
				{
					poolLocation = poolLocationStart;
					break;
				}

				poolLocationLeft -= blockSize;
				poolLocationStart += blockSize;
			}
			else
			{
				while(poolLocationRight <= poolLocationEnd)
				{
					if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationRight))
					{
						poolLocation = poolLocationRight;
						break;
					}
					
					if(__MEMORY_FREE_BLOCK_FLAG == *((uint16*)poolLocationEnd))
					{
						poolLocation = poolLocationEnd;
						break;
					}

					poolLocationRight += blockSize;
					poolLocationEnd -= blockSize;
				}

				break;
			}
		}
		while(true);
		// Keep looking for a free block on a bigger pool

		if(NULL != poolLocation)
		{
			*((uint16*)poolLocation) = __MEMORY_USED_BLOCK_FLAG;
#ifndef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
			*((uint16*)poolLocation + 1) = (uint16)pool;
#endif
			memoryPool->poolLastFreeBlock[pool] = poolLocation;
			
			HardwareManager::resumeInterrupts();
			return poolLocation;
		}
	}

#ifndef __SHIPPING
	Printer::setDebugMode();
	Printer::clear();
	MemoryPool::printDetailedUsage(1, 8);
	Printer::text("Block's size requested: ", 20, 26, NULL);
	Printer::int32(numberOfBytes, 44, 26, NULL);
#ifndef __RELEASE
	Printer::text("Caller address: ", 20, 27, NULL);
	Printer::hex(lp, 36, 27, 8, NULL);
#endif

	Error::triggerException("MemoryPool::allocate: pool exhausted", NULL);		
#endif

	// Return designed address
	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void MemoryPool::free(uint8* object)
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	NM_ASSERT(__SINGLETON_NOT_CONSTRUCTED != _singletonConstructed, "MemoryPool::free: no properly constructed yet");

#ifndef __RELEASE
	if(NULL == object)
	{
		return;
	}
#endif

	int16 pool = *((uint16*)object + 1);

	if(0 > pool)
	{
		// Calls to delete non dynamic singletons are intented to fall here.
		return;
	}

	// Look for the registry in which the object is
	NM_ASSERT(pool <= __MEMORY_POOLS , "MemoryPool::free: deleting something not allocated");

	memoryPool->poolLastFreeBlock[pool] = object;

#ifdef __DEBUG
	// Get the total objects in the pool
	uint32 numberOfBlocks = memoryPool->poolSizes[pool][ePoolSize] / memoryPool->poolSizes[pool][eBlockSize];

	HardwareManager::suspendInterrupts();

	// Look for the pool in which it is allocated
	for(uint32 i = 0, displacement = 0; i < numberOfBlocks; i++, displacement += memoryPool->poolSizes[pool][eBlockSize])
	{
		// If the object has been found
		if(object == &memoryPool->poolLocation[pool][displacement])
		{
			// Free the block
			*(uint32*)((uint32)object) = __MEMORY_FREE_BLOCK_FLAG;
			
			HardwareManager::resumeInterrupts();
			return;
		}
	}

	// Thrown exception
	ASSERT(false, "MemoryPool::free: deleting something not allocated");

#endif

	// Set address as free
	*(uint32*)((uint32)object) = __MEMORY_FREE_BLOCK_FLAG;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __RELEASE
static void MemoryPool::printResumedUsage(int32 x, int32 y)
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	uint32 originalY = y;
	uint32 i;
	uint32 totalUsedBlocks = 0;
	uint32 totalUsedBytes = 0;
	uint32 pool;
	uint32 displacement = 0;

	Printer::text("MEMORY:", x, y, NULL);
	uint32 poolSize = MemoryPool::getPoolSize(memoryPool);
	Printer::text("Total: ", x, ++y, NULL);
	Printer::int32(poolSize, x + 12 - Math::getDigitsCount(poolSize), y++, NULL);

	for(y += 2, pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 totalBlocks = memoryPool->poolSizes[pool][ePoolSize] / memoryPool->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += memoryPool->poolSizes[pool][eBlockSize])
		{
			if(memoryPool->poolLocation[pool][displacement])
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * memoryPool->poolSizes[pool][eBlockSize];

		uint32 usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;

		Printer::text("           ", x, originalY + 3 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printer::int32(memoryPool->poolSizes[pool][eBlockSize], x, y, NULL);
			Printer::int32(usedBlocksPercentage, x + 11 - Math::getDigitsCount(usedBlocksPercentage), y, NULL);
			Printer::text("% ", x + 11, y++, NULL);
		}

		totalUsedBlocks = 0 ;
	}

	y = originalY;
	int32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printer::int32(usedBytesPercentage, x + 11 - Math::getDigitsCount(usedBytesPercentage), y, NULL);
	Printer::text("% ", x + 11, y++, NULL);
	Printer::text("Used: ", x, ++y, NULL);
	Printer::int32(totalUsedBytes, x + 12 - Math::getDigitsCount(totalUsedBytes), y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void MemoryPool::printDetailedUsage(int32 x, int32 y)
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	uint32 i;
	uint32 totalUsedBlocks = 0;
	uint32 totalUsedBytes = 0;
	uint32 pool;
	uint32 displacement = 0;

	Printer::text("MEMORY POOLS STATUS", x, y++, NULL);

	Printer::text("Pool Free Used", x, ++y, NULL);
	Printer::text("\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, ++y, NULL);

	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 totalBlocks = memoryPool->poolSizes[pool][ePoolSize] / memoryPool->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += memoryPool->poolSizes[pool][eBlockSize])
		{
			if(*((uint32*)&memoryPool->poolLocation[pool][displacement]))
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * memoryPool->poolSizes[pool][eBlockSize];

		Printer::text("	              ", x, ++y, NULL);
		Printer::int32(memoryPool->poolSizes[pool][eBlockSize],  x, y, NULL);
		Printer::int32(memoryPool->poolSizes[pool][ePoolSize] / memoryPool->poolSizes[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
		Printer::int32(totalUsedBlocks, x + 10, y, NULL);

		int32 usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		Printer::int32(usedBlocksPercentage, x + 17 - Math::getDigitsCount(usedBlocksPercentage), y, NULL);
		Printer::text("% ", x + 17, y, NULL);

		totalUsedBlocks = 0 ;
	}

	++y;
	uint32 poolSize = MemoryPool::getPoolSize(memoryPool);
	Printer::text("Pool size: ", x, ++y, NULL);
	Printer::int32(poolSize, x + 18 - Math::getDigitsCount(poolSize), y, NULL);

	Printer::text("Pool usage: ", x, ++y, NULL);
	Printer::int32(totalUsedBytes, x + 18 - Math::getDigitsCount(totalUsedBytes), y++, NULL);

	uint32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printer::int32(usedBytesPercentage, x + 17 - Math::getDigitsCount(usedBytesPercentage), y, NULL);
	Printer::text("% ", x + 17, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MemoryPool::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	MemoryPool::reset(this);
	MemoryPool::cleanUp(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

 void MemoryPool::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MemoryPool::reset()
{
	uint32 pool = 0;
	uint32 i;

	// Initialize pool's sizes and pointers
	__SET_MEMORY_POOL_ARRAYS

	// Clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for(i = 0; i < this->poolSizes[pool][ePoolSize]; i++)
		{
			*((uint32*)&this->poolLocation[pool][i]) = __MEMORY_FREE_BLOCK_FLAG;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MemoryPool::cleanUp()
{
	uint32 pool = 0;

	// Clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 i = 0;
		for(; i < this->poolSizes[pool][ePoolSize]; i += this->poolSizes[pool][eBlockSize])
		{
			if(!*((uint32*)&this->poolLocation[pool][i]))
			{
				uint32 j = i;
				for(; j < this->poolSizes[pool][eBlockSize]; j++)
				{
					this->poolLocation[pool][j] = 0;
				}
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 MemoryPool::getPoolSize()
{
	uint32 size = 0;
	uint32 pool = 0;

	// Clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		size += this->poolSizes[pool][ePoolSize];
	}

	return size;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
