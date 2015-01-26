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

#ifndef VBJAENGINE_DEFAULT_AUTOMATIC_PAUSE_SCREEN_STATE_H_
#define VBJAENGINE_DEFAULT_AUTOMATIC_PAUSE_SCREEN_STATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define VBJaEngineDefaultAutomaticPauseScreenState_METHODS											\
	GameState_METHODS;												    			\

// declare the virtual methods which are redefined
#define VBJaEngineDefaultAutomaticPauseScreenState_SET_VTABLE(ClassName)								\
	GameState_SET_VTABLE(ClassName)								    				\
	__VIRTUAL_SET(ClassName, VBJaEngineDefaultAutomaticPauseScreenState, enter);						\
	__VIRTUAL_SET(ClassName, VBJaEngineDefaultAutomaticPauseScreenState, exit);						\
	__VIRTUAL_SET(ClassName, VBJaEngineDefaultAutomaticPauseScreenState, handleMessage);				\

__CLASS(VBJaEngineDefaultAutomaticPauseScreenState);

#define VBJaEngineDefaultAutomaticPauseScreenState_ATTRIBUTES							        			\
														            				\
	/* inherits */																	\
	GameState_ATTRIBUTES															\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

VBJaEngineDefaultAutomaticPauseScreenState VBJaEngineDefaultAutomaticPauseScreenState_getInstance(void);


#endif