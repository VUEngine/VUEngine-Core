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
#include <HardwareManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __BITS_IN_U32_TYPE			32	//(sizeof(u32) * 8)
#define __BITS_IN_U32_TYPE_EXP		5	// 2 ^ 5 = (sizeof(u32) * 8)


// MemoryPool's defines
#define __BLOCK_DEFINITION(BlockSize, Elements)															\
	BYTE pool ## BlockSize ## B[BlockSize * Elements]; 													\
	u32 pool ## BlockSize ## Directory[(Elements / __BITS_IN_U32_TYPE) + 								\
										((Elements % __BITS_IN_U32_TYPE) ? 1 : 0)];						\

#define __SET_MEMORY_POOL_ARRAY(BlockSize)																\
	{																									\
		int i = 0;																						\
		int totalBlocks = sizeof(this->pool ## BlockSize ## B) / BlockSize;								\
		int totalDirectoryEntries = (totalBlocks >> __BITS_IN_U32_TYPE_EXP) + 							\
									(__MODULO(totalBlocks, __BITS_IN_U32_TYPE) ? 1 : 0);				\
		for(; i < totalDirectoryEntries; i++)															\
		{																								\
			this->pool ## BlockSize ## Directory[i] = 0;												\
		}																								\
		int remainder = totalBlocks % __BITS_IN_U32_TYPE;												\
		u32 mask = 0xFFFFFFFF << remainder;																\
		this->pool ## BlockSize ## Directory[i - 1] = mask;												\
		this->poolLocation[pool] = &this->pool ## BlockSize ## B[0];									\
		this->poolDirectory[pool] = &this->pool ## BlockSize ## Directory[0];							\
		this->poolInfo[pool][ePoolSize] = sizeof(this->pool ## BlockSize ## B);							\
		this->poolInfo[pool][eBlockSize] = BlockSize;													\
		this->poolInfo[pool][eDirectoryEntries] = totalDirectoryEntries;								\
		this->poolInfo[pool++][eLastFreeBlockIndex] = 0;												\
	}


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
		u32* poolDirectory[__MEMORY_POOLS];																\
		/* pool's size and pool's block size */															\
		u16 poolInfo[__MEMORY_POOLS][4];																\

/**
 * @class 	MemoryPool
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(MemoryPool, Object);

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize,
	eDirectoryEntries,
	eLastFreeBlockIndex,
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

	int pool = __MEMORY_POOLS;
	int blockSize = this->poolInfo[pool][eBlockSize];

	bool blockFound = false;

	while(!blockFound && pool--)
	{
		// search for the smallest pool which can hold the data
		blockSize = this->poolInfo[pool][eBlockSize];
		int directoryEntries = this->poolInfo[pool][eDirectoryEntries];

		if(numberOfBytes <= blockSize)
		{
			int i = 0;
        	int j = 0;

			for(i = this->poolInfo[pool][eLastFreeBlockIndex], j = i;
				(i < directoryEntries) | (0 <= j);
				i++, j--
			)
			{
				blockFound = i < directoryEntries && (this->poolDirectory[pool][i] ^ 0xFFFFFFFF);

				if(blockFound)
				{
					this->poolInfo[pool][eLastFreeBlockIndex] = i;
					break;
				}

				blockFound = 0 <= j && (this->poolDirectory[pool][j] ^ 0xFFFFFFFF);

				if(blockFound)
				{
					this->poolInfo[pool][eLastFreeBlockIndex] = j;
					break;
				}
			}

			// keep looking for a free block on a bigger pool
		}
	}

	if(!blockFound)
	{
		Printing_setDebugMode(Printing_getInstance());
		Printing_clear(Printing_getInstance());
		MemoryPool_printDetailedUsage(this, 1, 8);
		Printing_text(Printing_getInstance(), "Pool's size: ", 20, 12, NULL);
		Printing_int(Printing_getInstance(), blockSize, 44, 12, NULL);
		Printing_text(Printing_getInstance(), "Block's size requested: ", 20, 13, NULL);
		Printing_int(Printing_getInstance(), numberOfBytes, 44, 13, NULL);
		Printing_text(Printing_getInstance(), "Caller address: ", 20, 15, NULL);

		Printing_hex(Printing_getInstance(), lp, 36, 15, 8, NULL);

		MemoryPool_printDirectory(this, 30, 20, 7);

		NM_ASSERT(false, "MemoryPool::allocate: pool exhausted");
	}

	u32 directoryIndex = this->poolInfo[pool][eLastFreeBlockIndex];
	u32 directoryEntry = this->poolDirectory[pool][directoryIndex];
	u32 blockDisplacement = 0;

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	while(directoryEntry & 0x00000001)
	{
		directoryEntry >>= 1;
		blockDisplacement++;
	}

	// mark block as used before flagging the actual block
	this->poolDirectory[pool][directoryIndex] |= (0x00000001 << blockDisplacement);

	int freeBlockIndex = ((directoryIndex << __BITS_IN_U32_TYPE_EXP) + blockDisplacement) * blockSize;

	// mark block as allocated
	ASSERT(__MEMORY_FREE_BLOCK_FLAG == *((u32*)&this->poolLocation[pool][freeBlockIndex]), "MemoryPool::allocate: block is not free");
	*((u32*)&this->poolLocation[pool][freeBlockIndex]) = __MEMORY_USED_BLOCK_FLAG;

	// return designed address
	return &this->poolLocation[pool][freeBlockIndex];
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
	ASSERT(object, "MemoryPool::free: null object");

	if((unsigned)((int)object - (int)&this->poolLocation[0][0]) > (unsigned)((int)&this->poolLocation[__MEMORY_POOLS - 1][this->poolInfo[__MEMORY_POOLS - 1][ePoolSize]] - (int)&this->poolLocation[0][0]))
	{
		return;
	}

	int pool = __MEMORY_POOLS;

	// look for the pool containing the object
	for(; pool-- && (u32)object < (u32)this->poolLocation[pool];);

	// look for the registry in which the object is
	NM_ASSERT(0 <= pool , "MemoryPool::free: deleting something not allocated");

	u32 displacement = (((u32)object) - ((u32)this->poolLocation[pool]));

	// thrown exception
	NM_ASSERT(object == &this->poolLocation[pool][displacement], "MemoryPool::free: deleting something not allocated");

	// mark block as free
	*(u32*)((u32)object) = __MEMORY_FREE_BLOCK_FLAG;

	u32 objectBlockIndex = displacement / this->poolInfo[pool][eBlockSize];

	// mark block in the dictionary
	this->poolDirectory[pool][objectBlockIndex >> __BITS_IN_U32_TYPE_EXP] &= (0x00000001 << __MODULO(objectBlockIndex, __BITS_IN_U32_TYPE)) ^ 0xFFFFFFFF;
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
		for(i = 0; i < this->poolInfo[pool][ePoolSize]; i++)
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
		for(; i < this->poolInfo[pool][ePoolSize]; i += this->poolInfo[pool][eBlockSize])
		{
			if(!*((u32*)&this->poolLocation[pool][i]))
			{
				int j = i;
				for(; j < this->poolInfo[pool][eBlockSize]; j++)
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
		size += this->poolInfo[pool][ePoolSize];
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
 * @param x			Camera column for the output
 * @param y			Camera row for the output
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
		int totalBlocks = this->poolInfo[pool][ePoolSize] / this->poolInfo[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolInfo[pool][eBlockSize])
		{
			if(*((u32*)&this->poolLocation[pool][displacement]))
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolInfo[pool][eBlockSize];

		Printing_text(printing, "	              ", x, ++y, NULL);
		Printing_int(printing, this->poolInfo[pool][eBlockSize],  x, y, NULL);
		Printing_int(printing, this->poolInfo[pool][ePoolSize] / this->poolInfo[pool][eBlockSize] - totalUsedBlocks, x + 5, y, NULL);
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
 * @param x			Camera column for the output
 * @param y			Camera row for the output
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
		int totalBlocks = this->poolInfo[pool][ePoolSize] / this->poolInfo[pool][eBlockSize];
		for(displacement = 0, i = 0, totalUsedBlocks = 0 ; i < totalBlocks; i++, displacement += this->poolInfo[pool][eBlockSize])
		{
			if(this->poolLocation[pool][displacement])
			{
				totalUsedBlocks++;
			}
		}

		totalUsedBytes += totalUsedBlocks * this->poolInfo[pool][eBlockSize];

		int usedBlocksPercentage = (100 * totalUsedBlocks) / totalBlocks;

		Printing_text(printing, "           ", x, originalY + 3 + pool, NULL);

		if(__MEMORY_POOL_WARNING_THRESHOLD < usedBlocksPercentage)
		{
			Printing_int(printing, this->poolInfo[pool][eBlockSize], x, y, NULL);
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

void MemoryPool_printDirectory(MemoryPool this, int x, int y, int pool)
{
	pool = pool < __MEMORY_POOLS ? pool : __MEMORY_POOLS - 1;

	for(; pool < __MEMORY_POOLS; pool++)
	{
		int blockSize = this->poolInfo[pool][eBlockSize];
		int directoryIndex = this->poolInfo[pool][eDirectoryEntries];

		int col = 0;
		int row = 0;

		for(; directoryIndex--;)
		{
			PRINT_HEX(this->poolDirectory[pool][directoryIndex], x + col, y + row++);
		}
		break;
	}
}
