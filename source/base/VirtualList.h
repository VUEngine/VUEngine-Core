/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef VIRTUAL_LIST_H_
#define VIRTUAL_LIST_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualNode.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define VirtualList_METHODS(ClassName)																				\
	Object_METHODS(ClassName);																						\

#define VirtualList_SET_VTABLE(ClassName)																\
	Object_SET_VTABLE(ClassName);																		\

#define VirtualList_ATTRIBUTES																			\
        /* it is derived from */																		\
        Object_ATTRIBUTES																				\
        /* a pointer to the head of the list */ 														\
        VirtualNode head;																				\
        /* a pointer to the tail of the list */															\
        VirtualNode tail;																				\

__CLASS(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(VirtualList);

void VirtualList_destructor(VirtualList this);
void VirtualList_clear(VirtualList this);
int VirtualList_pushFront(VirtualList this, const void* const data);
int VirtualList_pushBack(VirtualList this, const void* const data);
void VirtualList_popFront(VirtualList this);
int VirtualList_getSize(VirtualList this);
void* VirtualList_getNodeData(VirtualList this, int item);
VirtualNode VirtualList_getNode(VirtualList this, int item);
void* VirtualList_getObject(VirtualList this, void* const dataPointer);
VirtualNode VirtualList_find(VirtualList this, const void* const dataPointer);
int VirtualList_getDataPosition(VirtualList this, const void* const dataPointer);
int VirtualList_getNodePosition(VirtualList this, VirtualNode node);
int VirtualList_removeElement(VirtualList this, const void* const dataPointer);
void VirtualList_copy(VirtualList this, VirtualList sourceList);
VirtualNode VirtualList_begin(VirtualList this);
void* const VirtualList_front(VirtualList this);
VirtualNode VirtualList_end(VirtualList this);
void* const VirtualList_back(VirtualList this);
VirtualNode VirtualList_insertAfter(VirtualList this, VirtualNode node, const void* const data);
VirtualNode VirtualList_insertBefore(VirtualList this, VirtualNode node, const void* const data);
void VirtualList_swap(VirtualList this, VirtualList secondList);
void* VirtualList_getObjectAtPosition(VirtualList this, int position);


#endif
