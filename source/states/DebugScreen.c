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

#include <debugConfig.h>

#ifdef __DEBUG_TOOLS

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <DebugScreen.h>
#include <Debug.h>
#include <Game.h>
#include <Telegram.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

static void DebugScreen_destructor(DebugScreen this);

// class's constructor
static void DebugScreen_constructor(DebugScreen this);

// state's enter
static void DebugScreen_enter(DebugScreen this, void* owner);

// state's execute
static void DebugScreen_execute(DebugScreen this, void* owner);

// state's enter
static void DebugScreen_exit(DebugScreen this, void* owner);

// state's execute
static void DebugScreen_pause(DebugScreen this, void* owner){}

// state's execute
static void DebugScreen_resume(DebugScreen this, void* owner){}

// state's on message
static int DebugScreen_handleMessage(DebugScreen this, void* owner, Telegram telegram);

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
	kDebugExitScreen
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define DebugScreen_ATTRIBUTES			\
										\
	/* inherits */						\
	State_ATTRIBUTES					\



__CLASS_DEFINITION(DebugScreen);


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
__SINGLETON(DebugScreen);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void DebugScreen_constructor(DebugScreen this){
		
	__CONSTRUCT_BASE(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
static void DebugScreen_destructor(DebugScreen this){
	
	// destroy base
	__SINGLETON_DESTROY(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's enter
static void DebugScreen_enter(DebugScreen this, void* owner){
	
	Debug_show(Debug_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
static void DebugScreen_execute(DebugScreen this, void* owner){

	Clock_pause(Game_getInGameClock(Game_getInstance()), true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's exit 
static void DebugScreen_exit(DebugScreen this, void* owner){
	
	Debug_hide(Debug_getInstance());
	Clock_pause(Game_getInGameClock(Game_getInstance()), false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's on message
static int DebugScreen_handleMessage(DebugScreen this, void* owner, Telegram telegram){
	
	// process message
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));
		
				if(pressedKey & K_LL){
					
					Debug_showPreviousPage(Debug_getInstance());
				}
				else if(pressedKey & K_LR){
					
					Debug_showNextPage(Debug_getInstance());
				}
				else if(pressedKey & K_LU){
					
					Debug_showPreviousSubPage(Debug_getInstance());
				}
				else if(pressedKey & K_LD){
					
					Debug_showNextSubPage(Debug_getInstance());
				}
				else if(pressedKey & K_RL){
					
					Debug_diplaceLeft(Debug_getInstance());
				}
				else if(pressedKey & K_RR){
					
					Debug_diplaceRight(Debug_getInstance());
				}
				else if(pressedKey & K_RU){
					
					Debug_diplaceUp(Debug_getInstance());
				}
				else if(pressedKey & K_RD){
					
					Debug_diplaceDown(Debug_getInstance());
				}
			}
			break;
	}

	return true;
}

#endif