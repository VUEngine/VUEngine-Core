/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MemoryPool.h>
#include <Game.h>
#include <Utilities.h>
#include <Types.h>
#include <Printing.h>
#include <HardwareManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

// it is necessary for the object to be aligned to multiples of 2
#define __MEMORY_USED_BLOCK_FLAG	0xFFFFFFFF
#define __MEMORY_FREE_BLOCK_FLAG	0x00000000


// MemoryPool's defines
#define __BLOCK_DEFINITION(BlockSize, Elements)															\
	BYTE pool ## BlockSize ## B[BlockSize * Elements]; 													\

#define __SET_MEMORY_POOL_ARRAY(BlockSize)																\
	this->poolLocation[pool] = &this->pool ## BlockSize ## B[0];										\
	this->poolSizes[pool][ePoolSize] = sizeof(this->pool ## BlockSize ## B);							\
	this->poolSizes[pool++][eBlockSize] = BlockSize;													\


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MemoryPool_ATTRIBUTES																			\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* dynamic memory area */																		\
		/* must always put together the pools! */														\
		/* first byte is used as a usage flag */														\
		__MEMORY_POOL_ARRAYS																			\
		/* pointer to the beginning of each memory pool */												\
		BYTE* poolLocation[__MEMORY_POOLS];																\
		/* pool's size and pool's block size */															\
		int poolSizes[__MEMORY_POOLS][2];																\

/**
 * @class 	MemoryPool
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(MemoryPool, Object);

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize
};


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MemoryPool_constructor(MemoryPool this);
static void MemoryPool_reset(MemoryPool this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// defines a singleton (unique instance of a class)
#define __MEMORY_POOL_SINGLETON(ClassName)																\
																										\
	/* declare the static instance */																	\
	static ClassName ## _str _instance ## ClassName __MEMORY_POOL_SECTION_ATTRIBUTE;					\
																										\
	/* global pointer to speed up allocation and free */												\
	ClassName _memoryPool __INITIALIZED_DATA_SECTION_ATTRIBUTE = &_instance ## ClassName;				\
																										\
	/* a flag to know when to allow construction */														\
	static s8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE								\
									= __SINGLETON_NOT_CONSTRUCTED;										\
																										\
	/* define get instance method */																	\
	static void __attribute__ ((noinline)) ClassName ## _instantiate()									\
	{																									\
		NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,								\
			ClassName get instance during construction);												\
																										\
		_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;											\
																										\
		/* set the vtable pointer */																	\
		_instance ## ClassName.vTable = &ClassName ## _vTable;											\
																										\
		/* call constructor */																			\
		ClassName ## _constructor(&_instance ## ClassName);												\
																										\
		/* set the vtable pointer */																	\
		_instance ## ClassName.vTable = &ClassName ## _vTable;											\
																										\
		/* don't allow more constructs */																\
		_singletonConstructed = __SINGLETON_CONSTRUCTED;												\
	}																									\
																										\
	/* define get instance method */																	\
	ClassName ClassName ## _getInstance()																\
	{																									\
		/* first check if not constructed yet */														\
		if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)										\
		{																								\
			ClassName ## _instantiate();																\
		}																								\
																										\
		/* return the created singleton */																\
		return &_instance ## ClassName;																	\
	}


/**
 * Get instance
 *
 * @fn			MemoryPool_getInstance()
 * @memberof	MemoryPool
 * @public
 *
 * @return		MemoryPool instance
 */
__MEMORY_POOL_SINGLETON(MemoryPool)

