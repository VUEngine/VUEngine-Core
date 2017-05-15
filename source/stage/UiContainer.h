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

#ifndef UI_CONTAINER_H_
#define UI_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
//											CLASS'S ROM DECLARATION
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
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(UiContainer, UiContainerDefinition* uiContainerDefinition);

void UiContainer_destructor(UiContainer this);
void UiContainer_addEntities(UiContainer this, PositionedEntity* entities);
void UiContainer_transform(UiContainer this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void UiContainer_initialTransform(UiContainer this, Transformation* environmentTransform, u32 recursive);


#endif
