/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MemoryPool.h>
#include <Utilities.h>
#include <Types.h>
#include <HardwareManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

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
		ClassName _memoryPool __INITIALIZED_DATA_SECTION_ATTRIBUTE = 									\
			&_singletonWrapper ## ClassName.instance;													\
																										\
		/* a flag to know when to allow construction */													\
		static int8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE							\
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
			ClassName ## _checkVTable();																\
																										\
			/*  */																						\
			ClassName instance = &_singletonWrapper ## ClassName.instance;								\
			_singletonWrapper ## ClassName.objectMemoryFootprint = __OBJECT_MEMORY_FOOT_PRINT;			\
																										\
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
		ClassName ClassName ## _getInstance()															\
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

/**
 * Class constructor
 *
 * @private
 */
void MemoryPool::constructor()
{
	Base::constructor();

	MemoryPool::reset(this);
	MemoryPool::cleanUp(this);
}

/**
 * Class destructor
 */
 void MemoryPool::destructor()
{
	// allow a new construct
	Base::destructor();
}


/**
 * Clear all memory pool
 *
 * @private
 */
void MemoryPool::reset()
{
	uint32 pool = 0;
	uint32 i;

	// initialize pool's sizes and pointers
	__SET_MEMORY_POOL_ARRAYS

	// clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for(i = 0; i < this->poolSizes[pool][ePoolSize]; i++)
		{
			*((uint32*)&this->poolLocation[pool][i]) = __MEMORY_FREE_BLOCK_FLAG;
		}
	}
}

/**
 * Clear all memory pool
 */
void MemoryPool::cleanUp()
{
	uint32 pool = 0;

	// clear all memory pool entries
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

/**
 * Retrieve the pool's size
 *
 * @return			Total size of the memory pool
 */
uint32 MemoryPool::getPoolSize()
{
	uint32 size = 0;
	uint32 pool = 0;

	// clear all allocable objects usage
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		size += this->poolSizes[pool][ePoolSize];
	}

	return size;
}

/**
 * Print the pools' detailed usage
 *
 * @param x			Camera column for the output
 * @param y			Camera row for the output
 */
