/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	if(NULL != this->head)
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->head; NULL != node;)
		{
			VirtualNode aux = node;

			node = node->next;

			delete aux;
		}

		this->head = this->tail = NULL;

		HardwareManager::resumeInterrupts();
	}
}


/**
 * Delete all nodes' data and clear
 */
void VirtualList::deleteData()
{
	if(NULL != this->head)
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->head; NULL != node;)
		{
			delete node->data;

			VirtualNode aux = node;

			node = node->next;

			delete aux;
		}

		this->head = this->tail = NULL;

		HardwareManager::resumeInterrupts();
	}
}

/**
 * Add a new node to the beginning of the list
 *
 * @param data
 */
int32 VirtualList::pushFront(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	ASSERT(data, "VirtualList::pushFront: null data");

	// set the tail
	if(NULL == this->tail)
	{
		this->tail = this->head = newNode;
	}
	else
	{
		// link new node to the head
		newNode->next = this->head;

		// move the head
		this->head = newNode;

		// set previous if list isn't empty
		if(NULL != this->head->next)
		{
			this->head->next->previous = newNode;
		}
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
	if(NULL != this->head)
	{
		HardwareManager::suspendInterrupts();

		VirtualNode node = this->head;
		void* data = node->data;

		if(NULL != node->next)
		{
			this->head = node->next;
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

		HardwareManager::resumeInterrupts();

		return data;
	}

	HardwareManager::resumeInterrupts();

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
	if(NULL != this->tail)
	{
		HardwareManager::suspendInterrupts();
		VirtualNode node = this->tail;
		void* data = node->data;

		if(NULL != node->previous)
		{
			this->tail = node->previous;
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

		HardwareManager::resumeInterrupts();

		return data;
	}

	HardwareManager::resumeInterrupts();

	return NULL;
}

/**
 * Add a new node to the end of the list
 *
 * @param data
 */
int32 VirtualList::pushBack(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	ASSERT(data, "VirtualList::pushBack: null data");

	// set the tail
	if(NULL == this->head)
	{
		this->head = this->tail = newNode;
	}
	else
	{
		// link new node to the tail
		newNode->previous = this->tail;

		// move the tail
		this->tail = newNode;

		// set previous if list isn't empty
		if(NULL != this->tail->previous)
		{
			this->tail->previous->next = newNode;
		}
	}

	return true;
}

/**
 * Retrieve the number of objects the list has
 *
 * @return				Number of objects
 */
int32 VirtualList::getSize()
{
	int32 counter = 0;

	VirtualNode node = this->head;

	while(NULL != node)
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
void* VirtualList::getNodeData(int32 item)
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
VirtualNode VirtualList::getNode(int32 item)
{
	int32 counter = 0;

	int32 listSize = VirtualList::getSize(this);

	VirtualNode node = this->head;

	// if not null head
	if(NULL != node)
	{
		// if item hasn't reached list's size
		if(item < listSize)
		{
			// increase counter while node hasn't reached list's end
			// and counter hasn't reached the item requested
			while(NULL != node && counter < item)
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
	while(NULL != node && node->data != dataPointer)
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

	HardwareManager::suspendInterrupts();

	// if the node is the head of the list
	if(node == this->head)
	{
		if(NULL != node->next)
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
		// if node is the last in the list
		if(node == this->tail)
		{
			this->tail->previous->next = NULL;

			// set the tail
			this->tail = this->tail->previous;
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

	HardwareManager::resumeInterrupts();

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
#ifndef __ENABLE_PROFILER
#ifndef __RELEASE
	// if node isn't null
	if(VirtualList::checkThatNodeIsPresent(this, node))
	{
		return VirtualList::doRemoveNode(this, node);
	}

	return false;
#else
	return VirtualList::doRemoveNode(this, node);
#endif
#else
	return VirtualList::doRemoveNode(this, node);
#endif
}

/**
 * Move a node before another
 *
 * @param node			Reference node
 * @param nodeToMove			Node to move
 */
void VirtualList::moveBefore(VirtualNode node, VirtualNode nodeToMove)
{
	if(isDeleted(node) || isDeleted(nodeToMove) || node == nodeToMove || node->previous == nodeToMove)
	{
		return;
	}

	if(nodeToMove == this->head)
	{
		this->head = nodeToMove->next;
		this->head->previous = NULL;
	}
	else if(nodeToMove == this->tail)
	{
		this->tail = nodeToMove->previous;
	}

	if(NULL != nodeToMove->previous)
	{
		nodeToMove->previous->next = nodeToMove->next;
	}
	
	if(NULL != nodeToMove->next)
	{
		nodeToMove->next->previous = nodeToMove->previous;
	}

	nodeToMove->next = node;
	nodeToMove->previous = node->previous;

	if(NULL != node->previous)
	{
		node->previous->next = nodeToMove;
	}

	node->previous = nodeToMove;

	if(node == this->head)
	{
		this->head = nodeToMove;
	}

	NM_ASSERT(NULL == this->head->previous, "VirtualList::moveBefore: head is corrupted");
	NM_ASSERT(NULL == this->tail->next, "VirtualList::moveBefore: tail is corrupted");
}

/**
 * Move a node before another
 *
 * @param node			Reference node
 * @param nodeToMove			Node to move
 */
void VirtualList::moveAfter(VirtualNode node, VirtualNode nodeToMove)
{
	if(isDeleted(node) || isDeleted(nodeToMove) || node == nodeToMove)
	{
		return;
	}

	if(nodeToMove == this->head)
	{
		this->head = nodeToMove->next;
		this->head->previous = NULL;
	}
	else if(nodeToMove == this->tail)
	{
		this->tail = nodeToMove->previous;
	}

	if(NULL != nodeToMove->next)
	{
		nodeToMove->next->previous = nodeToMove->previous;
	}
	
	if(NULL != nodeToMove->previous)
	{
		nodeToMove->previous->next = nodeToMove->next;
	}

	nodeToMove->previous = node;
	nodeToMove->next = node->next;

	if(NULL != node->next)
	{
		node->next->previous = nodeToMove;
	}

	node->next = nodeToMove;

	if(node == this->tail)
	{
		this->tail = nodeToMove;
	}

	NM_ASSERT(NULL == this->head->previous, "VirtualList::moveBefore: head is corrupted");
	NM_ASSERT(NULL == this->tail->next, "VirtualList::moveBefore: tail is corrupted");

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
	
	for(; NULL != node && node->data != (void*)dataPointer; node = node->next);

	return node;
}

/**
 * Get position of data in the list
 *
 * @param dataPointer
 * @return								Numeric position of node, or -1 when node could not be found
 */
int32 VirtualList::getDataPosition(const void* const dataPointer)
{
	VirtualNode node = this->head;
	int32 position = 0;

	for(; NULL != node && node->data != (void*)dataPointer; node = node->next, position++);

	return !node ? -1 : position;
}

/**
 * Get position of node in the list
 *
 * @param node
 * @return				Numeric position of node
 */
int32 VirtualList::getNodePosition(VirtualNode node)
{
	VirtualNode currentNode = this->head;
	int32 position = 0;

	for(; NULL != node && currentNode && currentNode != node; currentNode = currentNode->next, position++);

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
	int32 counter = 0;
#endif

	VirtualNode node = sourceList->head;

	VirtualList::clear(this);

	while(NULL != node)
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
	int32 counter = 0;
#endif
	if(isDeleted(sourceList))
	{
		return;
	}

	VirtualNode node = sourceList->head;

	while(NULL != node)
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
	int32 counter = 0;
#endif

	VirtualNode node = sourceList->head;

	while(NULL != node)
	{
		// add next node
		VirtualList::removeElement(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::remove: endless list removing");
	}
}

/**
 * Copy source list's elements in reverse order
 *
 * @param sourceList
 */
void VirtualList::reverse(VirtualList sourceList)
{
#ifdef __DEBUG
	int32 counter = 0;
#endif

	VirtualNode node = sourceList->head;

	VirtualList::clear(this);

	while(NULL != node)
	{
		// add next node
		VirtualList::pushFront(this, node->data);
		// move to next node
		node = node->next;

		ASSERT(++counter < LIST_MAX_SIZE, "VirtualList::copy: endless list copying");
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
	return NULL != this->head ? this->head->data : NULL;
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
	return NULL != this->tail ? this->tail->data : NULL;
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
		NM_ASSERT(false, "VirtualList::checkThatNodeIsPresent: NULL node");
		return false;
	}

	for(VirtualNode auxNode = this->head; NULL != auxNode; auxNode = auxNode->next)
	{
		if(auxNode == node)
		{
			return true;
		}
	}

	NM_ASSERT(false, "VirtualList::checkThatNodeIsPresent: node not found");
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

	HardwareManager::suspendInterrupts();

	VirtualNode newNode = NULL;

	if(NULL == node || node == this->tail)
	{
		VirtualList::pushBack(this, data);

		newNode = this->tail;
	}
	else
	{
		newNode = new VirtualNode(data);

		if(NULL == newNode)
		{
			HardwareManager::resumeInterrupts();

			return false;
		}

		newNode->next = node->next;

		if(node->next)
		{
			node->next->previous = newNode;
		}

		node->next = newNode;

		newNode->previous = node;
	}

	HardwareManager::resumeInterrupts();

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

	HardwareManager::suspendInterrupts();

	VirtualNode newNode = NULL;

	if(NULL == node || node == this->head)
	{
		VirtualList::pushFront(this, data);

		newNode = this->head;
	}
	else
	{
		newNode = new VirtualNode(data);

		if(NULL == newNode)
		{
			HardwareManager::resumeInterrupts();

			return false;
		}

		newNode->previous = node->previous;
		newNode->next = node;

		if(NULL != node->previous)
		{
			node->previous->next = newNode;
		}

		node->previous = newNode;
	}

	HardwareManager::resumeInterrupts();

	return newNode;
}

/**
 * Swap two lists' heads and tails
 *
 * @param secondList	Function scope
 */
void VirtualList::swap(VirtualList secondList)
{
	HardwareManager::suspendInterrupts();

	// swap heads
	VirtualNode aux = this->head;

	this->head = secondList->head;

	secondList->head = aux;

	// swap tails
	aux = this->tail;

	this->tail = secondList->tail;

	secondList->tail = aux;

	HardwareManager::resumeInterrupts();
}

/**
 * Get node's address at given position
 *
 * @param position	Function scope
 * @return			Node data or NULL if no node could be found at position
 */
void* VirtualList::getObjectAtPosition(int32 position)
{
	VirtualNode node = this->head;

	int32 counter = 0;

	if(position < 0)
	{
		return NULL;
	}

	// locate node
	for(; NULL != node && counter < position; node = node->next, counter++);

	if(NULL != node)
	{
		return node->data;
	}

	return NULL;
}
