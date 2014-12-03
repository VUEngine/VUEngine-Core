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

#ifndef GAME_STATE_H_
#define GAME_STATE_H_


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
#define GameState_METHODS											\
		State_METHODS												\
		__VIRTUAL_DEC(transform);									\


// declare the virtual methods which are redefined
#define GameState_SET_VTABLE(ClassName)								\
		State_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, GameState, enter);					\
		__VIRTUAL_SET(ClassName, GameState, execute);				\
		__VIRTUAL_SET(ClassName, GameState, exit);					\
		__VIRTUAL_SET(ClassName, GameState, pause);					\
		__VIRTUAL_SET(ClassName, GameState, resume);				\
		__VIRTUAL_SET(ClassName, GameState, handleMessage);			\
		__VIRTUAL_SET(ClassName, GameState, transform);				\
	

#define GameState_ATTRIBUTES										\
																	\
	/* super's attributes */										\
	State_ATTRIBUTES;												\
																	\
	/* a pointer to the game's stage */								\
	Stage stage;													\
																	\
	/* flag to allow streaming */									\
	int canStream;


__CLASS(GameState);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// setup the init focus screen
GameState GameState_getInstance(void);

// class's constructor
void GameState_constructor(GameState this);

// class's destructor
void GameState_destructor(GameState this);

// state's enter
void GameState_enter(GameState this, void* owner);

// state's execute
void GameState_execute(GameState this, void* owner);

// state's enter
void GameState_exit(GameState this, void* owner);

// state's execute
void GameState_pause(GameState this, void* owner);

// state's execute
void GameState_resume(GameState this, void* owner);

// state's on message
int GameState_handleMessage(GameState this, void* owner, Telegram telegram);

// update level entities' positions
void GameState_transform(GameState this);

// propagate message to all entities in the level
int GameState_propagateMessage(GameState this, int message);
			
// process user input
void GameState_onKeyPressed(GameState this, int pressedKey);

// process user input
void GameState_onKeyUp(GameState this, int pressedKey);

// process user input
void GameState_onKeyHold(GameState this, int pressedKey);

// process user input
void GameState_onMessage(GameState this, int message);

// load a stage
void GameState_loadStage(GameState this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities, int flushCharGroups);

// set streaming flag
void GameState_setCanStream(GameState this, int canStream);

// get streaming flag
int GameState_canStream(GameState this);

// retrieve stage 
Stage GameState_getStage(GameState this);

#endif /*GAME_STATE_H_*/
