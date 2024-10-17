/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIRTUAL_CIRCULAR_LIST_H_
#define VIRTUAL_CIRCULAR_LIST_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class VirtualNode;


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class VirtualCircularList
///
/// Inherits from Object
///
/// Implements a circular linked list of non fixed data type elements.
/// @ingroup base
class VirtualCircularList : Object
{
	/// @protectedsection

	/// List's first node
	VirtualNode head;
	
	/// List's last element
	VirtualNode tail;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Retrieve the first data element of the list.
	/// @return First data element
	void* front();

	/// Retrieve the last data element of the list.
	/// @return Last data element
	void* back();

	/// Retrieve the first node of the list.
	/// @return First node
	VirtualNode begin();

	/// Retrieve the last node of the list.
	/// @return Last node
	VirtualNode end();

	/// Retrieve the node that holds the provided data.
	/// @param data: Pointer to the data to look for
	/// @return Node that holds the provided data
	VirtualNode find(const void* const data);

	/// Retrieve the index of the node holding the provided data pointer.
	/// @param data: Pointer to the data to look for
	/// @return The index of the data in the linked list
	int32 getDataIndex(const void* const data);
	
	/// Retrieve the node at provided position within the list.
	/// @param index: Index within the list
	/// @return Node at the provided index
	VirtualNode getNode(int32 index);

	/// Retrieve the index of the provided node.
	/// @param node: Node to look for
	/// @return The index of the node in the linked list
	int32 getNodeIndex(VirtualNode node);

	/// Retrieve the data at the provided index in the list.
	/// @param index: Index within the list
	/// @return Data at index in the linked list
	void* getDataAtIndex(int32 index);

	/// Retrieve the number of nodes in the list.
	/// @return Number of nodes in the list
	int32 getCount();

	/// Add a new node to the start of the list with the provided data.
	/// @param data: Pointer to the data to insert into the list
	/// @return Node holding the data
	VirtualNode pushFront(const void* const data);

	/// Add a new node to the end of the list with the provided data.
	/// @param data: Pointer to the data to insert into the list
	/// @return Node holding the data
	VirtualNode pushBack(const void* const data);

	/// Add a new node to the list with the provided data after the provided node.
	/// @param node: Reference node
	/// @param data: Pointer to the data to insert into the list
	/// @return Node holding the data
	VirtualNode insertAfter(VirtualNode node, const void* const data);

	/// Add a new node to the list with the provided data before the provided node.
	/// @param node: Reference node
	/// @param data: Pointer to the data to insert into the list
	/// @return Node holding the data
	VirtualNode insertBefore(VirtualNode node, const void* const data);

	/// Remove the first node of the list.
	/// @return Pointer to the data hold by the removed node
	void* popFront();

	/// Remove the last node of the list.
	/// @return Pointer to the data hold by the removed node
	void* popBack();

	/// Remove the provided node from the list.
	/// @param node: Node to remove
	/// @return True if the node was successfully removed
	bool removeNode(VirtualNode node);

	/// Remove the provided data from the list.
	/// @param data: Pointer to the data to remove from the list
	/// @return True if the data was successfully removed
	bool removeData(const void* const data);
		
	/// Reverse the nodes of the list.
	void reverse();

	/// Copy the elements from the provided list.
	/// @param sourceList: List with the elements to copy
	void copy(VirtualCircularList sourceList);

	/// Remove all the nodes from the list without deleting the data.
	void clear();

	/// Delete all the data and nodes from the list.
	void deleteData();
}


#endif
