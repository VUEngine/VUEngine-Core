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
#include <SpriteManager.h>


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
		
	ASSERT(this, "Level::constructor: null this");

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Level);
	
	__CONSTRUCT_BASE(State);
	
	// construct the stage
	this->stage = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Level_destructor(Level this){
	
	ASSERT(this, "Level::destructor: null this");

	// destroy the stage
	if (this->stage) {
	
		// destroy the stage
		__DELETE(this->stage);
		
		this->stage = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE(State);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's enter
void Level_enter(Level this, void* owner){

	ASSERT(this, "Level::enter: null this");

	// reset the global clock
	//Clock_reset(Game_getClock(Game_getInstance()));
	
	Clock_reset(Game_getInGameClock(Game_getInstance()));
	Clock_start(Game_getInGameClock(Game_getInstance()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_execute(Level this, void* owner){

	ASSERT(this, "Level::execute: null this");

	// stream level
	// must be called before updating the other entities
	Stage_stream(this->stage);

	// update the stage
	__VIRTUAL_CALL(void, Container, update, (Container)this->stage);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's exit 
void Level_exit(Level this, void* owner){
	
	ASSERT(this, "Level::exit: null this");

	// destroy the state
	__DELETE(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_pause(Level this, void* owner){

	ASSERT(this, "Level::pause: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
void Level_resume(Level this, void* owner){	

	ASSERT(this, "Level::resume: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's on message
int Level_handleMessage(Level this, void* owner, Telegram telegram){

	ASSERT(this, "Level::handleMessage: null this");

	// process message
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:
			
			Level_onKeyPressed(this, *((int*)Telegram_getExtraInfo(telegram)));
			break;
			
		case kKeyUp:
			
			Level_onKeyUp(this, *((int*)Telegram_getExtraInfo(telegram)));
			break;
			
		case kKeyHold:
			
			Level_onKeyHold(this, *((int*)Telegram_getExtraInfo(telegram)));
			break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update level entities' positions
void Level_transform(Level this){
	
	ASSERT(this, "Level::transform: null this");
	ASSERT(this->stage, "Level::transform: null stage");
	
	// static to avoid call to _memcpy
	static Transformation environmentTransform = {
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// scale
			{1, 1},
			// rotation
			{0, 0, 0}			
	};
	
	__VIRTUAL_CALL(void, Container, transform, (Container)this->stage, __ARGUMENTS(&environmentTransform));

	Screen_update(Screen_getInstance());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
void Level_onKeyPressed(Level this, int pressedKey){

	ASSERT(this, "Level::onKeyPressed: null this");

	__CALL_VARIADIC(Container_propagateEvent((Container)this->stage, Container_onKeyPressed, pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
void Level_onKeyUp(Level this, int pressedKey){

	ASSERT(this, "Level::onKeyUp: null this");

	__CALL_VARIADIC(Container_propagateEvent((Container)this->stage, Container_onKeyUp, pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
void Level_onKeyHold(Level this, int pressedKey){

	ASSERT(this, "Level::onKeyHold: null this");

	__CALL_VARIADIC(Container_propagateEvent((Container)this->stage, Container_onKeyHold, pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load a stage
void Level_loadStage(Level this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities){
	
	ASSERT(this, "Level::loadStage: null this");
	ASSERT(stageDefinition, "Level::loadStage: null stageDefinition");

	// disable hardware interrupts
	Game_disableHardwareInterrupts(Game_getInstance());

	if (this->stage) {
	
		// destroy the stage
		__DELETE(this->stage);
	}

	// reset the engine state
	Game_reset(Game_getInstance());

	// construct the stage
	this->stage = __NEW(Stage);

	ASSERT(this->stage, "Level::loadStage: null stage");
	
	//load world entities
	Stage_load(this->stage, stageDefinition, loadOnlyInRangeEntities);

	// transform everything
	Level_transform(this);

	// sort all sprites' layers
	SpriteManager_sortAllLayers(SpriteManager_getInstance());

	// render sprites as soon as possible
	SpriteManager_render(SpriteManager_getInstance());
	
	// reset ingame clock and start it
	Clock_reset(Game_getInGameClock(Game_getInstance()));
	Clock_start(Game_getInGameClock(Game_getInstance()));
	
	// allow hardware interrupts
	Game_enableHardwareInterrupts(Game_getInstance());
}