/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STRUCT_H_
#define STRUCT_H_


//---------------------------------------------------------------------------------------------------------
//											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											 PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Struct : Object
{
	/// @publicsection
	static inline BYTE* new(int32 numBytes);
	static void destructor();
}

static inline BYTE* Struct::new(int32 numBytes)
{
	extern MemoryPool _memoryPool;
	
	BYTE* memoryBlock = MemoryPool::allocate(_memoryPool, numBytes + STRUCT_VTABLE_POINTER_SIZE + STRUCT_DYNAMIC_FLAG_SIZE);

	*((uint32*)&memoryBlock[0]) = (uint32)&Struct_destructor;
	*((uint32*)&memoryBlock[STRUCT_VTABLE_POINTER_SIZE]) = STRUCT_DYNAMIC_FLAG;
	
	return &memoryBlock[-STRUCT_DYNAMIC_VTABLE_INDEX];
}

#endif
