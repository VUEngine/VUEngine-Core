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

#include "VirtualNode.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualNode::constructor(const void* const data)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Initialize members
	this->next = NULL;
	this->previous = NULL;
	this->data = (void*)data;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualNode::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* VirtualNode::getData()
{
	return this->data;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualNode::getNext()
{
	return this->next;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualNode VirtualNode::getPrevious()
{
	return this->previous;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VirtualNode::swapData(VirtualNode node)
{
	if(isDeleted(node))
	{
		return;
	}

	void* auxData = this->data;
	this->data = node->data;
	node->data = auxData;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
