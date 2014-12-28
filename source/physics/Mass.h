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

#ifndef MASS_H_
#define MASS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Mass_METHODS															\
		Object_METHODS															\

#define Mass_SET_VTABLE(ClassName)												\
		Object_SET_VTABLE(ClassName)											\

#define Mass_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* mass configuration */													\
	MassDefinition* massDefinition;												\
																				\
	/* radious */																\
	fix19_13 weight;															\

// A Mass which represent a generic object inside a Stage
__CLASS(Mass);

// defines a Mass configuration in ROM memory
typedef struct MassDefinition
{
	// the sprite
	fix19_13 weight;

} MassDefinition;

typedef const MassDefinition MassROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Mass, __PARAMETERS(fix19_13 weight));

void Mass_constructor(Mass this, fix19_13 weight);
void Mass_destructor(Mass this);
fix19_13 Mass_getWeight(Mass this);


#endif