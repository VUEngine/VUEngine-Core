/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
	this->poolSizes[pool++][eLastFreeBlockIndex] = 0;													\



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class MemoryPool : Object
{
	/* dynamic memory area */
	/* must always put together the pools! */
	/* first byte is used as a usage flag */
	__MEMORY_POOL_ARRAYS
	/* pointer to the beginning of each memory pool */
	BYTE* poolLocation[__MEMORY_POOLS];
	/* pool's size and pool's block size */
	u16 poolSizes[__MEMORY_POOLS][3];

	static MemoryPool getInstance();
	void cleanUp();
	BYTE* allocate(int numBytes);
	void free(BYTE* object);
	void printDirectory(int x, int y, int pool);
	void printDetailedUsage(int x, int y);
	void printResumedUsage(int x, int y);
}


#endif
