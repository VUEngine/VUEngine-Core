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
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param data
 */
void VirtualNode::constructor(const void* const data)
{
	Base::constructor();

	// initialize members
	this->next = NULL;
	this->previous = NULL;
	this->data = (void*)data;
}

/**
 * Class destructor
 */
void VirtualNode::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Set node's data pointer
 *
 * @param data
 */
void VirtualNode::setData(const void* const data)
{
	this->data = (void*)data;
}

/**
 * Retrieve node's data pointer
 *
 * @return		Data pointer
 */
void* VirtualNode::getData()
{
	return this->data;
}

/**
 * Get next node's address
 *
 * @return		Node
 */
VirtualNode VirtualNode::getNext()
{
	return this->next;
}

/**
 * set next node's address
 *
 * @param next
 */
void VirtualNode::setNext(VirtualNode next)
{
	this->next = next;
}

/**
 * Get previous node's address
 *
 * @return		Node
 */
VirtualNode VirtualNode::getPrevious()
{
	return this->previous;
}

/**
 * Set previous node's address
 *
 * @param previous
 */
void VirtualNode::setPrevious(VirtualNode previous)
{
	this->previous = previous;
}

/**
 * Swap the data between two nodes
 *
 * @param node
 */
void VirtualNode::swapData(VirtualNode node)
{
	// check that both nodes are valid and are not the same
	if(!(this && node && (this != node)))
	{
		return;
	}

	void* auxData = this->data;
	this->data = node->data;
	node->data = auxData;
}
