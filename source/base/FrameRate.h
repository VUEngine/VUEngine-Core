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

#ifndef FRAMERATE_H_
#define FRAMERATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define FrameRate_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define FrameRate_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(FrameRate);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

FrameRate FrameRate_getInstance();

void FrameRate_destructor(FrameRate this);

u16 FrameRate_getFps(FrameRate this);
void FrameRate_increaseFps(FrameRate this);
void FrameRate_print(FrameRate this, int col, int row);
void FrameRate_reset(FrameRate this);


#endif
