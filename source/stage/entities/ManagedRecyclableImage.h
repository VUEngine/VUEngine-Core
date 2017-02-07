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

#ifndef MANAGED_RECYCLABLE_IMAGE_H_
#define MANAGED_RECYCLABLE_IMAGE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <RecyclableImage.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedRecyclableImage_METHODS(ClassName)														\
		RecyclableImage_METHODS(ClassName)																\

#define ManagedRecyclableImage_SET_VTABLE(ClassName)													\
		RecyclableImage_SET_VTABLE(ClassName)															\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, initialTransform);								\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, transform);									\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, updateVisualRepresentation);					\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, update);										\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, passMessage);									\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, ready);										\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, suspend);										\
		__VIRTUAL_SET(ClassName, ManagedRecyclableImage, resume);										\

__CLASS(ManagedRecyclableImage);

#define ManagedRecyclableImage_ATTRIBUTES																\
		/* it is derived from */																		\
		RecyclableImage_ATTRIBUTES																		\
		/* sprites' list */																				\
		VirtualList managedSprites;																		\
		/* previous 2d projected position */															\
		VBVec2D previous2DPosition;																		\


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedRecyclableImage, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name);

void ManagedRecyclableImage_constructor(ManagedRecyclableImage this, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name);
void ManagedRecyclableImage_destructor(ManagedRecyclableImage this);
void ManagedRecyclableImage_initialTransform(ManagedRecyclableImage this, Transformation* environmentTransform, u32 recursive);
void ManagedRecyclableImage_transform(ManagedRecyclableImage this, const Transformation* environmentTransform);
void ManagedRecyclableImage_updateVisualRepresentation(ManagedRecyclableImage this);
void ManagedRecyclableImage_update(ManagedRecyclableImage this, u32 elapsedTime);
int ManagedRecyclableImage_passMessage(ManagedRecyclableImage this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
void ManagedRecyclableImage_ready(ManagedRecyclableImage this, u32 recursive);
void ManagedRecyclableImage_suspend(ManagedRecyclableImage this);
void ManagedRecyclableImage_resume(ManagedRecyclableImage this);


#endif
