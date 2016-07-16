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

#ifndef MANAGED_M_BACKGROUND_H_
#define MANAGED_M_BACKGROUND_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackground.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedMBackground_METHODS(ClassName)															\
	MBackground_METHODS(ClassName);																		\

#define ManagedMBackground_SET_VTABLE(ClassName)														\
		MBackground_SET_VTABLE(ClassName);																\
	    __VIRTUAL_SET(ClassName, ManagedMBackground, initialTransform);									\
	    __VIRTUAL_SET(ClassName, ManagedMBackground, transform);										\
		__VIRTUAL_SET(ClassName, ManagedMBackground, updateVisualRepresentation);						\
		__VIRTUAL_SET(ClassName, ManagedMBackground, update);											\
		__VIRTUAL_SET(ClassName, ManagedMBackground, passMessage);										\

__CLASS(ManagedMBackground);

#define ManagedMBackground_ATTRIBUTES																	\
        /* it is derived from */																		\
        MBackground_ATTRIBUTES																			\
        /* sprites' list */																				\
        VirtualList managedSprites;																		\
        /* previous 2d projected position */															\
        VBVec2D previous2DPosition;																		\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedMBackground, MBackgroundDefinition* definition, int id, const char* const name);

void ManagedMBackground_constructor(ManagedMBackground this, MBackgroundDefinition* definition, int id, const char* const name);
void ManagedMBackground_destructor(ManagedMBackground this);
void ManagedMBackground_initialTransform(ManagedMBackground this, Transformation* environmentTransform);
void ManagedMBackground_transform(ManagedMBackground this, const Transformation* environmentTransform);
void ManagedMBackground_updateVisualRepresentation(ManagedMBackground this);
void ManagedMBackground_update(ManagedMBackground this);
int ManagedMBackground_passMessage(ManagedMBackground this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);


#endif
