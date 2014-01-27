/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <MemoryPool.h>
#include <Utilities.h>
#include <Types.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/*------------------------------ERROR-MESSAGES------------------------------*/
						   /*--------------------Max lenght-----------------*/
#define HP_MC_ERR 			"Heap: Dynamic Mem depleted (BGMap Ch)"
#define HP_OBJ_ERR 			"Heap: Dynamic Mem depleted (Object Ch)"
#define HP_SC_ERR			"Heap: Dynamic Mem depleted (Scroll)"
#define HP_BG_ERR			"Heap: Dynamic Mem depleted (Backgroud)"
#define HP_TX_ERR			"Heap: Dynamic Mem depleted (TextBox)"
#define HP_MEMDEP_ERR		"Heap: Not enought free dynamic mem"
#define HP_OBJSIZE_ERR		"Heap: object to much big"
#define HP_NOALLOC_ERR		"Heap: Deleting something not allocated"
#define HP_NOBLOCK_ERR		"Heap: Index outside memory"


enum MemoryPoolSizes{
	ePoolSize = 0,
	eBlockSize
};

// it is neccesary for the object to be aligned to 2 multiples
#define __MEMORY_ALIGNEMENT	4

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/*
#define __POOL_512B_SIZE_FINAL 	__POOL_512B_SIZE
#define __POOL_256B_SIZE_FINAL 	__POOL_256B_SIZE
#define __POOL_192B_SIZE_FINAL 	__POOL_192B_SIZE
#define __POOL_128B_SIZE_FINAL 	__POOL_128B_SIZE
#define __POOL_96B_SIZE_FINAL 	__POOL_96B_SIZE
#define __POOL_64B_SIZE_FINAL 	__POOL_64B_SIZE
#define __POOL_48B_SIZE_FINAL 	__POOL_48B_SIZE
#define __POOL_32B_SIZE_FINAL 	__POOL_32B_SIZE
#define __POOL_24B_SIZE_FINAL 	__POOL_24B_SIZE
#define __POOL_16B_SIZE_FINAL 	__POOL_16B_SIZE
*/

#define __POOL_512B_SIZE_FINAL 	(__BLOCK_512B * 0)
#define __POOL_256B_SIZE_FINAL 	(__BLOCK_256B * 0)
#define __POOL_192B_SIZE_FINAL 	(__BLOCK_192B * 8)
#define __POOL_128B_SIZE_FINAL 	(__BLOCK_128B * 8)
#define __POOL_96B_SIZE_FINAL 	(__BLOCK_96B * 64)
#define __POOL_64B_SIZE_FINAL 	(__BLOCK_64B * 128)
#define __POOL_48B_SIZE_FINAL 	(__BLOCK_48B * 64)
#define __POOL_32B_SIZE_FINAL 	(__BLOCK_32B * 128)
#define __POOL_24B_SIZE_FINAL 	(__BLOCK_24B * 128)
#define __POOL_16B_SIZE_FINAL 	(__BLOCK_16B * 0)

#define MemoryPool_ATTRIBUTES							\
														\
	/* super's attributes */							\
	Object_ATTRIBUTES;									\
														\
	/* dynamic memory area */							\
	/* must always put together the pools! */			\
	/* first byte is used as a usage flag */			\
														\
	/*BYTE pool512B[__POOL_512B_SIZE_FINAL];*/			\
	BYTE pool256B[__POOL_256B_SIZE_FINAL]; 				\
	BYTE pool192B[__POOL_192B_SIZE_FINAL]; 				\
	BYTE pool128B[__POOL_128B_SIZE_FINAL];				\
	BYTE pool96B[__POOL_96B_SIZE_FINAL];				\
	BYTE pool64B[__POOL_64B_SIZE_FINAL];				\
	BYTE pool48B[__POOL_48B_SIZE_FINAL];				\
	BYTE pool32B[__POOL_32B_SIZE_FINAL];				\
	BYTE pool24B[__POOL_24B_SIZE_FINAL];				\
	BYTE pool16B[__POOL_16B_SIZE_FINAL];				\
	/* here ends the pool area */						\
														\
	/* pointer to the beggining of each memory pool */	\
	BYTE* poolLocation[__MEMORY_POOLS];					\
														\
	/* pool's size and pool's block size */				\
	int poolSizes[__MEMORY_POOLS][2];
	

