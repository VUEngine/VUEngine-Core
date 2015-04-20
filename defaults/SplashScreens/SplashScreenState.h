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

#ifndef SPLASH_SCREEN_STATE_H_
#define SPLASH_SCREEN_STATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SplashScreenState_METHODS												\
	GameState_METHODS;											    			\
	__VIRTUAL_DEC(print);														\
	__VIRTUAL_DEC(processInput);												\

// declare the virtual methods which are redefined
#define SplashScreenState_SET_VTABLE(ClassName)									\
	GameState_SET_VTABLE(ClassName)								   				\
	__VIRTUAL_SET(ClassName, SplashScreenState, enter);							\
	__VIRTUAL_SET(ClassName, SplashScreenState, exit);							\
	__VIRTUAL_SET(ClassName, SplashScreenState, execute);						\
	__VIRTUAL_SET(ClassName, SplashScreenState, resume);						\
	__VIRTUAL_SET(ClassName, SplashScreenState, handleMessage);					\
	__VIRTUAL_SET(ClassName, SplashScreenState, processInput);					\


__CLASS(SplashScreenState);

#define SplashScreenState_ATTRIBUTES								   			\
														            			\
	/* inherits */																\
	GameState_ATTRIBUTES														\
												            					\
	/* state to enter after this one */											\
	GameState nextState;														\
														           				\
	/* definition of screen's stage */											\
	StageDefinition* stageDefinition;											\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void SplashScreenState_constructor(SplashScreenState this);
void SplashScreenState_destructor(SplashScreenState this);
void SplashScreenState_enter(SplashScreenState this, void* owner);
void SplashScreenState_execute(SplashScreenState this, void* owner);
void SplashScreenState_exit(SplashScreenState this, void* owner);
void SplashScreenState_resume(SplashScreenState this, void* owner);
bool SplashScreenState_handleMessage(SplashScreenState this, void* owner, Telegram telegram);
void SplashScreenState_processInput(SplashScreenState this, u16 releasedKey);
void SplashScreenState_print(SplashScreenState this);
void SplashScreenState_setNextstate(SplashScreenState this, GameState nextState);


#endif