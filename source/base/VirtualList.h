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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef VIRTUALNODE_H_
#define VIRTUALNODE_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define VirtualNode_METHODS							\
		Object_METHODS								\


#define VirtualNode_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)						\


__CLASS(VirtualNode);
	


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_NEW_DECLARE(VirtualNode, __PARAMETERS(const void* const data));

//retrieve data pointer
void* VirtualNode_getData(VirtualNode this);

//get next node's address
VirtualNode VirtualNode_getNext(VirtualNode this);
//get next node's address
/*
inline extern  VirtualNode VirtualNode_getNext(VirtualNode this){
	VirtualNode* pointer = this; 
	return pointer[0];
}
*/

//set node's previous node's address
VirtualNode VirtualNode_getPrevious(VirtualNode this);

// swap the data between two nodes
void VirtualNode_swapData(VirtualNode this, VirtualNode node);


#endif /*VIRTUALNODE_H_*/





#ifndef VIRTUALLIST_H_
#define VIRTUALLIST_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define VirtualList_METHODS					\
	Object_METHODS;						

#define VirtualList_SET_VTABLE(ClassName)				\
	Object_SET_VTABLE(ClassName);						

// A texture which has the logic to be allocated in graphic memory
__CLASS(VirtualList);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(VirtualList);

// class's destructor
void VirtualList_destructor(VirtualList this);

// remove all nodes from the list
void VirtualList_clear(VirtualList this);

// add a new node to the begging of the list
int VirtualList_pushFront(VirtualList this, const void* const data);

int VirtualList_pushBack(VirtualList this, const void* const data);

// remove the first element in the list
void VirtualList_popFront(VirtualList this);

// retrieve the number of objects the list has
int VirtualList_getSize(VirtualList this);

// return data pointer of object in the given index node
void* VirtualList_getNodeData(VirtualList this, int item);

// get node at item position
VirtualNode VirtualList_getNode(VirtualList this, int item);

// get node's address of node containing datapointer
void* VirtualList_getObject(VirtualList this, void* const dataPointer);

// find a node in the list
VirtualNode VirtualList_find(VirtualList this, const void* const dataPointer);

// remove an element from the list
int VirtualList_removeElement(VirtualList this, const void* const dataPointer);

// copy source list's elements to destiny list
void VirtualList_copy(VirtualList this, VirtualList sourceList);

// retrieve list's head's address
VirtualNode VirtualList_begin(VirtualList this);

// retrieve the first element
void* const VirtualList_front(VirtualList this);

// retrieve last list's node
VirtualNode VirtualList_end(VirtualList this);

// retrieve the last element
void* const VirtualList_back(VirtualList this);


// insert a node after the node specified
VirtualNode VirtualList_insertAfter(VirtualList this, VirtualNode node, const void* const data);

// insert a node before the node specified
VirtualNode VirtualList_insertBefore(VirtualList this, VirtualNode node, const void* const data);

// swap two lists heads and tails
void VirtualList_swap(VirtualList this, VirtualList secondList);

// get node's address at given position
void* VirtualList_getObjectAtPosition(VirtualList this, int position);

#endif /*VIRTUALLIST_H_*/
