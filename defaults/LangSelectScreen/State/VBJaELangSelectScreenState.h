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

#ifndef VBJAENGINE_LANGUAGE_SELECTION_SCREEN_STATE_H_
#define VBJAENGINE_LANGUAGE_SELECTION_SCREEN_STATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <GameState.h>
#include <OptionsSelector.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define VBJaELangSelectScreenState_METHODS												\
	GameState_METHODS;											    					\

// declare the virtual methods which are redefined
#define VBJaELangSelectScreenState_SET_VTABLE(ClassName)								\
	GameState_SET_VTABLE(ClassName)								    					\
	__VIRTUAL_SET(ClassName, VBJaELangSelectScreenState, enter);						\
	__VIRTUAL_SET(ClassName, VBJaELangSelectScreenState, exit);							\
	__VIRTUAL_SET(ClassName, VBJaELangSelectScreenState, execute);						\
	__VIRTUAL_SET(ClassName, VBJaELangSelectScreenState, resume);						\
	__VIRTUAL_SET(ClassName, VBJaELangSelectScreenState, handleMessage);				\


__CLASS(VBJaELangSelectScreenState);

#define VBJaELangSelectScreenState_ATTRIBUTES								   			\
														            					\
	/* inherits */																		\
	GameState_ATTRIBUTES																\
														            					\
	/* state to enter after this one */													\
	GameState nextState;																\
														            					\
	/* definition of screen's stage */													\
	StageDefinition* stageDefinition;													\
																						\
	OptionsSelector languageSelector;													\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

VBJaELangSelectScreenState VBJaELangSelectScreenState_getInstance(void);

void VBJaELangSelectScreenState_setNextstate(VBJaELangSelectScreenState this, GameState nextState);


#endif