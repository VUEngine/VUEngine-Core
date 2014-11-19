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

#ifndef GAME_H_
#define GAME_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <HardwareManager.h>
#include <Clock.h>
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
#define Game_METHODS						\
		Object_METHODS						\


// declare the virtual methods which are redefined
#define Game_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Game, handleMessage);				\


__CLASS(Game);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
Game Game_getInstance();

// class's destructor
void Game_destructor(Game this);

// set game's initial state
void Game_start(Game this, State state);

// set game's state
void Game_changeState(Game this, State state);

// disable interrutps
void Game_disableHardwareInterrupts(Game this);

// enable interrupts
void Game_enableHardwareInterrupts(Game this);

// recover graphics memory
void Game_recoverGraphicMemory(Game this);

// erase engine's current status
void Game_reset(Game this);

// backup engine's current status 
void Game_saveState(Game this);

// reload engine's current status
void Game_recoverState(Game this);

// process input data according to the actual game status
void Game_handleInput(Game this, int currentKey);

// render the game
void Game_render(Game this);

// update engine's world's state
void Game_update(Game this);

// process a telegram
int Game_handleMessage(Game this, Telegram telegram);

// set rest flag
void Game_setRestFlag(Game this, int flag);

// retrieve clock
Clock Game_getClock(Game this);

// retrieve in game clock
Clock Game_getInGameClock(Game this);

// print engine' class's sizes
void Game_printClassSizes(int x, int y);

// retrieve last process' name
char* Game_getLastProcessName(Game this);

// retrieve optical config structure
Optical Game_getOptical(Game this);

// set optical config structure
void Game_setOptical(Game this, Optical optical);

#ifdef __DEBUG_TOOLS
int Game_isInDebugMode(Game this);
#endif

#endif /*GAMEENGINE_H_*/
