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

#ifndef CHARSET_MANAGER_H_
#define CHARSET_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define CharSetManager_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define CharSetManager_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(CharSetManager);


//---------------------------------------------------------------------------------------------------------
//											PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

CharSetManager CharSetManager_getInstance();

void CharSetManager_destructor(CharSetManager this);
void CharSetManager_reset(CharSetManager this);
CharSet CharSetManager_getCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);
void CharSetManager_releaseCharSet(CharSetManager this, CharSet charSet);
void CharSetManager_defragment(CharSetManager this);
void CharSetManager_defragmentProgressively(CharSetManager this);
int CharSetManager_getTotalUsedChars(CharSetManager this);
int CharSetManager_getTotalFreeChars(CharSetManager this);
int CharSetManager_getTotalCharSets(CharSetManager this);
void CharSetManager_print(CharSetManager this, int x, int y);


#endif
