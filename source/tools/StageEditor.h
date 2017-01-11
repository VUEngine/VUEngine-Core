/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
// 											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define StageEditor_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define StageEditor_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, StageEditor, handleMessage);											\

// declare a StageEditor
__CLASS(StageEditor);

/**
 * For level editing
 *
 * @memberof 		StageEditor
 */
typedef struct UserObject
{
    /// Pointer to EntityDefinition
	EntityDefinition* entityDefinition;
	/// Name of the Entity
	char* name;

} UserObject;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

StageEditor StageEditor_getInstance();

void StageEditor_destructor(StageEditor this);

bool StageEditor_handleMessage(StageEditor this, Telegram telegram);
void StageEditor_start(StageEditor this, GameState gameState);
void StageEditor_stop(StageEditor this);
void StageEditor_update(StageEditor this);


#endif

#endif
