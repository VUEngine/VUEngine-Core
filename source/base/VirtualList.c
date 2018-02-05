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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VirtualNode.h>
#include <VirtualList.h>
#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

// define a limit to prevent, and detect looped lists
#define LIST_MAX_SIZE 1000


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	VirtualList
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(VirtualList, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VirtualList_constructor(VirtualList this);
static void VirtualList_checkThatNodeIsPresent(VirtualList this, VirtualNode node);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(VirtualList)
__CLASS_NEW_END(VirtualList);

/**
 * Class constructor
 *
 * @memberof		VirtualList
 * @private
 *
 * @param this	Function scope
 */
static void VirtualList_constructor(VirtualList this)
{
	ASSERT(this, "VirtualList::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// set members' default values
	this->head = NULL;
	this->tail = NULL;
}

/**
 * Class destructor
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this	Function scope
 */
void VirtualList_destructor(VirtualList this)
{
	ASSERT(this, "VirtualList::destructor: null this");

	// make sure we remove all nodes
	VirtualList_clear(this);

	// destroy super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Remove all nodes from the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this	Function scope
 */
void VirtualList_clear(VirtualList this)
{
	ASSERT(this, "VirtualList::clear: null this");

	if(this->head)
	{
		// point to the head
		VirtualNode node = this->head;

		// move the head to next node
		this->head = this->head->next;

		// while there are nodes
		while(node)
		{
			// call destructor
			__DELETE(node);

			// move the node to the head
			node = this->head;

			// move the head
			if(this->head)
			{
				this->head = this->head->next;
			}
		}

		ASSERT(!this->head, "VirtualList::clear: head is not NULL");

		this->tail = NULL;
	}
}

/**
 * Add a new node to the beginning of the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param data
 */
int VirtualList_pushFront(VirtualList this, const void* const data)
{
	ASSERT(this, "VirtualList::pushFront: null this");

	VirtualNode newNode = __NEW(VirtualNode, data);

	// set previous if list isn't empty
	if(this->head)
	{
		this->head->previous = newNode;
	}

	// assign the node to the head of the list
	newNode->next = this->head;

	// move the head
	this->head = newNode;

	// set the tail
	if(!this->tail)
	{
		this->tail = this->head;
	}

	return true;
}

/**
 * Remove the first element from the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 *
 * @return			Removed element
 */
void* VirtualList_popFront(VirtualList this)
{
	ASSERT(this, "VirtualList::popFront: null this");

	// if head isn't null
	if(this->head)
	{
		VirtualNode node = this->head;
		void* data = node->data;

		if(node->next)
		{
			this->head = node->next;

			// move head's previous pointer
			this->head->previous = NULL;
		}
		else
		{
			// set head
			this->head = NULL;
			this->tail = NULL;
		}

		// free dynamic memory
		__DELETE(node);

		return data;
	}

	return NULL;
}

/**
 * Remove the last element from the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 *
 * @return			Removed element
 */
void* VirtualList_popBack(VirtualList this)
{
	ASSERT(this, "VirtualList::popBack: null this");

	// if tail isn't null
	if(this->tail)
	{
		VirtualNode node = this->tail;
		void* data = node->data;

		if(node->previous)
		{
			this->tail = node->previous;

			// move head's previous pointer
			this->tail->next = NULL;
		}
		else
		{
			// set tail
			this->tail = NULL;
			this->head = NULL;
		}

		// free dynamic memory
		__DELETE(node);

		return data;
	}

	return NULL;
}

/**
 * Add a new node to the end of the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param data
 */
int VirtualList_pushBack(VirtualList this, const void* const data)
{
	ASSERT(this, "VirtualList::pushBack: null this");

	VirtualNode newNode = __NEW(VirtualNode, data);

	ASSERT(data, "VirtualList::pushBack: null data");

	// set the tail
	if(!this->head)
	{
		this->head = this->tail = newNode;
	}
	else
	{
		// set previous if list isn't empty
		if(this->tail)
		{
			this->tail->next = newNode;
		}

		// assign the node to the head of the list
		newNode->previous = this->tail;

		// move the tail
		this->tail = newNode;
	}

	return true;
}

/**
 * Retrieve the number of objects the list has
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 *
 * @return				Number of objects
 */
int VirtualList_getSize(VirtualList this)
{
	ASSERT(this, "VirtualList::getSize: null this");

	int counter = 0;

	VirtualNode node = this->head;

	while(node)
	{
		// load next node
		node = node->next;

		++counter;

		// increment counter
		ASSERT(counter < LIST_MAX_SIZE, "VirtualList::getSize: endless list getting size");

	}

	return counter;
}

/**
 * Return data pointer of object in the given index node
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param item
 *
 * @return				Data pointer of object in the given index node
 */
void* VirtualList_getNodeData(VirtualList this, int item)
{
	ASSERT(this, "VirtualList::getNodeData: null this");

	// get the node
	VirtualNode node = VirtualList_getNode(this, item);

	// return the data
	return (node) ? node->data : NULL;
}

/**
 * Get node at item position
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param item		Numeric position of node
 *
 * @return				Node
 */
VirtualNode VirtualList_getNode(VirtualList this, int item)
{
	ASSERT(this, "VirtualList::getNode: null this");

	int counter=0;
	// get list's size

	int listSize = VirtualList_getSize(this);
	// load head

	VirtualNode node = this->head;

	// if not null head
	if(node)
	{
		// if item hasn't reached list's size
		if(item < listSize)
		{
			// increase counter while node hasn't reached list's end
			// and counter hasn't reached the item requested
			while((node) && (counter < item))
			{
				// increase counter
				counter++;

				// load next node
				node = node->next;

				// if item reached
				if(counter == item)
				{
					// return node's data
					return node;
				}
			}
			// if item reached
			if(counter == item)
			{
				// return node's data
				return node;
			}
			return NULL;
		}
	}
	return NULL;
}

/**
 * Get address of node containing dataPointer
 *
 * @memberof						VirtualList
 * @public
 *
 * @param this						Function scope
 * @param dataPointer
 *
 * @return								Node
 */
void* VirtualList_getObject(VirtualList this, void* const dataPointer)
{
	ASSERT(this, "VirtualList::getObject: null this");

	VirtualNode node = this->head;

	// check if data pointer is valid
	if(!dataPointer)
	{
		return NULL;
	}

	// locate node
	while(node && node->data != dataPointer)
	{
		node = node->next;
	}

	return node;
}

/**
 * Remove a node
 *
 * @memberof		VirtualList
 * @private
 *
 * @param this		Function scope
 * @param node		Node to be removed from list
 *
 * @return				Flag whether action was successful or not
 */
bool VirtualList_removeNode(VirtualList this, VirtualNode node)
{
	ASSERT(this, "VirtualList::removeNode: null this");

	// if node isn't null
	if(node)
	{
		// if the node is the head of the list
		if(node == this->head)
		{
			if(node->next)
			{
				// move head to next element
				this->head = node->next;

				// move head's previous pointer
				this->head->previous = NULL;
			}
			else
			{
				// set head
				this->head = this->tail = NULL;
			}
		}
		else
		{
			// if node isn't the last in the list
			if(node == this->tail)
			{
				// set the tail
				this->tail = this->tail->previous;

				this->tail->next = NULL;
			}
			else
			{
				// join the previous and next nodes
				node->previous->next = node->next;

				node->next->previous = node->previous;
			}
		}

		// free dynamic memory
		__DELETE(node);

		return true;
	}

	return false;
}

/**
 * Find a node in the list
 *
 * @memberof						VirtualList
 * @public
 *
 * @param this						Function scope
 * @param dataPointer
 *
 * @return								Node
 */
VirtualNode VirtualList_find(VirtualList this, const void* const dataPointer)
{
	ASSERT(this, "VirtualList::find: null this");

	VirtualNode node = this->head;

	for(; node && node->data != (void*)dataPointer; node = node->next);

	return node;
}

/**
 * Get position of data in the list
 *
 * @memberof						VirtualList
 * @public
 *
 * @param this						Function scope
 * @param dataPointer
 *
 * @return								Numeric position of node, or -1 when node could not be found
 */
int VirtualList_getDataPosition(VirtualList this, const void* const dataPointer)
{
	ASSERT(this, "VirtualList::getDataPosition: null this");

	VirtualNode node = this->head;
		int position = 0;

	for(; node && node->data != (void*)dataPointer; node = node->next, position++);

	return !node ? -1 : position;
}

/**
 * Get position of node in the list
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param node
 *
 * @return				Numeric position of node
 */
int VirtualList_getNodePosition(VirtualList this, VirtualNode node)
{
	ASSERT(this, "VirtualList::removeElement: null this");

	VirtualNode currentNode = this->head;
		int position = 0;

	for(; node && currentNode && currentNode != node; currentNode = currentNode->next, position++);

	return !node || !currentNode ? -1 : position;
}

/**
 * Remove a node from the list
 *
 * @memberof						VirtualList
 * @public
 *
 * @param this					Function scope
 * @param dataPointer
 *
 * @return					Flag whether action was successful or not
 */
bool VirtualList_removeElement(VirtualList this, const void* const dataPointer)
{
	ASSERT(this, "VirtualList::removeElement: null this");

	return VirtualList_removeNode(this, VirtualList_find(this, dataPointer));
}

/**
 * Copy source list's elements to destination list
 *
 * @memberof			VirtualList
 * @public
 *
 * @param this				Function scope
 * @param sourceList
 */
void VirtualList_copy(VirtualList this, VirtualList sourceList)
{
	ASSERT(this, "VirtualList::copy: null this");

#ifdef __DEBUG
	int counter = 0;
#endif

	VirtualNode node = sourceList->head;

	VirtualList_clear(this);

	while(node)
	{
		// add next node
		VirtualList_pushBack(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::copy: endless list copying");
	}
}

/**
 * Retrieve list's head's address
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this		Function scope
 *
 * @return			Node
 */
VirtualNode VirtualList_begin(VirtualList this)
{
	ASSERT(this, "VirtualList::begin: null this");

	return this->head;
}

/**
 * Retrieve the first element
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this	Function scope
 *
 * @return		Head data
 */
void* VirtualList_front(VirtualList this)
{
	ASSERT(this, "VirtualList::front: null this");

	return this->head ? this->head->data : NULL;
}

/**
 * Retrieve list's last node
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this	Function scope
 *
 * @return		Node
 */
VirtualNode VirtualList_end(VirtualList this)
{
	ASSERT(this, "VirtualList::end: null this");

	return this->tail;
}

/**
 * Retrieve the last element
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this	Function scope
 *
 * @return		Tail data
 */
void* VirtualList_back(VirtualList this)
{
	ASSERT(this, "VirtualList::back: null this");

	return this->tail ? this->tail->data : NULL;
}

/**
 * Check if a node is part of this list
 *
 * @memberof	VirtualList
 * @private
 *
 * @param this	Function scope
 * @param node	node to check
 */
__attribute__ ((unused)) static void VirtualList_checkThatNodeIsPresent(VirtualList this, VirtualNode node)
{
	ASSERT(this, "VirtualList::checkThatNodeIsPresent: null this");

	if(!node)
	{
		return;
	}

	VirtualNode auxNode = this->head;

	for(; auxNode; auxNode = auxNode->next)
	{
		if(auxNode == node)
		{
			return;
		}
	}

	NM_ASSERT(false, "VirtualList::checkThatNodeIsPresent: node doesn't belong to me");
}

/**
 * Insert a node after the node specified
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this	Function scope
 * @param node	Insert after this node
 * @param data	Data for new node
 *
 * @return		Newly inserted Node
 */
VirtualNode VirtualList_insertAfter(VirtualList this, VirtualNode node, const void* const data)
{
	ASSERT(this, "VirtualList::insertAfter: null this");

#ifdef __DEBUG
	VirtualList_checkThatNodeIsPresent(this, node);
#endif

	VirtualNode newNode = NULL;

	if(!node || node == this->tail)
	{
		VirtualList_pushBack(this, data);

		newNode = this->tail;
	}
	else
	{
		newNode = __NEW(VirtualNode, data);

		if(!newNode)
		{
			return false;
		}

		// set previous if list isn't empty
		newNode->next = node->next;

		if(node->next)
		{
			node->next->previous = newNode;
		}

		node->next = newNode;

		newNode->previous = node;
	}

	return newNode;
}

/**
 * Insert a node before the node specified
 *
 * @memberof	VirtualList
 * @public
 *
 * @param this	Function scope
 * @param node	Insert before this node
 * @param data	Data for new node
 *
 * @return		Newly inserted Node
 */
VirtualNode VirtualList_insertBefore(VirtualList this, VirtualNode node, const void* const data)
{
	ASSERT(this, "VirtualList::insertBefore: null this");

#ifdef __DEBUG
	VirtualList_checkThatNodeIsPresent(this, node);
#endif

	VirtualNode newNode = NULL;

	if(!node || node == this->head)
	{
		VirtualList_pushFront(this, data);

		newNode = this->head;
	}
	else
	{
		newNode = __NEW(VirtualNode, data);

		if(!newNode)
		{
			return false;
		}

		// set previous if list isn't empty
		newNode->next = node;
		newNode->previous = node->previous;
		node->previous->next = newNode;
		node->previous = newNode;
	}

	return newNode;
}

/**
 * Swap two lists' heads and tails
 *
 * @memberof			VirtualList
 * @public
 *
 * @param this			Function scope
 * @param secondList	Function scope
 */
void VirtualList_swap(VirtualList this, VirtualList secondList)
{
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

/**
 * Get node's address at given position
 *
 * @memberof		VirtualList
 * @public
 *
 * @param this		Function scope
 * @param position	Function scope
 *
 * @return			Node data or NULL if no node could be found at position
 */
void* VirtualList_getObjectAtPosition(VirtualList this, int position)
{
	ASSERT(this, "VirtualList::getObjectAtPosition: null this");

	VirtualNode node = this->head;

	int counter = 0;

	if(position < 0)
	{
		return NULL;
	}

	// locate node
	for(; node && counter < position; node = node->next, counter++);

	if(counter < VirtualList_getSize(this))
	{
		return node->data;
	}

	return NULL;
}
