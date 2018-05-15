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

class VirtualList : Object
{
	/**
	* @var VirtualNode head
	* @brief			A pointer to the head of the list
	* @memberof		VirtualList
	*/
	VirtualNode head;
	/**
	* @var VirtualNode tail
	* @brief			A pointer to the tail of the list
	* @memberof		VirtualList
	*/
	VirtualNode tail;

	void constructor(VirtualList this);
	void* back(VirtualList this);
	VirtualNode begin(VirtualList this);
	void clear(VirtualList this);
	void copy(VirtualList this, VirtualList sourceList);
	VirtualNode end(VirtualList this);
	VirtualNode find(VirtualList this, const void* const dataPointer);
	void* front(VirtualList this);
	int getDataPosition(VirtualList this, const void* const dataPointer);
	VirtualNode getNode(VirtualList this, int item);
	void* getNodeData(VirtualList this, int item);
	int getNodePosition(VirtualList this, VirtualNode node);
	void* getObject(VirtualList this, void* const dataPointer);
	void* getObjectAtPosition(VirtualList this, int position);
	int getSize(VirtualList this);
	VirtualNode insertAfter(VirtualList this, VirtualNode node, const void* const data);
	VirtualNode insertBefore(VirtualList this, VirtualNode node, const void* const data);
	void* popFront(VirtualList this);
	void* popBack(VirtualList this);
	int pushBack(VirtualList this, const void* const data);
	int pushFront(VirtualList this, const void* const data);
	bool removeNode(VirtualList this, VirtualNode node);
	bool removeElement(VirtualList this, const void* const dataPointer);
	void swap(VirtualList this, VirtualList secondList);
}


#endif
