/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <VirtualNode.h>

#include "VirtualCircularList.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// define a limit to prevent, and detect looped lists
#define LIST_MAX_SIZE 1000

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualCircularList::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Set members' default values
	this->head = NULL;
	this->tail = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualCircularList::destructor()
{
	// Make sure we remove all nodes
	VirtualCircularList::clear(this);

	// Destroy super object

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualCircularList::front()
{
	return this->head ? this->head->data : NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualCircularList::back()
{
	return this->tail ? this->tail->data : NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::begin()
{
	return this->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::end()
{
	return this->tail;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::find(const void* const data)
{
	if(this->head)
	{
		VirtualNode node = this->head;

		do
		{
			if(node->data == (void*)data)
			{
				return node;
			}

			node = node->next;
		}
		while(node != this->head);
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::getNode(int32 item)
{
	int32 counter = 0;

	int32 listSize = VirtualCircularList::getCount(this);

	VirtualNode node = this->head;

	// If not null head
	if(node)
	{
		// If item hasn't reached list's size
		if(item < listSize)
		{
			// Increase counter while node hasn't reached list's end
			// And counter hasn't reached the item requested
			while(node && counter < item)
			{
				// Increase counter
				counter++;

				// Load next node
				node = node->next;

				// If item reached
				if(counter == item)
				{
					// Return node's data
					return node;
				}
			}

			// If item reached
			if(counter == item)
			{
				// Return node's data
				return node;
			}

			return NULL;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualCircularList::getDataIndex(const void* const data)
{
	if(this->head)
	{
		int32 position = 0;
		VirtualNode node = this->head;

		do
		{
			if(node->data == (void*)data)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualCircularList::getNodeIndex(VirtualNode node)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualCircularList::getCount()
{
	int32 counter = 0;

	if(this->head)
	{
		// Point to the head
		VirtualNode node = this->head;

		// While node doesn't reach the head again
		do
		{
			counter++;

			// Increment counter
			ASSERT(counter < LIST_MAX_SIZE, "VirtualCircularList::getCount: endless list getting size");

			// Move the node to the head
			node = node->next;
		}
		while(node != this->head);
	}

	return counter;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::pushFront(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	// Set previous if list isn't empty
	if(this->head)
	{
		this->head->previous = newNode;
	}

	// Assign the node to the head of the list
	newNode->next = this->head;
	newNode->previous = this->tail;

	// Move the head
	this->head = newNode;

	// Set the tail
	if(this->tail)
	{
		this->tail->next = this->head;
	}
	else
	{
		this->tail = this->head;
		newNode->next = newNode->previous = newNode;
	}

	return newNode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualCircularList::pushBack(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	// Set previous if list isn't empty
	if(this->tail)
	{
		this->tail->next = newNode;
	}

	// Assign the node to the head of the list
	newNode->next = this->head;
	newNode->previous = this->tail;

	// Move the tail
	this->tail = newNode;

	// Set the tail
	if(this->head)
	{
		this->head->previous = this->tail;
	}
	else
	{
		this->head = this->tail;
		newNode->next = newNode->previous = newNode;
	}

	return newNode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		// Set previous if list isn't empty
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualCircularList::popFront()
{
	if(this->head)
	{
		VirtualNode node = this->head;
		void* data = node->data;

		if(this->head != this->tail)
		{
			this->head = this->head->next;

			// Move head's previous pointer
			this->head->previous = this->tail;
			this->tail->next = this->head;
		}
		else
		{
			// Set head
			this->head = NULL;
			this->tail = NULL;
		}

		// Free dynamic memory
		delete node;

		return data;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualCircularList::popBack()
{
	if(this->tail)
	{
		VirtualNode node = this->tail;
		void* data = node->data;

		if(this->head != this->tail)
		{
			this->tail = this->tail->previous;

			// Move head's previous pointer
			this->tail->next = this->head;
			this->head->previous = this->tail;
		}
		else
		{
			// Set head
			this->head = NULL;
			this->tail = NULL;
		}

		// Free dynamic memory
		delete node;

		return data;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VirtualCircularList::removeNode(VirtualNode node)
{
	// If node isn't null
	if(VirtualCircularList::checkThatNodeIsPresent(this, node))
	{
		// If the node is the head of the list
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
				// Join the previous and next nodes
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VirtualCircularList::removeData(const void* const data)
{
	return VirtualCircularList::removeNode(this, VirtualCircularList::find(this, data));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualCircularList::reverse()
{
	if(NULL != this->head && this->tail != this->head)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualCircularList::clear()
{
	if(this->head)
	{
		// Point to the head
		VirtualNode node = this->head;

		// While node doesn't reach the head again
		do
		{
			// Call destructor
			delete node;

			// Move the node to the head
			node = node->next;
		}
		while(node != this->head);

		this->head = NULL;
		this->tail = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualCircularList::deleteData()
{
	if(!isDeleted(this->head))
	{
		// Point to the head
		VirtualNode node = this->head;

		// While node doesn't reach the head again
		do
		{
			delete node->data;

			VirtualNode aux = node;

			node = node->next;

#ifndef __SHPPING
			if(isDeleted(aux))
			{
				continue;
			}
#endif

			delete aux;
		}
		while(node != this->head);

		this->head = NULL;
		this->tail = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VirtualCircularList::checkThatNodeIsPresent(VirtualNode node)
{
	if(!node)
	{
		return false;
	}

	if(this->head)
	{
		// Point to the head
		VirtualNode auxNode = this->head;

		// While node doesn't reach the head again
		do
		{
			if(auxNode == node)
			{
				return true;
			}

			// Move the node to the head
			auxNode = auxNode->next;
		}
		while(auxNode != this->head);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
