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

#ifndef VIRTUAL_NODE_H_
#define VIRTUAL_NODE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define VirtualNode_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

#define VirtualNode_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

#define VirtualNode_ATTRIBUTES																			\
	    Object_ATTRIBUTES																				\
	    /**
         * @var VirtualNode next
         * @brief           pointer to next node
         * @memberof        VirtualNode
		 */																								\
	    VirtualNode next;																				\
	    /**
         * @var VirtualNode previous
         * @brief           pointer to previous node
         * @memberof        VirtualNode
		 */																								\
	    VirtualNode previous;																			\
	    /**
         * @var void*       data
         * @brief           pointer to the data
         * @memberof        VirtualNode
		 */																								\
	    void* data;																						\

__CLASS(VirtualNode);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(VirtualNode, const void* const data);

void* VirtualNode_getData(VirtualNode this);
VirtualNode VirtualNode_getNext(VirtualNode this);
VirtualNode VirtualNode_getPrevious(VirtualNode this);
void VirtualNode_swapData(VirtualNode this, VirtualNode node);


#endif
