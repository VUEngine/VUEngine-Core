/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef VIRTUAL_NODE_H_
#define VIRTUAL_NODE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define VirtualNode_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

#define VirtualNode_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

#define VirtualNode_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\
		/**
		 * @var VirtualNode next
		 * @brief			pointer to next node
		 * @memberof		VirtualNode
		 */																								\
		VirtualNode next;																				\
		/**
		 * @var VirtualNode previous
		 * @brief			pointer to previous node
		 * @memberof		VirtualNode
		 */																								\
		VirtualNode previous;																			\
		/**
		 * @var void*		data
		 * @brief			pointer to the data
		 * @memberof		VirtualNode
		 */																								\
		void* data;																						\

__CLASS(VirtualNode);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(VirtualNode, const void* const data);

void* VirtualNode_getData(VirtualNode this);
VirtualNode VirtualNode_getNext(VirtualNode this);
VirtualNode VirtualNode_getPrevious(VirtualNode this);
void VirtualNode_swapData(VirtualNode this, VirtualNode node);


#endif
