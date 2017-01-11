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

#ifndef MANAGED_M_BACKGROUND_H_
#define MANAGED_M_BACKGROUND_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackground.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedMBackground_METHODS(ClassName)															\
	    MBackground_METHODS(ClassName)																	\

#define ManagedMBackground_SET_VTABLE(ClassName)														\
		MBackground_SET_VTABLE(ClassName)																\
	    __VIRTUAL_SET(ClassName, ManagedMBackground, initialTransform);									\
	    __VIRTUAL_SET(ClassName, ManagedMBackground, transform);										\
		__VIRTUAL_SET(ClassName, ManagedMBackground, updateVisualRepresentation);						\
		__VIRTUAL_SET(ClassName, ManagedMBackground, update);											\
		__VIRTUAL_SET(ClassName, ManagedMBackground, passMessage);										\
		__VIRTUAL_SET(ClassName, ManagedMBackground, ready);										    \
		__VIRTUAL_SET(ClassName, ManagedMBackground, suspend);											\
		__VIRTUAL_SET(ClassName, ManagedMBackground, resume);											\

__CLASS(ManagedMBackground);

#define ManagedMBackground_ATTRIBUTES																	\
        /* it is derived from */																		\
        MBackground_ATTRIBUTES																			\
        /* sprites' list */																				\
        VirtualList managedSprites;																		\
        /* previous 2d projected position */															\
        VBVec2D previous2DPosition;																		\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedMBackground, MBackgroundDefinition* definition, s16 id, s16 internalId, const char* const name);

void ManagedMBackground_constructor(ManagedMBackground this, MBackgroundDefinition* definition, s16 id, s16 internalId, const char* const name);
void ManagedMBackground_destructor(ManagedMBackground this);
void ManagedMBackground_initialTransform(ManagedMBackground this, Transformation* environmentTransform, u32 recursive);
void ManagedMBackground_transform(ManagedMBackground this, const Transformation* environmentTransform);
void ManagedMBackground_updateVisualRepresentation(ManagedMBackground this);
void ManagedMBackground_update(ManagedMBackground this, u32 elapsedTime);
int ManagedMBackground_passMessage(ManagedMBackground this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
void ManagedMBackground_ready(ManagedMBackground this, u32 recursive);
void ManagedMBackground_suspend(ManagedMBackground this);
void ManagedMBackground_resume(ManagedMBackground this);


#endif
