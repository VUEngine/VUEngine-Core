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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VirtualNode.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VirtualNode_constructor(VirtualNode this, const void* const data);
static void VirtualNode_destructor(VirtualNode this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VirtualNode, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(VirtualNode, const void* const data)
__CLASS_NEW_END(VirtualNode, data);

// class's constructor
static void VirtualNode_constructor(VirtualNode this, const void* const data)
{
	__CONSTRUCT_BASE();

	// initialize members
	this->next = NULL;
	this->previous = NULL;
	this->data = (void*)data;
}

// class's destructor
static void VirtualNode_destructor(VirtualNode this)
{
	ASSERT(this, "VirtualNode::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set node's data pointer
void VirtualNode_setData(VirtualNode this, const void* const data)
{
	ASSERT(this, "VirtualNode::destructor: null this");

	this->data = (void*)data;
}

// retrieve data pointer
void* VirtualNode_getData(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getData: null this");

	return this->data;
}

// get next node's address
VirtualNode VirtualNode_getNext(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getNext: null this");

	return this->next;
}

// set node's next node's address
void VirtualNode_setNext(VirtualNode this, VirtualNode next)
{
	ASSERT(this, "VirtualNode::setNext: null this");

	this->next = next;
}

// set node's previous node's address
VirtualNode VirtualNode_getPrevious(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getPrevious: null this");

	return this->previous;
}

// set node's previous node's address
void VirtualNode_setPrevious(VirtualNode this, VirtualNode previous)
{
	ASSERT(this, "VirtualNode::setPrevious: null this");

	this->previous = previous;
}

// swap the data between two nodes
void VirtualNode_swapData(VirtualNode this, VirtualNode node)
{
	ASSERT(this, "VirtualNode::swapData: null this");

	// check that both nodes are valid and are not the same
	if(!(this && node && (this != node)))
	{
		return;
	}

	void* auxData = this->data;
	this->data = node->data;
	node->data = auxData;
}