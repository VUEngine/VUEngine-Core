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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <VirtualList.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void VirtualNode_constructor(VirtualNode this, const void* const data);


// class's destructor
static void VirtualNode_destructor(VirtualNode this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define VirtualNode_ATTRIBUTES				\
											\
	/* super's attributes */				\
	Object_ATTRIBUTES;						\
											\
	/* pointer to next node */				\
	VirtualNode next;						\
											\
	/* pointer to previos node */			\
	VirtualNode previous;					\
											\
	/* pointer to the data */				\
	void* data;


__CLASS_DEFINITION(VirtualNode);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_NEW_DEFINITION(VirtualNode, __PARAMETERS(const void* const data))
__CLASS_NEW_END(VirtualNode, __ARGUMENTS(data));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void VirtualNode_constructor(VirtualNode this, const void* const data){
	
	__CONSTRUCT_BASE(Object);

	// initialize members
	this->next = NULL;
	this->previous = NULL;	
	this->data = (void*)data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
static void VirtualNode_destructor(VirtualNode this){

	ASSERT(this, "VirtualNode::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set node's data pointer
void VirtualNode_setData(VirtualNode this, const void* const data){
	
	ASSERT(this, "VirtualNode::destructor: null this");

	this->data = (void*)data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve data pointer
void* VirtualNode_getData(VirtualNode this){
	
	ASSERT(this, "VirtualNode::getData: null this");

	return this->data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get next node's address
VirtualNode VirtualNode_getNext(VirtualNode this){
	
	ASSERT(this, "VirtualNode::getNext: null this");

	return this->next;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set node's next node's address
void VirtualNode_setNext(VirtualNode this, VirtualNode next){
	
	ASSERT(this, "VirtualNode::setNext: null this");

	this->next = next;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set node's previous node's address
VirtualNode VirtualNode_getPrevious(VirtualNode this){
	
	ASSERT(this, "VirtualNode::getPrevious: null this");

	return this->previous;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set node's previous node's address
void VirtualNode_setPrevious(VirtualNode this, VirtualNode previous){
	
	ASSERT(this, "VirtualNode::setPrevious: null this");

	this->previous = previous;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// swap the data between two nodes
void VirtualNode_swapData(VirtualNode this, VirtualNode node){
	
	ASSERT(this, "VirtualNode::swapData: null this");

	// check that both nodes are valid and are not the same
	if(!(this && node && (this != node))){
		
		return;
	}
	
	void* auxData = this->data;
	this->data = node->data;
	node->data = auxData;
}

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define a limit to prevent, and detect looped lists
#define LISTMAXSIZE 1000


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */
#define VirtualList_ATTRIBUTES					\
												\
	/* it is derivated from*/					\
	Object_ATTRIBUTES							\
												\
	/* a pointer to the head of the list */ 	\
	VirtualNode head;							\
												\
	/* a pointer to the tail of the list */		\
	VirtualNode tail; 



__CLASS_DEFINITION(VirtualList);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void VirtualList_constructor(VirtualList this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_NEW_DEFINITION(VirtualList)
__CLASS_NEW_END(VirtualList);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void VirtualList_constructor(VirtualList this){

	__CONSTRUCT_BASE(Object);	

	// set members' default values
	this->head = NULL;		
	this->tail  = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void VirtualList_destructor(VirtualList this){

	ASSERT(this, "VirtualList::destructor: null this");

	// make sure we remove all nodes
	VirtualList_clear(this);

	// destroy super object
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove all nodes from the list
void VirtualList_clear(VirtualList this){

	ASSERT(this, "VirtualList::clear: null this");

	if(this->head){
		
		// point to the head
		VirtualNode node = this->head;
		
		// move the head to next node
		this->head = this->head->next;
		
		// while there are nodes
		while(node){
			
			// call destructor
			__DELETE(node);
			
			//move the node to the head
			node = this->head;
			
			//move the head
			if(this->head){

				this->head = this->head->next;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add a new node to the begging of the list
int VirtualList_pushFront(VirtualList this, const void* const data){
	
	ASSERT(this, "VirtualList::pushFront: null this");

	VirtualNode newNode = __NEW(VirtualNode, __ARGUMENTS(data));

	//set previous if list isn't empty
	if(this->head){
		
		this->head->previous = newNode;
	}
	
	// assign the node to the head of the list	
	newNode->next = this->head;
	
	//move the head
	this->head = newNode;

	// set the tail
	if(!this->tail){
		
		this->tail = this->head;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove the first element in the list
void VirtualList_popFront(VirtualList this){

	ASSERT(this, "VirtualList::popFront: null this");

	//if head isn't null
	if(this->head){

		VirtualNode node = this->head;
		
		if(node->next){
			
			this->head = node->next;
			
			//move head's previous pointer
			this->head->previous=NULL;
		}
		else{

			//set head
			this->head = NULL;
		}			

		//free dynamic memory
		__DELETE(node);		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add a new node to the end of the list
int VirtualList_pushBack(VirtualList this, const void* const data){

	ASSERT(this, "VirtualList::pushBack: null this");

	VirtualNode newNode = __NEW(VirtualNode, __ARGUMENTS(data));
	
	ASSERT(data, "VirtualList::pushBack: null data");
		
	// set the tail
	if(!this->head){
		
		this->head = this->tail = newNode;
	}
	else{

		// set previous if list isn't empty
		if(this->tail){
			
			// 
			this->tail->next = newNode;
		}
		
		// assign the node to the head of the list	
		newNode->previous = this->tail;
		
		//move the tail
		this->tail = newNode;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve the number of objects the list has
int VirtualList_getSize(VirtualList this){

	ASSERT(this, "VirtualList::getSize: null this");

	int counter = 0;
	
	VirtualNode node = this->head;
	
	while(node){
		
		//load next node
		node = node->next;
		
		++counter;
		
		//increment counter	
		ASSERT(counter < LISTMAXSIZE, "VirtualList::getSize: endless list getting size");
		
	}
	
	return counter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return data pointer of object in the given index node
void* VirtualList_getNodeData(VirtualList this, int item){
	
	ASSERT(this, "VirtualList::getNodeData: null this");

	// get the node
	VirtualNode node = VirtualList_getNode(this, item);
	
	// return the data
	return (node)? node->data: NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get node at item position
VirtualNode VirtualList_getNode(VirtualList this, int item){
	
	ASSERT(this, "VirtualList::getNode: null this");

	int counter=0;
	//get list's size
	
	int listSize = VirtualList_getSize(this);
	//load head
	
	VirtualNode node = this->head;
	
	//if not null head
	if(node){
		
		//if item hasn't reached list's size
		if(item < listSize){
			
			//increase counter while node hasn't reached list's end
			//and counter hasn't reached the item requested
			while((node) && (counter < item)){
				
				//increase counter
				counter++;
				
				//load next node
				node = node->next;
				
				//if item reached
				if(counter == item){
					
					//return node's data
					return node;
				}
			}
			//if item reached
			if(counter == item){
				
				//return node's data
				return node;
			}
			return NULL;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get node's address of node containing datapointer
void* VirtualList_getObject(VirtualList this, void* const dataPointer){
	
	ASSERT(this, "VirtualList::getObject: null this");

	VirtualNode node = this->head;
	
	// check if data pointer is valid
	if(!dataPointer){
		
		return NULL;
	}
	
	//locate node
	while(node && node->data != dataPointer){
		
		node = node->next;
	}
	
	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a node
static int VirtualList_removeNode(VirtualList this, VirtualNode node){

	ASSERT(this, "VirtualList::removeNode: null this");

	//if node isn't null
	if(node){

		// if the node is the head of the list
		if(node == this->head){
			
			if(node->next){

				// move head to next element
				this->head = node->next;
				
				//move head's previous pointer
				this->head->previous = NULL;
			}
			else{

				//set head
				this->head = this->tail = NULL;
			}			
		}
		else{
			
			//if node isn't the last in the list
			if(node == this->tail){
				
				// set the tail
				this->tail = this->tail->previous;
				
				this->tail->next = NULL;
			}
			else{

				//join the previous and next nodes
				node->previous->next = node->next;
				
				node->next->previous = node->previous;
			}
		}		

		//free dynamic memory
		__DELETE(node);		

		return true;		
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// find a node in the list
VirtualNode VirtualList_find(VirtualList this, const void* const dataPointer){
	
	ASSERT(this, "VirtualList::removeElement: null this");

	VirtualNode node = this->head;
	
	CACHE_ENABLE;
	for(; node && VirtualNode_getData(node) !=  (void*)dataPointer; node = VirtualNode_getNext(node));
	CACHE_DISABLE;
	
	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a node from the list
int VirtualList_removeElement(VirtualList this, const void* const dataPointer){
	
	ASSERT(this, "VirtualList::removeElement: null this");

	return VirtualList_removeNode(this, VirtualList_find(this, dataPointer));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// copy source list's elements to destiny list
void VirtualList_copy(VirtualList this, VirtualList sourceList){
	
	ASSERT(this, "VirtualList::copy: null this");
	
#ifdef __DEBUG
	int counter = 0;
#endif
	
	VirtualNode node = sourceList->head;
	
	VirtualList_clear(this);
	
	while(node){
		
		//add next node 
		VirtualList_pushBack(this, node->data);
		//move to next node
		node = node->next;
		
		ASSERT(++counter < LISTMAXSIZE, "VirtualList::copy: endless list copying");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve list's head's address
VirtualNode VirtualList_begin(VirtualList this){
	
	ASSERT(this, "VirtualList::begin: null this");

	return this->head;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve the first element
void* const VirtualList_front(VirtualList this){
	
	ASSERT(this, "VirtualList::front: null this");

	return this->head->data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve last list's node
VirtualNode VirtualList_end(VirtualList this){

	ASSERT(this, "VirtualList::end: null this");

	return this->tail;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve the last element
void* const VirtualList_back(VirtualList this){
	
	ASSERT(this, "VirtualList::back: null this");

	return this->tail->data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// insert a node after the node specified
VirtualNode VirtualList_insertAfter(VirtualList this, VirtualNode node, const void* const data){

	ASSERT(this, "VirtualList::insertAfter: null this");

	VirtualNode newNode = NULL;
	
	if(node == this->head){
		
		VirtualList_pushBack(this, data);
		
		newNode = VirtualList_end(this);
	}
	else{
		
		newNode = __NEW(VirtualNode, __ARGUMENTS(data));
		
		if(!newNode){
			
			return false;
		}
	
		//set previous if list isn't empty
		newNode->next = node->next;
		
		if(node->next){
			
			node->next->previous = newNode;
		}
		
		node->next = newNode;
		
		newNode->previous = node;		
	}
	
	return newNode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// insert a node before the node specified
VirtualNode VirtualList_insertBefore(VirtualList this, VirtualNode node, const void* const data){
	
	ASSERT(this, "VirtualList::insertBefore: null this");

	VirtualNode newNode = NULL;
	
	if(node == this->head){
		
		VirtualList_pushBack(this, data);
	
		newNode = VirtualList_end(this);		
	}
	else{
		
		newNode = __NEW(VirtualNode, __ARGUMENTS(data));
		
		if(!newNode){
			
			return false;
		}

		//set previous if list isn't empty	
		newNode->next = node;
		newNode->previous = node->previous;
		node->previous->next = newNode;
		node->previous = newNode;
	}
	
	return newNode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// swap two lists heads and tails
void VirtualList_swap(VirtualList this, VirtualList secondList){
	
	ASSERT(this, "VirtualList::swap: null this");

	// swap heads
	VirtualNode aux = this->head;
	
	this->head = secondList->head;
	
	secondList->head = aux;
	
	// swap tails
	aux = this->tail;
		
	this->tail = secondList->tail;
		
	secondList->tail = aux;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get node's address at given position
void* VirtualList_getObjectAtPosition(VirtualList this, int position){
	
	ASSERT(this, "VirtualList::getObjectAtPosition: null this");

	VirtualNode node = this->head;
	
	int counter = 0;
	
	if(position < 0){
		
		return NULL;
	}
	
	//locate node
	for(; node && counter < position; node = node->next, counter++);

	if(counter < VirtualList_getSize(this)){
		
		return node->data;
	}
	
	return NULL;
}