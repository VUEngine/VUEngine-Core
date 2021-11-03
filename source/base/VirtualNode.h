/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIRTUAL_NODE_H_
#define VIRTUAL_NODE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
class VirtualNode : Object
{
	// pointer to next node
	VirtualNode next;
	// pointer to previous node
	VirtualNode previous;
	// pointer to the data
	void* data;

	/// @publicsection
	void constructor(const void* const data);
	void* getData();
	VirtualNode getNext();
	VirtualNode getPrevious();
	void swapData(VirtualNode node);
}


#endif
