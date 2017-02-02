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

#ifndef VIRTUAL_LIST_H_
#define VIRTUAL_LIST_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualNode.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define VirtualList_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

#define VirtualList_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

#define VirtualList_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\
		/**
		 * @var VirtualNode head
		 * @brief			A pointer to the head of the list
		 * @memberof		VirtualList
		 */																								\
		VirtualNode head;																				\
		/**
		 * @var VirtualNode tail
		 * @brief			A pointer to the tail of the list
		 * @memberof		VirtualList
		 */ 																							\
		VirtualNode tail;																				\

__CLASS(VirtualList);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(VirtualList);

void VirtualList_destructor(VirtualList this);

void* VirtualList_back(VirtualList this);
VirtualNode VirtualList_begin(VirtualList this);
void VirtualList_clear(VirtualList this);
void VirtualList_copy(VirtualList this, VirtualList sourceList);
VirtualNode VirtualList_end(VirtualList this);
VirtualNode VirtualList_find(VirtualList this, const void* const dataPointer);
void* VirtualList_front(VirtualList this);
int VirtualList_getDataPosition(VirtualList this, const void* const dataPointer);
VirtualNode VirtualList_getNode(VirtualList this, int item);
void* VirtualList_getNodeData(VirtualList this, int item);
int VirtualList_getNodePosition(VirtualList this, VirtualNode node);
void* VirtualList_getObject(VirtualList this, void* const dataPointer);
void* VirtualList_getObjectAtPosition(VirtualList this, int position);
int VirtualList_getSize(VirtualList this);
VirtualNode VirtualList_insertAfter(VirtualList this, VirtualNode node, const void* const data);
VirtualNode VirtualList_insertBefore(VirtualList this, VirtualNode node, const void* const data);
void VirtualList_popFront(VirtualList this);
int VirtualList_pushBack(VirtualList this, const void* const data);
int VirtualList_pushFront(VirtualList this, const void* const data);
bool VirtualList_removeElement(VirtualList this, const void* const dataPointer);
void VirtualList_swap(VirtualList this, VirtualList secondList);


#endif
