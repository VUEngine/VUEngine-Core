/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef STAGE_EDITOR_H_
#define STAGE_EDITOR_H_


#ifdef __STAGE_EDITOR


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Entity.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define StageEditor_METHODS														\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define StageEditor_SET_VTABLE(ClassName)										\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, StageEditor, handleMessage);					\

// declare a StageEditor
__CLASS(StageEditor);

// for level editing
typedef struct UserObject
{
	EntityDefinition* entityDefinition;
	char* name;

} UserObject;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

StageEditor StageEditor_getInstance();

void StageEditor_destructor(StageEditor this);
void StageEditor_update(StageEditor this);
void StageEditor_start(StageEditor this, GameState gameState);
void StageEditor_stop(StageEditor this);
bool StageEditor_handleMessage(StageEditor this, Telegram telegram);

#endif


#endif