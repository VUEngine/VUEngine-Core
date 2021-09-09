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
#include <VirtualCircularList.h>
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
void VirtualCircularList::constructor()
{
	Base::constructor();

	// set members' default values
	this->head = NULL;
	this->tail = NULL;
}

/**
 * Class destructor
 */
void VirtualCircularList::destructor()
{
	// make sure we remove all nodes
	VirtualCircularList::clear(this);

	// destroy super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Remove all nodes from the list
 */
void VirtualCircularList::clear()
{
	if(this->head)
	{
		// point to the head
		VirtualNode node = this->head;

		// while node doesn't reach the head again
		do
		{
			// call destructor
			delete node;

			// move the node to the head
			node = node->next;
		}
		while(node != this->head);

		this->head = NULL;
		this->tail = NULL;
	}
}

/**
 * Add a new node to the beginning of the list
 *
 * @param data
 */
int32 VirtualCircularList::pushFront(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	// set previous if list isn't empty
	if(this->head)
	{
		this->head->previous = newNode;
	}

	// assign the node to the head of the list
	newNode->next = this->head;
	newNode->previous = this->tail;

	// move the head
	this->head = newNode;

	// set the tail
	if(this->tail)
	{
		this->tail->next = this->head;
	}
	else
	{
		this->tail = this->head;
		newNode->next = newNode->previous = newNode;
	}

	return true;
}

/**
 * Remove the first element from the list
 *
 * @return			Removed element
 */
void* VirtualCircularList::popFront()
{
	if(this->head)
	{
		VirtualNode node = this->head;
		void* data = node->data;

		if(this->head != this->tail)
		{
			this->head = this->head->next;

			// move head's previous pointer
			this->head->previous = this->tail;
			this->tail->next = this->head;
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
 * Add a new node to the end of the list
 *
 * @param data
 */
int32 VirtualCircularList::pushBack(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	// set previous if list isn't empty
	if(this->tail)
	{
		this->tail->next = newNode;
	}

	// assign the node to the head of the list
	newNode->next = this->head;
	newNode->previous = this->tail;

	// move the tail
	this->tail = newNode;

	// set the tail
	if(this->head)
	{
		this->head->previous = this->tail;
	}
	else
	{
		this->head = this->tail;
		newNode->next = newNode->previous = newNode;
	}

	return true;
}

/**
 * Remove the last element from the list
 *
 * @return			Removed element
 */
void* VirtualCircularList::popBack()
{
	if(this->tail)
	{
		VirtualNode node = this->tail;
		void* data = node->data;

		if(this->head != this->tail)
		{
			this->tail = this->tail->previous;

			// move head's previous pointer
			this->tail->next = this->head;
			this->head->previous = this->tail;
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
 * Check if a node is part of this list
 *
 * @private
 * @param node	node to check
 */
bool VirtualCircularList::checkThatNodeIsPresent(VirtualNode node)
{
	if(!node)
	{
		return false;
	}

	if(this->head)
	{
		// point to the head
		VirtualNode auxNode = this->head;

		// while node doesn't reach the head again
		do
		{
			if(auxNode == node)
			{
				return true;
			}

			// move the node to the head
			auxNode = auxNode->next;
		}
		while(auxNode != this->head);
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
VirtualNode VirtualCircularList::insertAfter(VirtualNode node, const void* const data)
{
	if(!VirtualCircularList::checkThatNodeIsPresent(this, node))
	{
		return NULL;
	}

	VirtualNode newNode = NULL;

	if(!node || node == this->tail)
	{
		VirtualCircularList::pushBack(this, data);

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
 * Retrieve the number of objects the list has
 *
 * @return				Number of objects
 */
int32 VirtualCircularList::getSize()
{
	int32 counter = 0;

	if(this->head)
	{
		// point to the head
		VirtualNode node = this->head;

		// while node doesn't reach the head again
		do
		{
			counter++;

			// increment counter
			ASSERT(counter < LIST_MAX_SIZE, "VirtualCircularList::getSize: endless list getting size");

			// move the node to the head
			node = node->next;
		}
		while(node != this->head);
	}

	return counter;
}

/**
 * Return data pointer of object in the given index node
 *
 * @param item
 * @return				Data pointer of object in the given index node
 */
void* VirtualCircularList::getNodeData(int32 item)
{
	// get the node
	VirtualNode node = VirtualCircularList::getNode(this, item);

	// return the data
	return (node) ? node->data : NULL;
}

/**
 * Get node at item position
 *
 * @param item		Numeric position of node
 * @return				Node
 */
VirtualNode VirtualCircularList::getNode(int32 item)
{
	int32 counter = 0;

	int32 listSize = VirtualCircularList::getSize(this);

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
 * Checks if node belongs to the list
 *
 * @param node		VirtualNode
 * @return				node's position
 */
int32 VirtualCircularList::getNodePosition(VirtualNode node)
{
	if(node && this->head)
	{
		VirtualNode auxNode = this->head;

		int32 position = 0;

		do
		{
			if(node == auxNode)
			{
				return position;
			}

			auxNode = auxNode->next;
			position++;
		}
		while(auxNode != this->head);
	}

	return -1;
}

/**
 * Checks if node belongs to the list
 *
 * @param node		VirtualNode
 * @return			boolean
 */
bool VirtualCircularList::isValidNode(VirtualNode node)
{
	return 0 <= VirtualCircularList::getNodePosition(this, node);
}

/**
 * Get address of node containing dataPointer
 *
 * @param dataPointer
 * @return								Node
 */
void* VirtualCircularList::getObject(void* const dataPointer)
{
	// check if data pointer is valid
	if(!dataPointer)
	{
		return NULL;
	}

	if(this->head)
	{
		// point to the head
		VirtualNode node = this->head;

		// while node doesn't reach the head again
		do
		{
			if(node->data == dataPointer)
			{
				return node;
			}

			node = node->next;
		}
		while(node != this->head);
	}

	return NULL;
}

/**
 * Remove a node
 *
 * @param node		Node to be removed from list
 * @return				Flag whether action was successful or not
 */
bool VirtualCircularList::removeNode(VirtualNode node)
{
	// if node isn't null
	if(VirtualCircularList::isValidNode(this, node))
	{
		// if the node is the head of the list
		if(this->head == this->tail)
		{
			if(node == this->head)
			{
				this->head = this->tail = NULL;
			}
		}
		else
		{
			if(node == this->head)
			{
				this->head = this->head->next;
				this->head->previous = this->tail;
				this->tail->next = this->head;
			}
			else if(node == this->tail)
			{
				this->tail = this->tail->previous;
				this->tail->next = this->head;
				this->head->previous = this->tail;
			}
			else
			{
				// Make sure that node belongs to this list
				// join the previous and next nodes
				node->previous->next = node->next;
				node->next->previous = node->previous;
			}
		}

		delete node;
		return true;
	}

	NM_ASSERT(false, "VirtualCircularList::removeNode: removing invalid node");

	return false;
}

/**
 * Find a node in the list
 *
 * @param dataPointer
 * @return								Node
 */
VirtualNode VirtualCircularList::find(const void* const dataPointer)
{
	if(this->head)
	{
		VirtualNode node = this->head;

		do
		{
			if(node->data == (void*)dataPointer)
			{
				return node;
			}

			node = node->next;
		}
		while(node != this->head);
	}

	return NULL;
}

/**
 * Get position of data in the list
 *
 * @param dataPointer
 * @return								Numeric position of node, or -1 when node could not be found
 */
int32 VirtualCircularList::getDataPosition(const void* const dataPointer)
{
	if(this->head)
	{
		int32 position = 0;
		VirtualNode node = this->head;

		do
		{
			if(node->data == (void*)dataPointer)
			{
				return position;
			}

			node = node->next;
			position++;
		}
		while(node != this->head);
	}

	return -1;
}

/**
 * Remove a node from the list
 *
 * @param dataPointer
 * @return					Flag whether action was successful or not
 */
bool VirtualCircularList::removeElement(const void* const dataPointer)
{
	return VirtualCircularList::removeNode(this, VirtualCircularList::find(this, dataPointer));
}

/**
 * Copy source list's elements to destination list
 *
 * @param sourceList
 */
void VirtualCircularList::copy(VirtualCircularList sourceList)
{
#ifdef __DEBUG
	int32 counter = 0;
#endif

	if(sourceList->head)
	{
		VirtualNode node = sourceList->head;

		VirtualCircularList::clear(this);

		do
		{
			VirtualCircularList::pushBack(this, node->data);

			node = node->next;

			ASSERT(++counter < LIST_MAX_SIZE, "VirtualCircularList::copy: endless list copying");
		}
		while(node != sourceList->head);
	}
}

/**
 * Copy source list's elements to destination list in reverse order
 *
 */
void VirtualCircularList::reverse()
{
	if(this->head && this->tail != this->head)
	{
		VirtualNode node = this->head;
		VirtualNode nextNode = node->next;

		do
		{
			VirtualNode helper = node->next;
			node->next = node->previous;
			node->previous = helper;

			node = nextNode;
			nextNode = nextNode->next;
		}
		while(nextNode != this->head);

		VirtualNode helper = node->next;
		node->next = node->previous;
		node->previous = helper;

		helper = this->head;
		this->head = this->tail;
		this->tail = helper;
	}
}

/**
 * Retrieve list's head's address
 *
 * @return			Node
 */
VirtualNode VirtualCircularList::begin()
{
	return this->head;
}

/**
 * Retrieve the first element
 *
 * @return		Head data
 */
void* VirtualCircularList::front()
{
	return this->head ? this->head->data : NULL;
}

/**
 * Retrieve list's last node
 *
 * @return		Node
 */
VirtualNode VirtualCircularList::end()
{
	return this->tail;
}

/**
 * Retrieve the last element
 *
 * @return		Tail data
 */
void* VirtualCircularList::back()
{
	return this->tail ? this->tail->data : NULL;
}

/**
 * Retrieve the last element
 *
 * @param node
 */
bool VirtualCircularList::moveHead(VirtualNode node)
{
	if(!VirtualCircularList::isValidNode(this, node))
	{
		return false;
	}

	this->head = node;
	this->tail = this->head->previous;

	return true;
}
