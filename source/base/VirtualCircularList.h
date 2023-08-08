/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIRTUAL_CIRCULAR_LIST_H_
#define VIRTUAL_CIRCULAR_LIST_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VirtualNode;

/// @ingroup base
class VirtualCircularList : Object
{
	// A pointer to the head of the list
	VirtualNode head;
	// A pointer to the tail of the list
	VirtualNode tail;

	/// @publicsection
	void constructor();
	void* back();
	VirtualNode begin();
	void clear();
	void copy(VirtualCircularList sourceList);
	void reverse();
	VirtualNode end();
	VirtualNode find(const void* const dataPointer);
	void* front();
	int32 getDataPosition(const void* const dataPointer);
	VirtualNode getNode(int32 item);
	void* getNodeData(int32 item);
	int32 getNodePosition(VirtualNode node);
	void* getObject(void* const dataPointer);
	void* getObjectAtPosition(int32 position);
	int32 getSize();
	VirtualNode insertAfter(VirtualNode node, const void* const data);
	VirtualNode insertBefore(VirtualNode node, const void* const data);
	void* popFront();
	void* popBack();
	int32 pushBack(const void* const data);
	int32 pushFront(const void* const data);
	bool removeNode(VirtualNode node);
	bool removeElement(const void* const dataPointer);
	void swap(VirtualCircularList secondList);
	bool moveHead(VirtualNode node);
}


#endif
