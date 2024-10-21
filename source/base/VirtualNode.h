/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIRTUAL_NODE_H_
#define VIRTUAL_NODE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class VirtualNode
///
/// Inherits from Object
///
/// Implements an element of linked lists.
/// @ingroup base
class VirtualNode : Object
{
	/// @protectedsection

	/// Pointer to the next node in the linked list
	VirtualNode next;

	/// Pointer to the previous node in the linked list
	VirtualNode previous;

	/// Pointer to the data
	void* data;

	/// @publicsection

	/// Class' constructor
	/// @param data: Pointer to the data hold by this node
	void constructor(const void* const data);

	/// Retrieve the pointer to the data.
	/// @return Pointer to the data
	void* getData();

	/// Retrieve the next node in the linked list.
	/// @return Next node in the linked list
	VirtualNode getNext();

	/// Retrieve the previous node in the linked list.
	/// @return Previous node in the linked list
	VirtualNode getPrevious();

	/// Swap the data with another node
	/// @param node: Node to swap data with
	void swapData(VirtualNode node);
}


#endif
