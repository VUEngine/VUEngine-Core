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

#include <Level.h>

#include <Game.h>
#include <Screen.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


	
__CLASS_DEFINITION(Level);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
void Level_constructor(Level this){
		
	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Level);
	
	__CONSTRUCT_BASE(State);
	
	// construct the stage
	this->stage = __NEW(Stage);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Level_destructor(Level this){
	
	// destroy the stage
	__DELETE(this->stage);
	
	// destroy the super object
	__DESTROY_BASE(State);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's enter
void Level_enter(Level this, void* owner){

	// reset the global clock
	//Clock_reset(Game_getClock(Game_getInstance()));
	
	Clock_reset(_inGameClock);
	Clock_start(_inGameClock);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_execute(Level this, void* owner){

	// update the stage
	__VIRTUAL_CALL(void, Container, update, (Container)this->stage);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's exit 
void Level_exit(Level this, void* owner){
	
	// destroy the state
	__DELETE(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_pause(Level this, void* owner){
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_resume(Level this, void* owner){	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's on message
int Level_handleMessage(Level this, void* owner, Telegram telegram){

	// process message
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:
			
			Level_onKeyPressed(this, *((int*)Telegram_getExtraInfo(telegram)));
			break;
			
		case kKeyUp:
			
			Level_onKeyUp(this, *((int*)Telegram_getExtraInfo(telegram)));
			break;
			
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw the level
void Level_render(Level this){
	
	Transformation environmentTransform = {
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// scale
			{1, 1},
			// rotation
			{0, 0, 0}			
	};
	
	__VIRTUAL_CALL(void, Container, render, (Container)this->stage, __ARGUMENTS(environmentTransform));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
void Level_onKeyPressed(Level this, int pressedKey){

	__CALL_VARIADIC(Container_propagateEvent((Container)this->stage, Container_onKeyPressed, pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
void Level_onKeyUp(Level this, int pressedKey){

	__CALL_VARIADIC(Container_propagateEvent((Container)this->stage, Container_onKeyUp, pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load a stage
void Level_loadStage(Level this, StageDefinition* stageDefinition, int fadeDelay){
	
	_asciiChar = (const u16*)ASCII_CH;
	
	// make a fade out
	Screen_FXFadeOut(Game_getInstance(), fadeDelay);
	
	//clear char and bgmap memory
	vbClearScreen();
	
	// reset the engine state
	Game_reset(Game_getInstance());
	
	// destroy the stage
	__DELETE(this->stage);

	// construct the stage
	this->stage = __NEW(Stage);

	//load world entities
	Stage_load(this->stage, stageDefinition);

	// reset ingame clock and start it
	Clock_reset(_inGameClock);
	Clock_start(_inGameClock);
	
	// turn back the background
	VIP_REGS[BKCOL] = 0x00;

	// make a fade in
	Screen_FXFadeIn(Game_getInstance(), fadeDelay);
}