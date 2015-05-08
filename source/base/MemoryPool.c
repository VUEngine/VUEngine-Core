/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1OBJECTSPERHEAP1, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MemoryPool.h>
#include <Game.h>
#include <Utilities.h>
#include <Types.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

// it is neccesary for the object to be aligned to 2 multiples
#define __MEMORY_ALIGNMENT	4

// TODO: remove me
#define __MEMORY_POOLS		11

#define __MEMORY_POOL_ARRAYS													\
	__BLOCK_DEFINITION(192, 1)													\
	__BLOCK_DEFINITION(164, 4)													\
	__BLOCK_DEFINITION(136, 48)													\
	__BLOCK_DEFINITION(96, 48)													\
	__BLOCK_DEFINITION(80, 32)													\
	__BLOCK_DEFINITION(72, 64)													\
	__BLOCK_DEFINITION(64, 32)													\
	__BLOCK_DEFINITION(32, 64)													\
	__BLOCK_DEFINITION(28, 256)													\
	__BLOCK_DEFINITION(24, 640)													\
	__BLOCK_DEFINITION(20, 256)													\

#define __SET_MEMORY_POOL_ARRAYS												\
	__SET_MEMORY_POOL_ARRAY(192)												\
	__SET_MEMORY_POOL_ARRAY(164)												\
	__SET_MEMORY_POOL_ARRAY(136)												\
	__SET_MEMORY_POOL_ARRAY(96)													\
	__SET_MEMORY_POOL_ARRAY(80)													\
	__SET_MEMORY_POOL_ARRAY(72)													\
	__SET_MEMORY_POOL_ARRAY(64)													\
	__SET_MEMORY_POOL_ARRAY(32)													\
	__SET_MEMORY_POOL_ARRAY(28)													\
	__SET_MEMORY_POOL_ARRAY(24)													\
	__SET_MEMORY_POOL_ARRAY(20)													\

#define __MIN_BLOCK 		20

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MemoryPool_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* dynamic memory area */													\
	/* must always put together the pools! */									\
	/* first byte is used as a usage flag */									\
	__MEMORY_POOL_ARRAYS														\
																				\
	/* pointer to the beggining of each memory pool */							\
	BYTE* poolLocation[__MEMORY_POOLS];											\
																				\
	/* pool's size and pool's block size */										\
	int poolSizes[__MEMORY_POOLS][2];											\


__CLASS_DEFINITION(MemoryPool, Object);

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class constructor
static void MemoryPool_constructor(MemoryPool this);

// clear all dynamic memory
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
}

// class's destructor
void MemoryPool_destructor(MemoryPool this)
{
	ASSERT(this, "MemoryPool::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// allocate memory for data
void* MemoryPool_allocate(MemoryPool this, int numBytes)
{
	ASSERT(this, "MemoryPool::allocate: null this");

	int i = 0;
	int blockSize = __MIN_BLOCK;
	int numberOfOjects = 0;
	int pool = 0;
	int displacement = 0;
	int displacementStep = 0;

	// the first 4 bytes are for memory block usage flag
	numBytes += __MEMORY_ALIGNMENT;

	// seach for the shortest pool which can hold the data
	for (pool = __MEMORY_POOLS; pool-- && numBytes > this->poolSizes[pool][eBlockSize];);

	ASSERT(pool >= 0, "MemoryPool::allocate: object size overflow");

	// pool found
	blockSize = this->poolSizes[pool][eBlockSize];

	// get the number of allocable objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	// how much must displace on each iteration
	displacementStep = this->poolSizes[pool][eBlockSize];

	// look for a free block
	for (i = 0, displacement = 0;
	    i < numberOfOjects && this->poolLocation[pool][displacement];
	    i++, displacement += displacementStep);

	if (i >= numberOfOjects)
	{
		Printing_clear(Printing_getInstance());
		MemoryPool_printMemUsage(this, 1, 8);
		NM_ASSERT(false, "MemoryPool::allocate: pool exhausted");
	}

	// assign address to allocated object
	this->poolLocation[pool][displacement] = 0xFF;

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

	if (!object)
	{
		return;
	}

	// look for the pool containing the object
	for (pool = 0; object >= &this->poolLocation[pool][0 + __MEMORY_ALIGNMENT] && pool < __MEMORY_POOLS; pool++);

	// look for the registry in which the object is
	ASSERT(pool <= __MEMORY_POOLS , "MemoryPool::free: deleting something not allocated");

	// move one pool back since the above loop passed the target by one
	pool--;

	// get the total objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	// search for the pool in which it is allocated
	for (i = 0, displacement = 0; i < numberOfOjects; i++, displacement += this->poolSizes[pool][eBlockSize])
	{
		// if the object has been found
		if (object == &this->poolLocation[pool][displacement + __MEMORY_ALIGNMENT])
		{
			// free the block
			this->poolLocation[pool][displacement] = 0x00;

			return;
		}
	}

	// thrown exception
	ASSERT(false, "MemoryPool::free: deleting something not allocated");

#endif

	object[-__MEMORY_ALIGNMENT] = 0x00;
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
	for (pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		//memset(this->poolSizes[pool], 0 , sizeof(this->poolSizes[pool]));
		for (i = 0; i < this->poolSizes[pool][ePoolSize]; i++)
		{
			this->poolLocation[pool][i] = 0x00;
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
	for (pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		size += this->poolSizes[pool][ePoolSize];
	}

	return size;
}

// print dynamic memory usage
void MemoryPool_printMemUsage(MemoryPool this, int x, int y)
{
	ASSERT(this, "MemoryPool::printMemUsage: null this");

	int i;
	int counter = 0;
	int total = 0;
	int pool;
	int displacement = 0;

	Printing_text(Printing_getInstance(), "MEMORY STATUS", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Pool size: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), MemoryPool_getPoolSize(MemoryPool_getInstance()), x + 11, y++, NULL);

	Printing_text(Printing_getInstance(), "Pool", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "Free", x + 7, y, NULL);
	Printing_text(Printing_getInstance(), "Used", x + 14, y++, NULL);

	for (pool = 0; pool < __MEMORY_POOLS; pool++)
	{
		for (displacement = 0, i = 0, counter = 0 ; i < this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize]; i++, displacement += this->poolSizes[pool][eBlockSize])
		{
			if (this->poolLocation[pool][displacement])
			{
				counter++;
			}
		}

		total += counter * this->poolSizes[pool][eBlockSize];

		Printing_text(Printing_getInstance(), Utilities_itoa(this->poolSizes[pool][eBlockSize],10,0) ,  x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize] - counter, x +7, y, NULL);
		Printing_text(Printing_getInstance(), "      ", x + 14, y, NULL);
		Printing_int(Printing_getInstance(), counter, x + 14, y, NULL);
		counter = 0 ;
	}
}