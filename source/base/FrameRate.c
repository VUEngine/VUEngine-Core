/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <FrameRate.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// FrameRate.c

#define FrameRate_ATTRIBUTES					\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* actuar frames per second */				\
	int fps;


// define the FrameRate
__CLASS_DEFINITION(FrameRate);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void FrameRate_constructor(FrameRate this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(FrameRate);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void FrameRate_constructor(FrameRate this){
	
	__CONSTRUCT_BASE(Object);
	
	this->fps = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void FrameRate_destructor(FrameRate this){
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset internal values
void FrameRate_reset(FrameRate this){
	
	this->fps = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve fps
int FrameRate_getFps(FrameRate this){
	
	return this->fps;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// increase the fps count
void FrameRate_increaseFPS(FrameRate this){
	
	this->fps++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print fps
void FrameRate_print(FrameRate this, int col, int row){
	
	vbjPrintText("FPS   ", col, row);
	//vbjPrintText("      ", col + 5, row);
	vbjPrintInt(this->fps, col + vbjIntLen(this->fps) + 2, row);
}
