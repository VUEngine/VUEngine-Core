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

#ifdef __ANIMATION_EDITOR

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <AnimationEditorScreen.h>
#include <AnimationEditor.h>
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

static void AnimationEditorScreen_destructor(AnimationEditorScreen this);

// class's constructor
static void AnimationEditorScreen_constructor(AnimationEditorScreen this);

// state's enter
static void AnimationEditorScreen_enter(AnimationEditorScreen this, void* owner);

// state's execute
static void AnimationEditorScreen_execute(AnimationEditorScreen this, void* owner);

// state's enter
static void AnimationEditorScreen_exit(AnimationEditorScreen this, void* owner);

// state's on message
static int AnimationEditorScreen_handleMessage(AnimationEditorScreen this, void* owner, Telegram telegram);

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
	kAnimationEditorExitScreen
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define AnimationEditorScreen_ATTRIBUTES			\
													\
	/* inherits */									\
	State_ATTRIBUTES								\



__CLASS_DEFINITION(AnimationEditorScreen);


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
__SINGLETON(AnimationEditorScreen);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void AnimationEditorScreen_constructor(AnimationEditorScreen this){
		
	__CONSTRUCT_BASE(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
static void AnimationEditorScreen_destructor(AnimationEditorScreen this){
	
	// destroy base
	__SINGLETON_DESTROY(State);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's enter
static void AnimationEditorScreen_enter(AnimationEditorScreen this, void* owner){
	
	Clock_pause(Game_getInGameClock(Game_getInstance()), true);
	AnimationEditor_start(AnimationEditor_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's execute
static void AnimationEditorScreen_execute(AnimationEditorScreen this, void* owner){

	AnimationEditor_update(AnimationEditor_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's exit 
static void AnimationEditorScreen_exit(AnimationEditorScreen this, void* owner){
	
	AnimationEditor_stop(AnimationEditor_getInstance());
	Clock_pause(Game_getInGameClock(Game_getInstance()), false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state's on message
static int AnimationEditorScreen_handleMessage(AnimationEditorScreen this, void* owner, Telegram telegram){
	
	// process message
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				MessageDispatcher_dispatchMessage(0, (Object)this, (Object)AnimationEditor_getInstance(), kKeyPressed, ((u16*)Telegram_getExtraInfo(telegram)));
			}
			break;
	}

	return true;
}

#endif