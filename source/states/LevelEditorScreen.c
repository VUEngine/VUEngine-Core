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

#ifdef __LEVEL_EDITOR

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <LevelEditorScreen.h>
#include <LevelEditor.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

static void LevelEditorScreen_destructor(LevelEditorScreen this);

// class's constructor
static void LevelEditorScreen_constructor(LevelEditorScreen this);

// state's enter
static void LevelEditorScreen_enter(LevelEditorScreen this, void* owner);

// state's execute
static void LevelEditorScreen_execute(LevelEditorScreen this, void* owner);

// state's enter
static void LevelEditorScreen_exit(LevelEditorScreen this, void* owner);

// state's on message
static int LevelEditorScreen_handleMessage(LevelEditorScreen this, void* owner, Telegram telegram);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											DECLARATIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */
extern const u16 ASCII_CH[];
extern State __CONCAT(START_LEVEL, _getInstance)();

enum Screens {
	kPvbScreen = 0,
	kPrecautionScreen,
	kVbJaeScreen,
	kLevelEditorExitScreen
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define LevelEditorScreen_ATTRIBUTES			\
												\
	/* inherits */								\
	State_ATTRIBUTES							\



__CLASS_DEFINITION(LevelEditorScreen);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// it's a singleton
__SINGLETON(LevelEditorScreen);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void LevelEditorScreen_constructor(LevelEditorScreen this){
		
	__CONSTRUCT_BASE(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
static void LevelEditorScreen_destructor(LevelEditorScreen this){
	
	// destroy base
	__SINGLETON_DESTROY(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's enter
static void LevelEditorScreen_enter(LevelEditorScreen this, void* owner){
	
	Clock_pause(Game_getInGameClock(Game_getInstance()), true);
	LevelEditor_start(LevelEditor_getInstance(), (Level)StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
static void LevelEditorScreen_execute(LevelEditorScreen this, void* owner){

	LevelEditor_update(LevelEditor_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's exit 
static void LevelEditorScreen_exit(LevelEditorScreen this, void* owner){
	
	LevelEditor_stop(LevelEditor_getInstance());
	Clock_pause(Game_getInGameClock(Game_getInstance()), false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's on message
static int LevelEditorScreen_handleMessage(LevelEditorScreen this, void* owner, Telegram telegram){
	
	// process message
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				MessageDispatcher_dispatchMessage(0, (Object)this, (Object)LevelEditor_getInstance(), kKeyPressed, ((u16*)Telegram_getExtraInfo(telegram)));
			}
			break;
	}

	return true;
}

#endif