void MemoryPool::printDetailedUsage(int32 x, int32 y)
{
	uint32 i;
	uint32 totalUsedBlocks = 0;
	uint32 totalUsedBytes = 0;
	uint32 pool;
	uint32 displacement = 0;

	Printing printing = Printing::getInstance();

	Printing::resetCoordinates(printing);

	Printing::text(printing, "MEMORY POOLS STATUS", x, y++, NULL);

	Printing::text(printing, "Pool Free Used", x, ++y, NULL);
	Printing::text(printing, "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, ++y, NULL);

	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 totalBlocks = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if(*((uint32*)&this->poolLocation[pool][displacement]))
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolSizes[pool][eBlockSize];

		Printing::text(printing, "	              ", x, ++y, NULL);
		Printing::int32(printing, this->poolSizes[pool][eBlockSize],  x, y, NULL);
		Printing::int32(printing, this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
		Printing::int32(printing, totalUsedBlocks, x + 10, y, NULL);

		int32 usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		Printing::int32(printing, usedBlocksPercentage, x + 17 - Utilities::intLength(usedBlocksPercentage), y, NULL);
		Printing::text(printing, "% ", x + 17, y, NULL);

		totalUsedBlocks = 0 ;
	}

	++y;
	uint32 poolSize = MemoryPool::getPoolSize(this);
	Printing::text(printing, "Pool size: ", x, ++y, NULL);
	Printing::int32(printing, poolSize, x + 18 - Utilities::intLength(poolSize), y, NULL);

	Printing::text(printing, "Pool usage: ", x, ++y, NULL);
	Printing::int32(printing, totalUsedBytes, x + 18 - Utilities::intLength(totalUsedBytes), y++, NULL);

	uint32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing::int32(printing, usedBytesPercentage, x + 17 - Utilities::intLength(usedBytesPercentage), y, NULL);
	Printing::text(printing, "% ", x + 17, y++, NULL);
}

/**
 * Print the pools' resumed usage
 *
 * @param x			Camera column for the output
 * @param y			Camera row for the output
 */
void MemoryPool::printResumedUsage(int32 x, int32 y)
{
	uint32 originalY = y;
	uint32 i;
	uint32 totalUsedBlocks = 0;
	uint32 totalUsedBytes = 0;
	uint32 pool;
	uint32 displacement = 0;

	Printing printing = Printing::getInstance();

	Printing::text(printing, "MEMORY:", x, y, NULL);
	uint32 poolSize = MemoryPool::getPoolSize(MemoryPool::getInstance());
	Printing::text(printing, "Total: ", x, ++y, NULL);
	Printing::int32(printing, poolSize, x + 12 - Utilities::intLength(poolSize), y, NULL);

	for(y += 2, pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		uint32 totalBlocks = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if(this->poolLocation[pool][displacement])
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolSizes[pool][eBlockSize];

		uint32 usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;

		Printing::text(printing, "           ", x, originalY + 3 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printing::int32(printing, this->poolSizes[pool][eBlockSize], x, y, NULL);
			Printing::int32(printing, usedBlocksPercentage, x + 7 - Utilities::intLength(usedBlocksPercentage), y, NULL);
			Printing::text(printing, "% ", x + 7, y++, NULL);
		}

		totalUsedBlocks = 0 ;
	}

	y = originalY;
	int32 usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing::int32(printing, usedBytesPercentage, x + 11 - Utilities::intLength(usedBytesPercentage), y, NULL);
	Printing::text(printing, "% ", x + 11, y++, NULL);
	Printing::text(printing, "Used: ", x, ++y, NULL);
	Printing::int32(printing, totalUsedBytes, x + 12 - Utilities::intLength(totalUsedBytes), y++, NULL);
}

/**
 * Free the memory pool entry were the given object is allocated
 *
 * @param object	Pointer to the memory pool entry to free
 */
void MemoryPool::free(BYTE* object)
{
	NM_ASSERT(__SINGLETON_NOT_CONSTRUCTED != _singletonConstructed, "MemoryPool::free: no properly constructed yet");

#ifdef __DEBUG
	if(!(object >= &this->poolLocation[0][0] && object < &this->poolLocation[__MEMORY_POOLS - 1][0] + this->poolSizes[__MEMORY_POOLS - 1][ePoolSize]))
	{
		return;
	}
#endif

	if(NULL == object)
	{
		return;
	}

	uint32 pool = 0;

	// look for the pool containing the object
	for(pool = 0; pool < __MEMORY_POOLS && object >= &this->poolLocation[pool][0]; pool++);

	// look for the registry in which the object is
	ASSERT(pool <= __MEMORY_POOLS , "MemoryPool::free: deleting something not allocated");

	// move one pool back since the above loop passed the target by one
	pool--;

	this->poolLastFreeBlock[pool] = object;

#ifdef __DEBUG
	// get the total objects in the pool
	uint32 numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	HardwareManager::suspendInterrupts();

	// search for the pool in which it is allocated
	for(uint32 i = 0, displacement = 0; i < numberOfOjects; i++, displacement += this->poolSizes[pool][eBlockSize])
	{
		// if the object has been found
		if(object == &this->poolLocation[pool][displacement])
		{
			// free the block
			*(uint32*)((uint32)object) = __MEMORY_FREE_BLOCK_FLAG;
			HardwareManager::resumeInterrupts();
			return;
		}
	}

	// thrown exception
	ASSERT(false, "MemoryPool::free: deleting something not allocated");

#endif

	// set address as free
	*(uint32*)((uint32)object) = __MEMORY_FREE_BLOCK_FLAG;
}

// Have to undefine these in order for the lp to not get corrupted by the checks on the this pointer
#undef ASSERT
#undef NM_ASSERT
#undef __SAFE_CAST

#define ASSERT(Statement, ...)
#define NM_ASSERT(Statement, ...)
#define __SAFE_CAST(ClassName, object) (ClassName)object

/**
 * Allocate a given amount of bytes in one of the memory pools
 *
 * @param numberOfBytes		Number of bytes to allocate
 * @return					Pointer to the memory pool entry allocated
 */
BYTE* MemoryPool::allocate(int32 numberOfBytes)
{
	NM_ASSERT(__SINGLETON_NOT_CONSTRUCTED != _singletonConstructed, "MemoryPool::allocate: no properly constructed yet");

#ifndef __SHIPPING
#ifndef __RELEASE
	int32 lp = HardwareManager::getLinkPointer();
#endif
#endif

	uint32 pool = __MEMORY_POOLS;

	HardwareManager::suspendInterrupts();

	while(pool--)
	{
		uint32 blockSize = this->poolSizes[pool][eBlockSize];

		if((uint32)numberOfBytes > blockSize)
		{
			continue;
		}

		uint32 numberOfOjects = this->poolSizes[pool][ePoolSize] / blockSize;
		BYTE* poolLocationStart = &this->poolLocation[pool][0];
		BYTE* poolLocationLeft = this->poolLastFreeBlock[pool];
		BYTE* poolLocationRight = poolLocationLeft + blockSize;
		BYTE* poolLocationEnd = &this->poolLocation[pool][blockSize * (numberOfOjects - 1)];

		BYTE* poolLocation = NULL;

		do
		{
			if(poolLocationRight <= poolLocationEnd)
			{
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint32*)poolLocationRight))
				{
					poolLocation = poolLocationRight;
					break;
				}
				else if(__MEMORY_FREE_BLOCK_FLAG == *((uint32*)poolLocationEnd))
				{
					poolLocation = poolLocationEnd;
					break;
				}

				poolLocationRight += blockSize;
				poolLocationEnd -= blockSize;
			}

			if(poolLocationLeft >= poolLocationStart)
			{
				if(__MEMORY_FREE_BLOCK_FLAG == *((uint32*)poolLocationLeft))
				{
					poolLocation = poolLocationLeft;
					break;
				}
				else if(__MEMORY_FREE_BLOCK_FLAG == *((uint32*)poolLocationStart))
				{
					poolLocation = poolLocationStart;
					break;
				}

				poolLocationLeft -= blockSize;
				poolLocationStart += blockSize;
			}
		}
		while((poolLocationStart <= poolLocationLeft) || (poolLocationEnd >= poolLocationRight));
		// keep looking for a free block on a bigger pool

		if(NULL != poolLocation)
		{
			*((uint32*)poolLocation) = __MEMORY_USED_BLOCK_FLAG;
			this->poolLastFreeBlock[pool] = poolLocation;
			HardwareManager::resumeInterrupts();
			return poolLocation;
		}
	}

#ifndef __SHIPPING
	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	MemoryPool::printDetailedUsage(this, 1, 8);
	Printing::text(Printing::getInstance(), "Block's size requested: ", 20, 26, NULL);
	Printing::int32(Printing::getInstance(), numberOfBytes, 44, 26, NULL);
#ifndef __RELEASE
	Printing::text(Printing::getInstance(), "Caller address: ", 20, 27, NULL);
	Printing::hex(Printing::getInstance(), lp, 36, 27, 8, NULL);
#endif

	Error::triggerException("MemoryPool::allocate: pool exhausted", NULL);		
#endif

	// return designed address
	return NULL;
}
