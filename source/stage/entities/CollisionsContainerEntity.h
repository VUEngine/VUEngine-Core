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

#ifndef COLLISIONS_CONTAINER_ENTITY_H_
#define COLLISIONS_CONTAINER_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define CollisionsContainerEntity_METHODS(ClassName)													\
		Entity_METHODS(ClassName)																		\

#define CollisionsContainerEntity_SET_VTABLE(ClassName)													\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, update);									\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, transform);									\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, updateVisualRepresentation);				\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, handleMessage);								\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, initialize);										\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, ready);										\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, handlePropagatedMessage);					\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, passMessage);								\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, isVisible);									\

#define CollisionsContainerEntity_ATTRIBUTES															\
		Entity_ATTRIBUTES																				\

__CLASS(CollisionsContainerEntity);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef EntityDefinition CollisionsContainerEntityDefinition;
typedef const CollisionsContainerEntityDefinition CollisionsContainerEntityROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionsContainerEntity, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name);

void CollisionsContainerEntity_constructor(CollisionsContainerEntity this, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name);
void CollisionsContainerEntity_destructor(CollisionsContainerEntity this);
void CollisionsContainerEntity_initialize(CollisionsContainerEntity this, bool recursive);
void CollisionsContainerEntity_ready(CollisionsContainerEntity this, bool recursive);
void CollisionsContainerEntity_update(CollisionsContainerEntity this, u32 elapsedTime);
void CollisionsContainerEntity_transform(CollisionsContainerEntity this, const Transformation* environmentTransform);
void CollisionsContainerEntity_updateVisualRepresentation(CollisionsContainerEntity this);
bool CollisionsContainerEntity_handleMessage(CollisionsContainerEntity this, Telegram telegram);
bool CollisionsContainerEntity_handlePropagatedMessage(CollisionsContainerEntity this, int message);
int CollisionsContainerEntity_passMessage(CollisionsContainerEntity this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
bool CollisionsContainerEntity_isVisible(CollisionsContainerEntity this, int pad, bool recursive);


#endif
