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

#ifndef M_BACKGROUND_H_
#define M_BACKGROUND_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define MBackground_METHODS(ClassName)																	\
		Entity_METHODS(ClassName)																		\

#define MBackground_SET_VTABLE(ClassName)																\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, MBackground, suspend);													\
		__VIRTUAL_SET(ClassName, MBackground, resume);													\

#define MBackground_ATTRIBUTES																			\
        /* super's attributes */																		\
        Entity_ATTRIBUTES																				\
        /* ROM definition */																			\
        MBackgroundDefinition* mBackgroundDefinition;													\

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

__CLASS_NEW_DECLARE(MBackground, MBackgroundDefinition* mBackgroundDefinition, s16 id, const char* const name);

void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id, const char* const name);
void MBackground_destructor(MBackground this);
void MBackground_suspend(MBackground this);
void MBackground_resume(MBackground this);


#endif
