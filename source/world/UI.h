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

#ifndef UI_H_
#define UI_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define UI_METHODS(ClassName)																			\
		Container_METHODS(ClassName)																	\
		__VIRTUAL_DEC(ClassName, void, addEntities, PositionedEntity* entities);						\

// declare the virtual methods which are redefined
#define UI_SET_VTABLE(ClassName)																		\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, UI, addEntities);														\
		__VIRTUAL_SET(ClassName, UI, transform);														\
		__VIRTUAL_SET(ClassName, UI, initialTransform);													\

// declare a UI
__CLASS(UI);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a UI for ROM memory
typedef struct UIDefinition
{
	// ui's entities
	PositionedEntity* entities;

	// the class type
	void* allocator;

} UIDefinition;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(UI, UIDefinition* uiDefinition);

void UI_destructor(UI this);
void UI_addEntities(UI this, PositionedEntity* entities);
void UI_transform(UI this, const Transformation* environmentTransform);
void UI_initialTransform(UI this, Transformation* environmentTransform);


#endif
