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

#ifndef FRAMERATE_H_
#define FRAMERATE_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// declare the virtual methods
#define FrameRate_METHODS											\
		Object_METHODS												\


// declare the virtual methods which are redefined
#define FrameRate_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)								\
		
	
__CLASS(FrameRate);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
FrameRate FrameRate_getInstance();

// class's destructor
void FrameRate_destructor(FrameRate this);

// reset internal values
void FrameRate_reset(FrameRate this);

// retrieve fps
int FrameRate_getFps(FrameRate this);

// retrieve raw fps
int FrameRate_getRawFps(FrameRate this);

// increase the update fps count
void FrameRate_increaseRenderFPS(FrameRate this);

// increase the update raw fps count
void FrameRate_increaseRawFPS(FrameRate this);

// increase the update raw fps count
void FrameRate_increaseLogicFPS(FrameRate this);

// increase the update raw fps count
void FrameRate_increasePhysicsFPS(FrameRate this);

// test if FPS are almost at their maximum
int FrameRate_areFPSHigh(FrameRate this);

// print fps
void FrameRate_print(FrameRate this,int col,int row);

// print renderFPS
void FrameRate_printLastRecord(FrameRate this, int col, int row);

#endif /*FRAMERATE_H_*/
