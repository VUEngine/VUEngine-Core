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
#ifndef ANIMATION_EDITOR_SCREEN_H_
#define ANIMATION_EDITOR_SCREEN_H_

#ifdef __ANIMATION_EDITOR
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
#define AnimationEditorScreen_METHODS										\
	State_METHODS;									

// declare the virtual methods which are redefined
#define AnimationEditorScreen_SET_VTABLE(ClassName)							\
	State_SET_VTABLE(ClassName)											\
	__VIRTUAL_SET(ClassName, AnimationEditorScreen, enter);					\
	__VIRTUAL_SET(ClassName, AnimationEditorScreen, execute);				\
	__VIRTUAL_SET(ClassName, AnimationEditorScreen, exit);					\
	__VIRTUAL_SET(ClassName, AnimationEditorScreen, handleMessage);			\


__CLASS(AnimationEditorScreen);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// setup the init focus screen
AnimationEditorScreen AnimationEditorScreen_getInstance(void);

#endif

#endif /*ANIMATION_EDITOR_SCREEN_H_*/
