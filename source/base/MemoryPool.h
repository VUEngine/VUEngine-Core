/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

// MemoryPool's defines
#define __BLOCK_DEFINITION(BlockSize, Elements)															\
	BYTE pool ## BlockSize ## B[BlockSize * Elements]; 													\

#define __SET_MEMORY_POOL_ARRAY(BlockSize)																\
	this->poolLocation[pool] = &this->pool ## BlockSize ## B[0];										\
	this->poolSizes[pool][ePoolSize] = sizeof(this->pool ## BlockSize ## B);							\
	this->poolSizes[pool][eBlockSize] = BlockSize;														\
	this->poolLastFreeBlock[pool] = &this->poolLocation[pool][0];										\
	pool++;																								\


//---------------------------------------------------------------------------------------------------------
//											ENUMS
//---------------------------------------------------------------------------------------------------------

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize,
	eLastFreeBlock,
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
singleton class MemoryPool : Object
{
	// dynamic memory area
	// must always put together the pools!
	// first byte is used as a usage flag
	__MEMORY_POOL_ARRAYS;
	// pointer to the beginning of each memory pool
	BYTE* poolLocation[__MEMORY_POOLS];
	// pool's size and pool's block size
	uint16 poolSizes[__MEMORY_POOLS][2];
	// pool's size and pool's block size
	BYTE* poolLastFreeBlock[__MEMORY_POOLS];

	/// @publicsection
	static MemoryPool getInstance();
	void cleanUp();
	BYTE* allocate(int32 numBytes);
	void free(BYTE* object);
	void printDirectory(int32 x, int32 y, int32 pool);
	void printDetailedUsage(int32 x, int32 y);
	void printResumedUsage(int32 x, int32 y);
}


#endif
