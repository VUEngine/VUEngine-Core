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
#ifndef LEVEL_EDITOR_SCREEN_H_
#define LEVEL_EDITOR_SCREEN_H_

#ifdef __LEVEL_EDITOR
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <State.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define LevelEditorScreen_METHODS										\
	State_METHODS;									

// declare the virtual methods which are redefined
#define LevelEditorScreen_SET_VTABLE(ClassName)							\
	State_SET_VTABLE(ClassName)											\
	__VIRTUAL_SET(ClassName, LevelEditorScreen, enter);					\
	__VIRTUAL_SET(ClassName, LevelEditorScreen, execute);				\
	__VIRTUAL_SET(ClassName, LevelEditorScreen, exit);					\
	__VIRTUAL_SET(ClassName, LevelEditorScreen, handleMessage);			\


__CLASS(LevelEditorScreen);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// setup the init focus screen
LevelEditorScreen LevelEditorScreen_getInstance(void);

#endif

#endif /*LEVEL_EDITOR_SCREEN_H_*/
