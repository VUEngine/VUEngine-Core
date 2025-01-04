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

#include "VirtualList.h"

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

void VirtualList::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// set members' default values
	this->head = NULL;
	this->tail = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualList::destructor()
{
	// make sure we remove all nodes
	VirtualList::clear(this);

	// destroy super object

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualList::front()
{
	return NULL != this->head ? this->head->data : NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualList::back()
{
	return NULL != this->tail ? this->tail->data : NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::begin()
{
	return this->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::end()
{
	return this->tail;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::find(const void* const data)
{
	VirtualNode node = this->head;
	
	for(; NULL != node && node->data != (void*)data; node = node->next);

	return node;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::getNode(int32 item)
{
	int32 counter = 0;

	int32 listSize = VirtualList::getCount(this);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualList::getDataIndex(const void* const data)
{
	VirtualNode node = this->head;
	int32 position = 0;

	for(; NULL != node && node->data != (void*)data; node = node->next, position++);

	return !node ? -1 : position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualList::getNodeIndex(VirtualNode node)
{
	VirtualNode currentNode = this->head;
	int32 position = 0;

	for(; NULL != node && currentNode && currentNode != node; currentNode = currentNode->next, position++);

	return !node || !currentNode ? -1 : position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualList::getDataAtIndex(int32 position)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VirtualList::getCount()
{
	int32 counter = 0;

	VirtualNode node = this->head;

	while(NULL != node)
	{
		// load next node
		node = node->next;

		++counter;

		// increment counter
		ASSERT(counter < LIST_MAX_SIZE, "VirtualList::getCount: endless list getting size");
	}

	return counter;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::pushFront(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	ASSERT(data, "VirtualList::pushFront: null data");

	HardwareManager::suspendInterrupts();

	// set the tail
	if(NULL == this->tail)
	{
		this->tail = this->head = newNode;
	}
	else
	{
		// link new node to the head
		newNode->next = this->head;

		// link the head to the new node
		this->head->previous = newNode;
		
		// move the head
		this->head = newNode;
	}

	HardwareManager::resumeInterrupts();

	return newNode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualList::pushBack(const void* const data)
{
	VirtualNode newNode = new VirtualNode(data);

	ASSERT(data, "VirtualList::pushBack: null data");

	HardwareManager::suspendInterrupts();

	// set the tail
	if(NULL == this->head)
	{
		this->head = this->tail = newNode;
	}
	else
	{
		// link new node to the tail
		newNode->previous = this->tail;

		// link the tail to the new node
		this->tail->next = newNode;

		// move the tail
		this->tail = newNode;
	}

	HardwareManager::resumeInterrupts();

	return newNode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualList::popFront()
{
	HardwareManager::suspendInterrupts();

	// if head isn't null
	if(NULL != this->head)
	{
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualList::popBack()
{
	HardwareManager::suspendInterrupts();

	// if tail isn't null
	if(NULL != this->tail)
	{
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VirtualList::removeData(const void* const data)
{
	return VirtualList::doRemoveNode(this, VirtualList::find(this, data));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualList::reverse()
{
	HardwareManager::suspendInterrupts();

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
		while(NULL != nextNode);

		VirtualNode helper = node->next;
		node->next = node->previous;
		node->previous = helper;

		helper = this->head;
		this->head = this->tail;
		this->tail = helper;
	}

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualList::deleteData()
{
	if(!isDeleted(this->head))
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->head; NULL != node;)
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

		this->head = this->tail = NULL;

		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	HardwareManager::resumeInterrupts();

	// free dynamic memory
	delete node;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
