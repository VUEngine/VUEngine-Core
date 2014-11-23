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

#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include <Entity.h>

// for debugging
typedef struct UserObject {
	
	char* name;
	EntityDefinition* entityDefinition;
	
}UserObject;


#ifdef __LEVEL_EDITOR

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define LevelEditor_METHODS													\
		Object_METHODS														\


// declare the virtual methods which are redefined
#define LevelEditor_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)										\
		__VIRTUAL_SET(ClassName, LevelEditor, handleMessage);				\


// declare a LevelEditor
__CLASS(LevelEditor);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
LevelEditor LevelEditor_getInstance();

// class's destructor
void LevelEditor_destructor(LevelEditor this);

// update
void LevelEditor_update(LevelEditor this);

// start level editor
void LevelEditor_start(LevelEditor this);

// stop level editor
void LevelEditor_stop(LevelEditor this);

// process a telegram
int LevelEditor_handleMessage(LevelEditor this, Telegram telegram);

#endif

#endif /*CLOCK_H_*/