/**
 * Class constructor
 *
 * @memberof	MemoryPool
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) MemoryPool_constructor(MemoryPool this)
{
	ASSERT(this, "MemoryPool::constructor: null this");

	__CONSTRUCT_BASE(Object);

	MemoryPool_reset(this);
	MemoryPool_cleanUp(this);
}

/**
 * Class destructor
 *
 * @memberof	MemoryPool
 * @public
 *
 * @param this	Function scope
 */
 void MemoryPool_destructor(MemoryPool this)
{
	ASSERT(this, "MemoryPool::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Allocate a given amount of bytes in one of the memory pools
 *
 * @memberof				Error
 * @public
 *
 * @param this				Function scope
 * @param numberOfBytes		Number of bytes to allocate
 *
 * @return					Pointer to the memory pool entry allocated
 */
 BYTE* MemoryPool_allocate(MemoryPool this, int numberOfBytes)
{
	ASSERT(this, "MemoryPool::allocate: null this");

	int lp = HardwareManager_getLinkPointer(HardwareManager_getInstance());

	int i = 0;
	int numberOfOjects = 0;
	int pool = __MEMORY_POOLS - 1;
	int blockSize = this->poolSizes[pool][eBlockSize];
	int displacement = 0;

	// seach for the shortest pool which can hold the data
	for(; numberOfBytes > blockSize && pool--; blockSize = this->poolSizes[pool][eBlockSize]);

	if(0 > pool)
	{
		Printing_setDebugMode(Printing_getInstance());
		Printing_clear(Printing_getInstance());
		Printing_text(Printing_getInstance(), "Block's size requested: ", 20, 12, NULL);
		Printing_int(Printing_getInstance(), numberOfBytes, 44, 12, NULL);
		Printing_text(Printing_getInstance(), "Caller address: ", 20, 15, NULL);
		Printing_hex(Printing_getInstance(), lp, 36, 15, 8, NULL);

		NM_ASSERT(pool >= 0, "MemoryPool::allocate: object size overflow");
	}

	// get the number of allocable objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / blockSize;

	// look for a free block
	for(i = 0, displacement = 0;
		i < numberOfOjects && __MEMORY_FREE_BLOCK_FLAG != *((u32*)&this->poolLocation[pool][displacement]);
		i++, displacement += blockSize);

	if(i >= numberOfOjects)
	{
		Printing_setDebugMode(Printing_getInstance());
		Printing_clear(Printing_getInstance());
	//	MemoryPool_printDetailedUsage(this, 1, 8);
		Printing_text(Printing_getInstance(), "Pool's size: ", 20, 12, NULL);
		Printing_int(Printing_getInstance(), blockSize, 44, 12, NULL);
		Printing_text(Printing_getInstance(), "Block's size requested: ", 20, 13, NULL);
		Printing_int(Printing_getInstance(), numberOfBytes, 44, 13, NULL);
		Printing_text(Printing_getInstance(), "Caller address: ", 20, 15, NULL);

		Printing_hex(Printing_getInstance(), lp, 36, 15, 8, NULL);

		NM_ASSERT(false, "MemoryPool::allocate: pool exhausted");
	}

	// mark address as allocated
	*((u32*)&this->poolLocation[pool][displacement]) = __MEMORY_USED_BLOCK_FLAG;

	// return designed address
	return &this->poolLocation[pool][displacement];
}

/**
 * Free the memory pool entry were the given object is allocated
 *
 * @memberof		MemoryPool
 * @public
 *
 * @param this		Function scope
 * @param object	Pointer to the memory pool entry to free
 */
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
	for(pool = 0; pool < __MEMORY_POOLS && object >= &this->poolLocation[pool][0]; pool++);

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
		if(object == &this->poolLocation[pool][displacement])
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
	*((u32*)&object[0]) = __MEMORY_FREE_BLOCK_FLAG;
}

/**
 * Clear all memory pool
 *
 * @memberof		MemoryPool
 * @private
 *
 * @param this		Function scope
 */
static void MemoryPool_reset(MemoryPool this)
{
	ASSERT(this, "MemoryPool::reset: null this");

	int pool = 0;
	int i;

	// initialize pool's sizes and pointers
	__SET_MEMORY_POOL_ARRAYS

	// clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for(i = 0; i < this->poolSizes[pool][ePoolSize]; i++)
		{
			*((u32*)&this->poolLocation[pool][i]) = __MEMORY_FREE_BLOCK_FLAG;
		}
	}
}

/**
 * Clear all memory pool
 *
 * @memberof		MemoryPool
 * @public
 *
 * @param this		Function scope
 */
