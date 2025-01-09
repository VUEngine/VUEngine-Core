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
#include <Printing.h>
#include <Utilities.h>

#include "MemoryPool.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// defines a singleton (unique instance of a class)
#define __MEMORY_POOL_SINGLETON(ClassName)																\
																										\
		/* declare the static instance */																\
		typedef struct SingletonWrapper ## ClassName													\
		{																								\
			/* footprint to differentiate between objects and structs */								\
			uint32 objectMemoryFootprint;																\
			/* declare the static instance */															\
			ClassName ## _str instance;																	\
		} SingletonWrapper ## ClassName;																\
																										\
		static SingletonWrapper ## ClassName _singletonWrapper ## ClassName 							\
				__MEMORY_POOL_SECTION_ATTRIBUTE;														\
																										\
		/* global pointer to speed up allocation and free */											\
		ClassName _memoryPool __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 							\
			&_singletonWrapper ## ClassName.instance;													\
																										\
		/* a flag to know when to allow construction */													\
		static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE					\
										= __SINGLETON_NOT_CONSTRUCTED;									\
																										\
		/* define get instance method */																\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()								\
		{																								\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,							\
				ClassName get instance during construction);											\
																										\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;										\
																										\
			/* make sure that the class is properly set */												\
			__CALL_CHECK_VTABLE(ClassName);																\
																										\
			/*  */																						\
			ClassName instance = &_singletonWrapper ## ClassName.instance;								\
			_singletonWrapper ## ClassName.objectMemoryFootprint 										\
				=  (__OBJECT_MEMORY_FOOT_PRINT << 16) | -1;												\
			/* set the vtable pointer */																\
			instance->vTable = &ClassName ## _vTable;													\
																										\
			/* call constructor */																		\
			ClassName ## _constructor(instance);														\
																										\
			/* set the vtable pointer */																\
			instance->vTable = &ClassName ## _vTable;													\
																										\
			/* don't allow more constructs */															\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;											\
		}																								\
																										\
		/* define get instance method */																\
		static ClassName ClassName ## _getInstance()													\
		{																								\
			/* first check if not constructed yet */													\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)									\
			{																							\
				ClassName ## _instantiate();															\
			}																							\
																										\
			/* return the created singleton */															\
			return &_singletonWrapper ## ClassName.instance;											\
		}																								\
																										\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */						\
		void ClassName ## dummyMethodSingleton()

// Have to undefine these in order for the lp to not get corrupted by the checks on the memoryPool pointer
#undef ASSERT
#undef NM_ASSERT
#undef __SAFE_CAST

#define ASSERT(Statement, ...)
#define NM_ASSERT(Statement, ...)
#define __SAFE_CAST(ClassName, object) (ClassName)object

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static BYTE* MemoryPool::allocate(int32 numberOfBytes)
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

		BYTE* poolLocationStart = &memoryPool->poolLocation[pool][0];
		BYTE* poolLocationLeft = memoryPool->poolLastFreeBlock[pool];
		BYTE* poolLocationRight = poolLocationLeft + blockSize;
		BYTE* poolLocationEnd = poolLocationStart + memoryPool->poolSizes[pool][ePoolSize] - blockSize;
		BYTE* poolLocation = NULL;

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
	Printing::setDebugMode();
	Printing::clear();
	MemoryPool::printDetailedUsage(1, 8);
	Printing::text("Block's size requested: ", 20, 26, NULL);
	Printing::int32(numberOfBytes, 44, 26, NULL);
#ifndef __RELEASE
	Printing::text("Caller address: ", 20, 27, NULL);
	Printing::hex(lp, 36, 27, 8, NULL);
#endif

	Error::triggerException("MemoryPool::allocate: pool exhausted", NULL);		
#endif

	// Return designed address
	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void MemoryPool::free(BYTE* object)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	

	Printing::resetCoordinates();

	Printing::text("MEMORY:", x, y, NULL);
	uint32 poolSize = MemoryPool::getPoolSize();
	Printing::text("Total: ", x, ++y, NULL);
	Printing::int32(poolSize, x + 12 - Math::getDigitsCount(poolSize), y, NULL);

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

		Printing::text("           ", x, originalY + 3 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printing::int32(memoryPool->poolSizes[pool][eBlockSize], x, y, NULL);
			Printing::int32(usedBlocksPercentage, x + 7 - Math::getDigitsCount(usedBlocksPercentage), y, NULL);
			Printing::text("% ", x + 7, y++, NULL);
		}

		totalUsedBlocks = 0 ;
	}

	y = originalY;
	int32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing::int32(usedBytesPercentage, x + 11 - Math::getDigitsCount(usedBytesPercentage), y, NULL);
	Printing::text("% ", x + 11, y++, NULL);
	Printing::text("Used: ", x, ++y, NULL);
	Printing::int32(totalUsedBytes, x + 12 - Math::getDigitsCount(totalUsedBytes), y++, NULL);
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

	

	Printing::resetCoordinates();

	Printing::text("MEMORY POOLS STATUS", x, y++, NULL);

	Printing::text("Pool Free Used", x, ++y, NULL);
	Printing::text("\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, ++y, NULL);

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

		Printing::text("	              ", x, ++y, NULL);
		Printing::int32(memoryPool->poolSizes[pool][eBlockSize],  x, y, NULL);
		Printing::int32(memoryPool->poolSizes[pool][ePoolSize] / memoryPool->poolSizes[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
		Printing::int32(totalUsedBlocks, x + 10, y, NULL);

		int32 usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		Printing::int32(usedBlocksPercentage, x + 17 - Math::getDigitsCount(usedBlocksPercentage), y, NULL);
		Printing::text("% ", x + 17, y, NULL);

		totalUsedBlocks = 0 ;
	}

	++y;
	uint32 poolSize = MemoryPool::getPoolSize(memoryPool);
	Printing::text("Pool size: ", x, ++y, NULL);
	Printing::int32(poolSize, x + 18 - Math::getDigitsCount(poolSize), y, NULL);

	Printing::text("Pool usage: ", x, ++y, NULL);
	Printing::int32(totalUsedBytes, x + 18 - Math::getDigitsCount(totalUsedBytes), y++, NULL);

	uint32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing::int32(usedBytesPercentage, x + 17 - Math::getDigitsCount(usedBytesPercentage), y, NULL);
	Printing::text("% ", x + 17, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void MemoryPool::reset()
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	uint32 pool = 0;
	uint32 i;

	// Initialize pool's sizes and pointers
	__SET_MEMORY_POOL_ARRAYS

	// Clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for(i = 0; i < memoryPool->poolSizes[pool][ePoolSize]; i++)
		{
			*((uint32*)&memoryPool->poolLocation[pool][i]) = __MEMORY_FREE_BLOCK_FLAG;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void MemoryPool::cleanUp()
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	uint32 pool = 0;

	// Clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 i = 0;
		for(; i < memoryPool->poolSizes[pool][ePoolSize]; i += memoryPool->poolSizes[pool][eBlockSize])
		{
			if(!*((uint32*)&memoryPool->poolLocation[pool][i]))
			{
				uint32 j = i;
				for(; j < memoryPool->poolSizes[pool][eBlockSize]; j++)
				{
					memoryPool->poolLocation[pool][j] = 0;
				}
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 MemoryPool::getPoolSize()
{
	MemoryPool memoryPool = MemoryPool::getInstance();

	uint32 size = 0;
	uint32 pool = 0;

	// Clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		size += memoryPool->poolSizes[pool][ePoolSize];
	}

	return size;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MemoryPool::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	MemoryPool::reset();
	MemoryPool::cleanUp();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

 void MemoryPool::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
