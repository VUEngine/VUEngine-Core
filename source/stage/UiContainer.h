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

#ifndef UI_CONTAINER_H_
#define UI_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define UiContainer_METHODS(ClassName)																	\
		Container_METHODS(ClassName)																	\
		__VIRTUAL_DEC(ClassName, void, addEntities, PositionedEntity* entities);						\

// declare the virtual methods which are redefined
#define UiContainer_SET_VTABLE(ClassName)																\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, UiContainer, addEntities);												\
		__VIRTUAL_SET(ClassName, UiContainer, transform);												\
		__VIRTUAL_SET(ClassName, UiContainer, initialTransform);										\

// declare a UiContainer
__CLASS(UiContainer);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a UI for ROM memory
typedef struct UiContainerDefinition
{
	// ui's entities
	PositionedEntity* entities;

	// the class allocator
	AllocatorPointer allocator;

} UiContainerDefinition;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(UiContainer, UiContainerDefinition* uiContainerDefinition);

void UiContainer_destructor(UiContainer this);
void UiContainer_addEntities(UiContainer this, PositionedEntity* entities);
void UiContainer_transform(UiContainer this, const Transformation* environmentTransform);
void UiContainer_initialTransform(UiContainer this, Transformation* environmentTransform, u32 recursive);


#endif
