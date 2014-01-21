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
	/* raw frames per second */					\
	int rawFPS;									\
												\
	/* rendering frames per second */			\
	int renderFPS;								\
												\
	/* logic frames per second */				\
	int logicFPS;								\
												\
	/* physics frames per second */				\
	int physicsFPS;								\


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
	
	FrameRate_reset(this);
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
	
	this->rawFPS = 0;
	this->renderFPS = 0;
	this->logicFPS = 0;
	this->physicsFPS = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve raw FPS
int FrameRate_getRawFPS(FrameRate this){
	
	return this->rawFPS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve render FPS
int FrameRate_getRenderFPS(FrameRate this){
	
	return this->renderFPS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve raw logic FPS
int FrameRate_getLogicFPS(FrameRate this){
	
	return this->logicFPS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve raw logic FPS
int FrameRate_getPhysicsFPS(FrameRate this){
	
	return this->physicsFPS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// increase the renderFPS count
void FrameRate_increaseRenderFPS(FrameRate this){
	
	this->renderFPS++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// increase the update raw renderFPS count
void FrameRate_increaseRawFPS(FrameRate this){
	
	this->rawFPS++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// increase the update raw renderFPS count
void FrameRate_increaseLogicFPS(FrameRate this){
	
	this->logicFPS++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// increase the update raw renderFPS count
void FrameRate_increasePhysicsFPS(FrameRate this){
	
	this->physicsFPS++;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print renderFPS
void FrameRate_print(FrameRate this, int col, int row){
	
	Printing_text("FPS", col, row++);
	Printing_text("Raw                          ", col, row);
	Printing_int(this->rawFPS, col + 8, row++);
	Printing_text("Render                       ", col, row);
	Printing_int(this->renderFPS, col + 8, row++);
	Printing_text("Logic                        ", col, row);
	Printing_int(this->logicFPS, col + 8, row++);
	Printing_text("Physics                      ", col, row);
	Printing_int(this->physicsFPS, col + 8, row++);
}