void MemoryPool_cleanUp(MemoryPool this)
{
	ASSERT(this, "MemoryPool::reset: null this");

	int pool = 0;

	// clear all memory pool entries
	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		int i = 0;
		for(; i < this->poolSizes[pool][ePoolSize]; i += this->poolSizes[pool][eBlockSize])
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

/**
 * Retrieve the pool's size
 *
 * @memberof		MemoryPool
 * @public
 *
 * @param this		Function scope
 *
 * @return			Total size of the memory pool
 */
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

/**
 * Print the pools' detailed usage
 *
 * @memberof		MemoryPool
 * @public
 *
 * @param this		Function scope
 * @param x			Screen column for the output
 * @param y			Screen row for the output
 */
void MemoryPool_printDetailedUsage(MemoryPool this, int x, int y)
{
	ASSERT(this, "MemoryPool::printMemUsage: null this");

	int i;
	int totalUsedBlocks = 0;
	int totalUsedBytes = 0;
	int pool;
	int displacement = 0;

	Printing printing = Printing_getInstance();

	Printing_resetWorldCoordinates(printing);

	Printing_text(printing, "MEMORY POOL'S STATUS", x, y++, NULL);

	Printing_text(printing, "Pool", x, ++y, NULL);
	Printing_text(printing, "Free", x + 5, y, NULL);
	Printing_text(printing, "Used", x + 10, y, NULL);

	for(pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		int totalBlocks = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if(*((u32*)&this->poolLocation[pool][displacement]))
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolSizes[pool][eBlockSize];

		Printing_text(printing, "	              ", x, ++y, NULL);
		Printing_int(printing, this->poolSizes[pool][eBlockSize],  x, y, NULL);
		Printing_int(printing, this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
		Printing_int(printing, totalUsedBlocks, x + 10, y, NULL);

		int usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;
		Printing_int(printing, usedBlocksPercentage, x + 17 - Utilities_intLength(usedBlocksPercentage), y, NULL);
		Printing_text(printing, "% ", x + 17, y, NULL);

		totalUsedBlocks = 0 ;
	}

	++y;
	int poolSize = MemoryPool_getPoolSize(this);
	Printing_text(printing, "Pool size: ", x, ++y, NULL);
	Printing_int(printing, poolSize, x + 18 - Utilities_intLength(poolSize), y, NULL);

	Printing_text(printing, "Pool usage: ", x, ++y, NULL);
	Printing_int(printing, totalUsedBytes, x + 18 - Utilities_intLength(totalUsedBytes), y++, NULL);

	int usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing_int(printing, usedBytesPercentage, x + 17 - Utilities_intLength(usedBytesPercentage), y, NULL);
	Printing_text(printing, "% ", x + 17, y++, NULL);
}

/**
 * Print the pools' resumed usage
 *
 * @memberof		MemoryPool
 * @public
 *
 * @param this		Function scope
 * @param x			Screen column for the output
 * @param y			Screen row for the output
 */
void MemoryPool_printResumedUsage(MemoryPool this, int x, int y)
{
	ASSERT(this, "MemoryPool::printMemUsage: null this");

	int originalY = y;
	int i;
	int totalUsedBlocks = 0;
	int totalUsedBytes = 0;
	int pool;
	int displacement = 0;

	Printing printing = Printing_getInstance();

	Printing_text(printing, "MEMORY:", x, y, NULL);
	int poolSize = MemoryPool_getPoolSize(MemoryPool_getInstance());
	Printing_text(printing, "Total: ", x, ++y, NULL);
	Printing_int(printing, poolSize, x + 12 - Utilities_intLength(poolSize), y, NULL);

	for(y += 2, pool = 0; pool < __MEMORY_POOLS; pool++)
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

		Printing_text(printing, "           ", x, originalY + 3 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printing_int(printing, this->poolSizes[pool][eBlockSize], x, y, NULL);
			Printing_int(printing, usedBlocksPercentage, x + 7 - Utilities_intLength(usedBlocksPercentage), y, NULL);
			Printing_text(printing, "% ", x + 7, y++, NULL);
		}

		totalUsedBlocks = 0 ;
	}

	y = originalY;
	int usedBytesPercentage = (100 * totalUsedBytes) / poolSize;
	Printing_int(printing, usedBytesPercentage, x + 11 - Utilities_intLength(usedBytesPercentage), y, NULL);
	Printing_text(printing, "% ", x + 11, y++, NULL);
	Printing_text(printing, "Used: ", x, ++y, NULL);
	Printing_int(printing, totalUsedBytes, x + 12 - Utilities_intLength(totalUsedBytes), y++, NULL);
}
