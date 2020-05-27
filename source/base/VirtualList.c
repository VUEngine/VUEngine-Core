/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


//---------------------------------------------------------------------------------------------------------
//											CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

// define a limit to prevent, and detect looped lists
#define LIST_MAX_SIZE 1000


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void VirtualList::constructor()
{
	Base::constructor();

	// set members' default values
	this->head = NULL;
	this->tail = NULL;
}

/**
 * Class destructor
 */
void VirtualList::destructor()
{
	// make sure we remove all nodes
	VirtualList::clear(this);

	// destroy super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Remove all nodes from the list
 */
void VirtualList::clear()
{
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
			delete node;

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
 * @param data
 */
int VirtualList::pushFront(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

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
 * @return			Removed element
 */
void* VirtualList::popFront()
{
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
		delete node;

		return data;
	}

	return NULL;
}

/**
 * Remove the last element from the list
 *
 * @return			Removed element
 */
void* VirtualList::popBack()
{
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
		delete node;

		return data;
	}

	return NULL;
}

/**
 * Add a new node to the end of the list
 *
 * @param data
 */
int VirtualList::pushBack(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

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

		// link new node to the tail
		newNode->previous = this->tail;

		// move the tail
		this->tail = newNode;
	}

	return true;
}

/**
 * Retrieve the number of objects the list has
 *
 * @return				Number of objects
 */
