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

#ifndef LEVEL_H_
#define LEVEL_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <State.h>
#include <Telegram.h>
#include <Stage.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define Level_METHODS								\
		State_METHODS								\
		__VIRTUAL_DEC(transform);					\


// declare the virtual methods which are redefined
#define Level_SET_VTABLE(ClassName)								\
		State_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Level, enter);					\
		__VIRTUAL_SET(ClassName, Level, execute);				\
		__VIRTUAL_SET(ClassName, Level, exit);					\
		__VIRTUAL_SET(ClassName, Level, pause);					\
		__VIRTUAL_SET(ClassName, Level, resume);				\
		__VIRTUAL_SET(ClassName, Level, handleMessage);			\
		__VIRTUAL_SET(ClassName, Level, transform);				\
	

#define Level_ATTRIBUTES					\
											\
	/* super's attributes */				\
	State_ATTRIBUTES;						\
											\
	/* a pointer to the game's stage */		\
	Stage stage;							\
											\
	/* flag to allow streaming */			\
	int canStream;


__CLASS(Level);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// setup the init focus screen
Level Level_getInstance(void);

// class's constructor
void Level_constructor(Level this);

// class's destructor
void Level_destructor(Level this);

// state's enter
void Level_enter(Level this, void* owner);

// state's execute
void Level_execute(Level this, void* owner);

// state's enter
void Level_exit(Level this, void* owner);

// state's execute
void Level_pause(Level this, void* owner);

// state's execute
void Level_resume(Level this, void* owner);

// state's on message
int Level_handleMessage(Level this, void* owner, Telegram telegram);

// update level entities' positions
void Level_transform(Level this);

// process user input
void Level_onKeyPressed(Level this, int pressedKey);

// process user input
void Level_onKeyUp(Level this, int pressedKey);

// process user input
void Level_onKeyHold(Level this, int pressedKey);

// load a stage
void Level_loadStage(Level this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities, int flushCharGroups);

// set streaming flag
void Level_setCanStream(Level this, int canStream);

// get streaming flag
int Level_canStream(Level this);

// retrieve stage 
Stage Level_getStage(Level this);

#endif /*LEVEL_H_*/
