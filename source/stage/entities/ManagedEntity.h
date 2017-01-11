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

#ifndef MANAGED_ENTITY_H_
#define MANAGED_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Sprite.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedEntity_METHODS(ClassName)																\
		Entity_METHODS(ClassName)																		\

#define ManagedEntity_SET_VTABLE(ClassName)																\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ManagedEntity, initialTransform);										\
		__VIRTUAL_SET(ClassName, ManagedEntity, transform);												\
		__VIRTUAL_SET(ClassName, ManagedEntity, updateVisualRepresentation);							\
		__VIRTUAL_SET(ClassName, ManagedEntity, passMessage);											\
		__VIRTUAL_SET(ClassName, ManagedEntity, ready);											        \
		__VIRTUAL_SET(ClassName, ManagedEntity, suspend);											    \
		__VIRTUAL_SET(ClassName, ManagedEntity, resume);											    \

#define ManagedEntity_ATTRIBUTES																		\
        /* it is derived from */																		\
        Entity_ATTRIBUTES																				\
        /* sprites' list */																				\
        VirtualList managedSprites;																		\
        /* previous 2d projected position */															\
        VBVec2D previous2DPosition;																		\

__CLASS(ManagedEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef EntityDefinition ManagedEntityDefinition;
typedef const ManagedEntityDefinition ManagedEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedEntity, ManagedEntityDefinition* managedEntityDefinition, s16 id, s16 internalId, const char* const name);

void ManagedEntity_constructor(ManagedEntity this, ManagedEntityDefinition* managedEntityDefinition, s16 id, s16 internalId, const char* const name);
void ManagedEntity_destructor(ManagedEntity this);
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform, u32 recursive);
void ManagedEntity_transform(ManagedEntity this, const Transformation* environmentTransform);
void ManagedEntity_updateVisualRepresentation(ManagedEntity this);
int ManagedEntity_passMessage(ManagedEntity this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
void ManagedEntity_ready(ManagedEntity this, u32 recursive);
void ManagedEntity_suspend(ManagedEntity this);
void ManagedEntity_resume(ManagedEntity this);

#endif
