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

#ifndef VIRTUAL_LIST_H_
#define VIRTUAL_LIST_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualNode.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
class VirtualList : Object
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
	void copy(VirtualList sourceList);
	VirtualNode end();
	VirtualNode find(const void* const dataPointer);
	void* front();
	int getDataPosition(const void* const dataPointer);
	VirtualNode getNode(int item);
	void* getNodeData(int item);
	int getNodePosition(VirtualNode node);
	void* getObject(void* const dataPointer);
	void* getObjectAtPosition(int position);
	int getSize();
	VirtualNode insertAfter(VirtualNode node, const void* const data);
	VirtualNode insertBefore(VirtualNode node, const void* const data);
	void* popFront();
	void* popBack();
	int pushBack(const void* const data);
	int pushFront(const void* const data);
	bool removeNode(VirtualNode node);
	bool removeElement(const void* const dataPointer);
	void swap(VirtualList secondList);
}


#endif