int VirtualList::getSize()
{
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
 * @param item
 * @return				Data pointer of object in the given index node
 */
void* VirtualList::getNodeData(int item)
{
	// get the node
	VirtualNode node = VirtualList::getNode(this, item);

	// return the data
	return (node) ? node->data : NULL;
}

/**
 * Get node at item position
 *
 * @param item		Numeric position of node
 * @return				Node
 */
VirtualNode VirtualList::getNode(int item)
{
	int counter = 0;

	int listSize = VirtualList::getSize(this);

	VirtualNode node = this->head;

	// if not null head
	if(node)
	{
		// if item hasn't reached list's size
		if(item < listSize)
		{
			// increase counter while node hasn't reached list's end
			// and counter hasn't reached the item requested
			while(node && counter < item)
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
 * @param dataPointer
 * @return								Node
 */
void* VirtualList::getObject(void* const dataPointer)
{
	// check if data pointer is valid
	if(!dataPointer)
	{
		return NULL;
	}

	VirtualNode node = this->head;

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
 * @param node		Node to be removed from list
 */
bool VirtualList::doRemoveNode(VirtualNode node)
{
	if(NULL == node)
	{
		return false;
	}

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
	delete node;

	return true;
}

/**
 * Remove a node
 *
 * @param node		Node to be removed from list
 * @return				Flag whether action was successful or not
 */
bool VirtualList::removeNode(VirtualNode node)
{
	// if node isn't null
	if(VirtualList::checkThatNodeIsPresent(this, node))
	{
		return VirtualList::doRemoveNode(this, node);
	}

	return false;
}

/**
 * Find a node in the list
 *
 * @param dataPointer
 * @return								Node
 */
VirtualNode VirtualList::find(const void* const dataPointer)
{
	VirtualNode node = this->head;

	for(; node && node->data != (void*)dataPointer; node = node->next);

	return node;
}

/**
 * Get position of data in the list
 *
 * @param dataPointer
 * @return								Numeric position of node, or -1 when node could not be found
 */
int VirtualList::getDataPosition(const void* const dataPointer)
{
	VirtualNode node = this->head;
		int position = 0;

	for(; node && node->data != (void*)dataPointer; node = node->next, position++);

	return !node ? -1 : position;
}

/**
 * Get position of node in the list
 *
 * @param node
 * @return				Numeric position of node
 */
int VirtualList::getNodePosition(VirtualNode node)
{
	VirtualNode currentNode = this->head;
		int position = 0;

	for(; node && currentNode && currentNode != node; currentNode = currentNode->next, position++);

	return !node || !currentNode ? -1 : position;
}

/**
 * Remove a node from the list
 *
 * @param dataPointer
 * @return					Flag whether action was successful or not
 */
bool VirtualList::removeElement(const void* const dataPointer)
{
	return VirtualList::doRemoveNode(this, VirtualList::find(this, dataPointer));
}

/**
 * Copy source list's elements to destination list
 *
 * @param sourceList
 */
void VirtualList::copy(VirtualList sourceList)
{
#ifdef __DEBUG
	int counter = 0;
#endif

	VirtualNode node = sourceList->head;

	VirtualList::clear(this);

	while(node)
	{
		// add next node
		VirtualList::pushBack(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::copy: endless list copying");
	}
}

/**
 * Add source list's elements to destination list
 *
 * @param sourceList
 */
void VirtualList::add(VirtualList sourceList)
{
#ifdef __DEBUG
	int counter = 0;
#endif

	VirtualNode node = sourceList->head;

	while(node)
	{
		// add next node
		VirtualList::pushBack(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::copy: endless list copying");
	}
}

/**
 * Remove source list's elements from list
 *
 * @param sourceList
 */
void VirtualList::substract(VirtualList sourceList)
{
#ifdef __DEBUG
	int counter = 0;
#endif

	VirtualNode node = sourceList->head;

	while(node)
	{
		// add next node
		VirtualList::removeElement(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::remove: endless list removing");
	}
}

/**
 * Retrieve list's head's address
 *
 * @return			Node
 */
VirtualNode VirtualList::begin()
{
	return this->head;
}

/**
 * Retrieve the first element
 *
 * @return		Head data
 */
void* VirtualList::front()
{
	return this->head ? this->head->data : NULL;
}

/**
 * Retrieve list's last node
 *
 * @return		Node
 */
VirtualNode VirtualList::end()
{
	return this->tail;
}

/**
 * Retrieve the last element
 *
 * @return		Tail data
 */
void* VirtualList::back()
{
	return this->tail ? this->tail->data : NULL;
}

/**
 * Check if a node is part of this list
 *
 * @private
 * @param node	node to check
 */
bool VirtualList::checkThatNodeIsPresent(VirtualNode node)
{
	if(!node)
	{
		return false;
	}

	VirtualNode auxNode = this->head;

	for(; auxNode; auxNode = auxNode->next)
	{
		if(auxNode == node)
		{
			return true;
		}
	}

	return false;
}

/**
 * Insert a node after the node specified
 *
 * @param node	Insert after this node
 * @param data	Data for new node
 * @return		Newly inserted Node
 */
VirtualNode VirtualList::insertAfter(VirtualNode node, const void* const data)
{
	if(!VirtualList::checkThatNodeIsPresent(this, node))
	{
		return NULL;
	}

	VirtualNode newNode = NULL;

	if(!node || node == this->tail)
	{
		VirtualList::pushBack(this, data);

		newNode = this->tail;
	}
	else
	{
		newNode = new VirtualNode(data);

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
 * @param node	Insert before this node
 * @param data	Data for new node
 * @return		Newly inserted Node
 */
VirtualNode VirtualList::insertBefore(VirtualNode node, const void* const data)
{
	if(!VirtualList::checkThatNodeIsPresent(this, node))
	{
		return NULL;
	}

	VirtualNode newNode = NULL;

	if(!node || node == this->head)
	{
		VirtualList::pushFront(this, data);

		newNode = this->head;
	}
	else
	{
		newNode = new VirtualNode(data);

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
 * @param secondList	Function scope
 */
void VirtualList::swap(VirtualList secondList)
{
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
 * @param position	Function scope
 * @return			Node data or NULL if no node could be found at position
 */
void* VirtualList::getObjectAtPosition(int position)
{
	VirtualNode node = this->head;

	int counter = 0;

	if(position < 0)
	{
		return NULL;
	}

	// locate node
	for(; node && counter < position; node = node->next, counter++);

	if(node)
	{
		return node->data;
	}

	return NULL;
}
