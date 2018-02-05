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


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VirtualNode_constructor(VirtualNode this, const void* const data);
static void VirtualNode_destructor(VirtualNode this);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	VirtualNode
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(VirtualNode, Object);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(VirtualNode, const void* const data)
__CLASS_NEW_END(VirtualNode, data);

/**
 * Class constructor
 *
 * @memberof	VirtualNode
 * @private
 *
 * @param this	Function scope
 * @param data
 */
static void VirtualNode_constructor(VirtualNode this, const void* const data)
{
	ASSERT(this, "VirtualNode::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// initialize members
	this->next = NULL;
	this->previous = NULL;
	this->data = (void*)data;
}

/**
 * Class destructor
 *
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
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
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
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
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
 *
 * @return		Data pointer
 */
void* VirtualNode_getData(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getData: null this");

	return this->data;
}

/**
 * Get next node's address
 *
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
 *
 * @return		Node
 */
VirtualNode VirtualNode_getNext(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getNext: null this");

	return this->next;
}

/**
 * set next node's address
 *
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
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
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
 *
 * @return		Node
 */
VirtualNode VirtualNode_getPrevious(VirtualNode this)
{
	ASSERT(this, "VirtualNode::getPrevious: null this");

	return this->previous;
}

/**
 * Set previous node's address
 *
 * @memberof		VirtualNode
 * @public
 *
 * @param this		Function scope
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
 * @memberof	VirtualNode
 * @public
 *
 * @param this	Function scope
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
