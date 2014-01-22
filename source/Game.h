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
#define Game_SET_VTABLE(ClassName)				\
		Object_SET_VTABLE(ClassName)			\


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
void Game_start(Game this, State state, int fadeDelay);

// set game's state
void Game_setState(Game this, State state, int fadeDelay);

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

// set rest flag
void Game_setRestFlag(Game this, int flag);

// retrieve clock
Clock Game_getClock(Game this);

// retrieve in game clock
Clock Game_getInGameClock(Game this);

// print engine' class's sizes
void Game_printClassSizes(int x, int y);


#endif /*GAMEENGINE_H_*/
