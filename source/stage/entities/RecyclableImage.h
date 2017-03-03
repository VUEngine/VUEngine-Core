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

#ifndef RECYCLABLE_IMAGE_H_
#define RECYCLABLE_IMAGE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define RecyclableImage_METHODS(ClassName)																\
		Entity_METHODS(ClassName)																		\

#define RecyclableImage_SET_VTABLE(ClassName)															\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, RecyclableImage, initialize);											\
		__VIRTUAL_SET(ClassName, RecyclableImage, suspend);												\
		__VIRTUAL_SET(ClassName, RecyclableImage, resume);												\
		__VIRTUAL_SET(ClassName, RecyclableImage, setupGraphics);										\
		__VIRTUAL_SET(ClassName, RecyclableImage, releaseGraphics);										\

#define RecyclableImage_ATTRIBUTES																		\
		/* super's attributes */																		\
		Entity_ATTRIBUTES																				\
		/* ROM definition */																			\
		RecyclableImageDefinition* recyclableImageDefinition;											\

__CLASS(RecyclableImage);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a Scrolling background
typedef EntityDefinition RecyclableImageDefinition;

// defines a Scrolling background in ROM memory
typedef const EntityDefinition RecyclableImageROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(RecyclableImage, RecyclableImageDefinition* recyclableImageDefinition, s16 id, s16 internalId, const char* const name);

void RecyclableImage_constructor(RecyclableImage this, RecyclableImageDefinition* recyclableImageDefinition, s16 id, s16 internalId, const char* const name);
void RecyclableImage_destructor(RecyclableImage this);
void RecyclableImage_setDefinition(RecyclableImage this, RecyclableImageDefinition* recyclableImageDefinition);
void RecyclableImage_suspend(RecyclableImage this);
void RecyclableImage_resume(RecyclableImage this);
void RecyclableImage_initialize(RecyclableImage this, bool recursive);
void RecyclableImage_setupGraphics(RecyclableImage this);
void RecyclableImage_releaseGraphics(RecyclableImage this);

#endif
