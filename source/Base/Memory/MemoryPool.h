/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// MemoryPool's defines
#define __BLOCK_DEFINITION(BlockSize, Elements)															\
	BYTE pool ## BlockSize ## B[BlockSize * Elements]; 													\

#define __SET_MEMORY_POOL_ARRAY(BlockSize)																\
	this->poolLocation[pool] = &this->pool ## BlockSize ## B[0];										\
	this->poolSizes[pool][ePoolSize] = sizeof(this->pool ## BlockSize ## B);							\
	this->poolSizes[pool][eBlockSize] = BlockSize;														\
	this->poolLastFreeBlock[pool] = &this->poolLocation[pool][0];										\
	pool++;																								\

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum MemoryPoolSizes
{
	ePoolSize = 0,
	eBlockSize,
	eLastFreeBlock,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class MemoryPool
///
/// Inherits from Object
///
/// Implements a memory pool for dynamic object allocation.
singleton class MemoryPool : Object
{
	/// @protectedsection

	/// The actual blocks that constitute the memory pools
	__MEMORY_POOL_ARRAYS

	/// Array of pointers to the beginning of each memory pool
	BYTE* poolLocation[__MEMORY_POOLS];
	
	/// Mapping of memory pools to their block and total size
	uint16 poolSizes[__MEMORY_POOLS][2];
	
	/// Array of pointers to the last free block found in each pool to accelerate the next search
	BYTE* poolLastFreeBlock[__MEMORY_POOLS];

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return MemoryPool singleton
	static MemoryPool getInstance();

	/// Allocate a block big enough to hold the provided amount of bytes.
	/// @param numBytes: Total number of bytes to allocate
	/// @return A pointer to the allocated block
	static BYTE* allocate(int32 numBytes);

	/// Free the memory block in which the provided object was allocated.
	/// @param object: Pointer to the memory block to free
	static void free(BYTE* object);
	
	/// Print a resume of the memory usage.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printResumedUsage(int32 x, int32 y);

	/// Print all the details of the memory usage.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printDetailedUsage(int32 x, int32 y);
}

#endif