__CLASS_DEFINITION(MemoryPool);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class constructor
static void MemoryPool_constructor(MemoryPool this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// a singleton
__SINGLETON(MemoryPool);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class constructor
static void MemoryPool_constructor(MemoryPool this){
	
	__CONSTRUCT_BASE(Object);
	{	
		int pool = 0;
		int i;
		
		// initialize pool's sizes and pointers
		/*
		this->poolLocation[pool] = this->pool512B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool512B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_512B;
		*/	
		
		this->poolLocation[pool] = this->pool256B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool256B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_256B;

		this->poolLocation[pool] = this->pool192B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool192B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_192B;

		this->poolLocation[pool] = this->pool128B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool128B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_128B;
	
		this->poolLocation[pool] = this->pool96B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool96B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_96B;

		this->poolLocation[pool] = this->pool64B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool64B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_64B;

		this->poolLocation[pool] = this->pool48B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool48B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_48B;

		this->poolLocation[pool] = this->pool32B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool32B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_32B;
		
		this->poolLocation[pool] = this->pool24B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool24B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_24B;
		
		this->poolLocation[pool] = this->pool16B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool16B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_16B;
		
	
		/*
		this->poolLocation[pool] = this->pool8B;
		this->poolSizes[pool][ePoolSize] = sizeof(this->pool8B);
		this->poolSizes[pool++][eBlockSize] = __BLOCK_8B;
		*/
		
		// clear all allocable objects usage
		for(pool = 0; pool < __MEMORY_POOLS; pool++){
			
			//memset(this->poolSizes[pool], 0 , sizeof(this->poolSizes[pool]));
			for(i = 0; i < this->poolSizes[pool][ePoolSize]; i++){
				
				this->poolLocation[pool][i] = 0x00;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void MemoryPool_destructor(MemoryPool this){
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate memory for data
void* MemoryPool_allocate(MemoryPool this, int numBytes){
	
	int i = 0;
	int blockSize = __MIN_BLOCK;	
	int numberOfOjects = 0;
	int pool = 0;
	int displacement = 0;
	int displacementStep = 0;
	
	// the first 4 bytes are for memory block usage flag
	numBytes += __MEMORY_ALIGNEMENT;
	
	// seach for the shortest pool which can hold the data
	for(pool = __MEMORY_POOLS; pool-- && numBytes > this->poolSizes[pool][eBlockSize];);

	ASSERT(pool >= 0, "MemoryPool: object size overflow");
	
	// pool found
	blockSize = this->poolSizes[pool][eBlockSize];

	// get the number of allocable objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	// how much must displace on each iteration
	displacementStep = this->poolSizes[pool][eBlockSize];
	
	// look for a free block
	for(i = 0, displacement = 0; 
	    i < numberOfOjects && this->poolLocation[pool][displacement]; 
	    i++, displacement += displacementStep);	
	
#ifdef __DEBUG
	if (i >= numberOfOjects){
	
		MemoryPool_printMemUsage(this, 1, 1);
		ASSERT(false, "MemoryPool: pool exhausted");
	}
#endif
	
	// assign address to allocated object
	this->poolLocation[pool][displacement] = 0xFF;

	// return designed address
	return &this->poolLocation[pool][displacement + __MEMORY_ALIGNEMENT];
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// free memory when an object is no longer used
// remove an object from heap
void MemoryPool_free(MemoryPool this, BYTE* object){

#ifdef __DEBUG	
	
	int i;
	int pool = 0;
	int displacement = 0;
	int numberOfOjects = 0;

	if(!object){
		return;
	}

	// look for the pool containing the object
	for(pool = 0; 
	    object >= &this->poolLocation[pool][0 + __MEMORY_ALIGNEMENT] && pool < __MEMORY_POOLS; 
	    pool++);
	
	// look for the registry in which the object is
	ASSERT(pool <= __MEMORY_POOLS , HP_NOALLOC_ERR);
	
	// move one pool back since the above loop passed the target by one
	pool--;
	
	// get the total objects in the pool
	numberOfOjects = this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize];

	// search for the pool in which it is allocated
	for(i = 0, displacement = 0; i < numberOfOjects; i++, displacement += this->poolSizes[pool][eBlockSize]){

		// if the object has been found		
		if(object == &this->poolLocation[pool][displacement + __MEMORY_ALIGNEMENT]){

			// free the block
			this->poolLocation[pool][displacement] = 0x00;
			
			return;
		}
	}
	
	// thrown exception	
	ASSERT(false, HP_NOALLOC_ERR);
	
#endif

	object[-__MEMORY_ALIGNEMENT] = 0x00;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear all dynamic memory
void MemoryPool_reset(MemoryPool this){
	
	MemoryPool_constructor(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print dynamic memory usage
void MemoryPool_printMemUsage(MemoryPool this, int x, int y){
	
	int i;
	int counter = 0;
	int total = 0;
	int pool;
	int displacement = 0;
	
	Printing_text("MEMORY USAGE",x,y++);
	Printing_text( "POOL",  x,y);
	Printing_text( "FREE",   x + 7,y);
	Printing_text( "USED",  x+ 14,y);

	for(pool = 0; pool < __MEMORY_POOLS; pool++){
		
		for(displacement = 0, i = 0, counter = 0 ; i < this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize]; i++, displacement += this->poolSizes[pool][eBlockSize]){
			
			if(this->poolLocation[pool][displacement]){

				counter++;
			}
		}
		
		total += counter * this->poolSizes[pool][eBlockSize];
		
		Printing_text(Utilities_itoa(this->poolSizes[pool][eBlockSize],10,0) ,  x, ++y);
		Printing_int(this->poolSizes[pool][ePoolSize] / this->poolSizes[pool][eBlockSize] - counter, x +7, y);
		Printing_int(counter, x +14, y);
		counter = 0 ;
	}
}
