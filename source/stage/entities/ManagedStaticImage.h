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

#ifndef MANAGED_STATIC_IMAGE_H_
#define MANAGED_STATIC_IMAGE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <RecyclableImage.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedStaticImage_METHODS(ClassName)															\
		RecyclableImage_METHODS(ClassName)																\

#define ManagedStaticImage_SET_VTABLE(ClassName)														\
		RecyclableImage_SET_VTABLE(ClassName)															\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, removeChild);										\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, initialTransform);									\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, transform);										\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, updateVisualRepresentation);						\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, releaseGraphics);									\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, update);											\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, passMessage);										\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, ready);											\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, suspend);											\
		__VIRTUAL_SET(ClassName, ManagedStaticImage, resume);											\

__CLASS(ManagedStaticImage);

#define ManagedStaticImage_ATTRIBUTES																	\
		/* it is derived from */																		\
		RecyclableImage_ATTRIBUTES																		\
		/* sprites' list */																				\
		VirtualList managedSprites;																		\
		/* previous 2d projected position */															\
		VBVec2D previous2DPosition;																		\


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef RecyclableImageDefinition ManagedStaticImageDefinition;

typedef const ManagedStaticImageDefinition ManagedStaticImageROMDef;

//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedStaticImage, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name);

void ManagedStaticImage_constructor(ManagedStaticImage this, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name);
void ManagedStaticImage_destructor(ManagedStaticImage this);
void ManagedStaticImage_removeChild(ManagedStaticImage this, Container child);
void ManagedStaticImage_initialTransform(ManagedStaticImage this, Transformation* environmentTransform, u32 recursive);
void ManagedStaticImage_transform(ManagedStaticImage this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void ManagedStaticImage_updateVisualRepresentation(ManagedStaticImage this);
void ManagedStaticImage_releaseGraphics(ManagedStaticImage this);
void ManagedStaticImage_update(ManagedStaticImage this, u32 elapsedTime);
int ManagedStaticImage_passMessage(ManagedStaticImage this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
void ManagedStaticImage_ready(ManagedStaticImage this, bool recursive);
void ManagedStaticImage_suspend(ManagedStaticImage this);
void ManagedStaticImage_resume(ManagedStaticImage this);


#endif
