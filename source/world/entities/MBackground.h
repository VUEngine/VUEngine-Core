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

#ifndef M_BACKGROUND_H_
#define M_BACKGROUND_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <MSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define MBackground_METHODS														\
		Entity_METHODS															\

#define MBackground_SET_VTABLE(ClassName)										\
		Entity_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, MBackground, isVisible);						\
		__VIRTUAL_SET(ClassName, MBackground, transform);						\
		__VIRTUAL_SET(ClassName, MBackground, initialTransform);				\
		__VIRTUAL_SET(ClassName, MBackground, updateSpritePosition);			\

// A MBackground which represent a generic object inside a Stage
#define MBackground_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Entity_ATTRIBUTES;															\

__CLASS(MBackground);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a Scrolling background
typedef EntityDefinition MBackgroundDefinition;

// defines a Scrolling background in ROM memory
typedef const EntityDefinition MBackgroundROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(MBackground, __PARAMETERS(MBackgroundDefinition* mBackgroundDefinition, s16 id));

void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id);
void MBackground_destructor(MBackground this);
void MBackground_initialTransform(MBackground this, Transformation* environmentTransform);
void MBackground_transform(MBackground this, Transformation* environmentTransform);
int MBackground_isVisible(MBackground this, int pad);
int MBackground_updateSpritePosition(MBackground this);


#endif