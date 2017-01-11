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

#ifndef M_BACKGROUND_H_
#define M_BACKGROUND_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define MBackground_METHODS(ClassName)																	\
		Entity_METHODS(ClassName)																		\

#define MBackground_SET_VTABLE(ClassName)																\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, MBackground, suspend);													\
		__VIRTUAL_SET(ClassName, MBackground, resume);													\

#define MBackground_ATTRIBUTES																			\
        /* super's attributes */																		\
        Entity_ATTRIBUTES																				\
        /* ROM definition */																			\
        MBackgroundDefinition* mBackgroundDefinition;													\

__CLASS(MBackground);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a Scrolling background
typedef EntityDefinition MBackgroundDefinition;

// defines a Scrolling background in ROM memory
typedef const EntityDefinition MBackgroundROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(MBackground, MBackgroundDefinition* mBackgroundDefinition, s16 id, s16 internalId, const char* const name);

void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id, s16 internalId, const char* const name);
void MBackground_destructor(MBackground this);
void MBackground_setDefinition(MBackground this, MBackgroundDefinition* mBackgroundDefinition);
void MBackground_suspend(MBackground this);
void MBackground_resume(MBackground this);


#endif
