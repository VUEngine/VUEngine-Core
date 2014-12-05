/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */
 
#ifndef SCROLL_BACKGROUND_H_
#define SCROLL_BACKGROUND_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Entity.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define ScrollBackground_METHODS							\
		Entity_METHODS										\
	
	

#define ScrollBackground_SET_VTABLE(ClassName)										\
		Entity_SET_VTABLE(ClassName)												\
		__VIRTUAL_SET(ClassName, ScrollBackground, isVisible);						\
		__VIRTUAL_SET(ClassName, ScrollBackground, transform);						\
		__VIRTUAL_SET(ClassName, ScrollBackground, updateSpritePosition);			\

	

// A ScrollBackground which represent a generic object inside a Stage
#define ScrollBackground_ATTRIBUTES					\
													\
	/* super's attributes */						\
	Entity_ATTRIBUTES;								\
													\
	/* sprites for scrolling */						\
	Sprite scrollSprites[2];						\


__CLASS(ScrollBackground);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// defines a Scrolling background 
typedef EntityDefinition ScrollBackgroundDefinition;

// defines a Scrolling background in ROM memory
typedef const ScrollBackgroundDefinition ScrollBackgroundROMDef;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(ScrollBackground, __PARAMETERS(ScrollBackgroundDefinition* backgroundDefinition, s16 ID));

// class's constructor
void ScrollBackground_constructor(ScrollBackground this, ScrollBackgroundDefinition* scrollBackgroundDefinition, s16 ID);

// class's destructor
void ScrollBackground_destructor(ScrollBackground this);

// transform
void ScrollBackground_transform(ScrollBackground this, Transformation* environmentTransform);

// whether it is visible
int ScrollBackground_isVisible(ScrollBackground this, int pad);

// check if must update sprite's position
int ScrollBackground_updateSpritePosition(ScrollBackground this);

#endif /*SCROLL_BACKGROUND_H_*/
