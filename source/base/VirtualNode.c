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
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VirtualNode_constructor(VirtualNode this, const void* const data);
static void VirtualNode_destructor(VirtualNode this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class   VirtualNode
 * @extends Object
 * @brief
 */

__CLASS_DEFINITION(VirtualNode, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(VirtualNode, const void* const data)
__CLASS_NEW_END(VirtualNode, data);

/**
 * Class constructor
 *
 * @memberof    VirtualNode
 * @private
 *
 * @param this  Function scope
 * @param data
 */
static void VirtualNode_constructor(VirtualNode this, const void* const data)
{
	__CONSTRUCT_BASE(Object);

	// initialize members
	this->next = NULL;
	this->previous = NULL;
	this->data = (void*)data;
}

/**
 * Class destructor
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 */
__attribute__((unused)) static void VirtualNode_destructor(VirtualNode this)
{
	ASSERT(this, "VirtualNode::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Set node's data pointer
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 * @param data
 */
void VirtualNode_setData(VirtualNode this, const void* const data)
{
	ASSERT(this, "VirtualNode::destructor: null this");

	this->data = (void*)data;
}

/**
 * Retrieve node's data pointer
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 *
 * @return      Data pointer
 */
void* VirtualNode_getData(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getData: null this");

	return this->data;
}

/**
 * Get next node's address
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 *
 * @return      Node
 */
VirtualNode VirtualNode_getNext(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getNext: null this");

	return this->next;
}

/**
 * set next node's address
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 * @param next
 */
void VirtualNode_setNext(VirtualNode this, VirtualNode next)
{
	ASSERT(this, "VirtualNode::setNext: null this");

	this->next = next;
}

/**
 * Get previous node's address
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 *
 * @return      Node
 */
VirtualNode VirtualNode_getPrevious(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getPrevious: null this");

	return this->previous;
}

/**
 * Set previous node's address
 *
 * @memberof        VirtualNode
 * @public
 *
 * @param this      Function scope
 * @param previous
 */
void VirtualNode_setPrevious(VirtualNode this, VirtualNode previous)
{
	ASSERT(this, "VirtualNode::setPrevious: null this");

	this->previous = previous;
}

/**
 * Swap the data between two nodes
 *
 * @memberof    VirtualNode
 * @public
 *
 * @param this  Function scope
 * @param node
 */
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
