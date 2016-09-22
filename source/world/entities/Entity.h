/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef ENTITY_H_
#define ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <EntityFactory.h>
#include <BgmapSprite.h>
#include <ObjectSprite.h>
#include <Telegram.h>

//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_SPRITE_TRANSFORMATION		0x07
#define __UPDATE_SPRITE_POSITION			0x01
#define __UPDATE_SPRITE_SCALE       		0x02
#define __UPDATE_SPRITE_ROTATION       		0x04

#define __EVENT_ENTITY_LOADED	            "entLoad"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Entity_METHODS(ClassName)																		\
		Container_METHODS(ClassName)																	\
		__VIRTUAL_DEC(ClassName, bool, isVisible, int, bool);								            \
		__VIRTUAL_DEC(ClassName, void, setExtraInfo, void*);									        \
		__VIRTUAL_DEC(ClassName, bool, updateSpritePosition);											\
		__VIRTUAL_DEC(ClassName, bool, updateSpriteScale);									            \
		__VIRTUAL_DEC(ClassName, bool, updateSpriteRotation);									        \
		__VIRTUAL_DEC(ClassName, void, initialize, u32);												\
		__VIRTUAL_DEC(ClassName, void, ready, u32);														\
		__VIRTUAL_DEC(ClassName, u32, getAxisForFlipping);												\

#define Entity_SET_VTABLE(ClassName)																	\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Entity, initialTransform);												\
		__VIRTUAL_SET(ClassName, Entity, transform);													\
		__VIRTUAL_SET(ClassName, Entity, updateVisualRepresentation);									\
		__VIRTUAL_SET(ClassName, Entity, setLocalPosition);												\
		__VIRTUAL_SET(ClassName, Entity, handleMessage);												\
		__VIRTUAL_SET(ClassName, Entity, isVisible);													\
		__VIRTUAL_SET(ClassName, Entity, setExtraInfo);													\
		__VIRTUAL_SET(ClassName, Entity, updateSpritePosition);											\
		__VIRTUAL_SET(ClassName, Entity, updateSpriteScale);									        \
		__VIRTUAL_SET(ClassName, Entity, updateSpriteRotation);									        \
		__VIRTUAL_SET(ClassName, Entity, getPosition);													\
		__VIRTUAL_SET(ClassName, Entity, getWidth);														\
		__VIRTUAL_SET(ClassName, Entity, getHeight);													\
		__VIRTUAL_SET(ClassName, Entity, getDepth);														\
		__VIRTUAL_SET(ClassName, Entity, getGap);														\
		__VIRTUAL_SET(ClassName, Entity, getShape);														\
		__VIRTUAL_SET(ClassName, Entity, suspend);														\
		__VIRTUAL_SET(ClassName, Entity, resume);														\
		__VIRTUAL_SET(ClassName, Entity, canMoveOverAxis);												\
		__VIRTUAL_SET(ClassName, Entity, initialize);													\
		__VIRTUAL_SET(ClassName, Entity, ready);														\
		__VIRTUAL_SET(ClassName, Entity, getAxisForFlipping);											\
		__VIRTUAL_SET(ClassName, Entity, hide);											                \

#define Entity_ATTRIBUTES																				\
        /* it is derived from */																		\
        Container_ATTRIBUTES																			\
        /* entity factory */																            \
        EntityFactory entityFactory;                                                                    \
        /* sprites' list */																				\
        VirtualList sprites;																			\
        /* shape for collision detection */																\
        Shape shape;																					\
        /* shape for collision detection */																\
        Size size;																						\
        /* entity's definition */																		\
        EntityDefinition *entityDefinition;																\
        /* entity's definition */																		\
        VBVec3D* centerDisplacement;																	\
        /* flag to update sprites' attributes */														\
        bool updateSprites;																				\

	__CLASS(Entity);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Entity, EntityDefinition* entityDefinition, s16 id, const char* const name);

void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id, const char* const name);
void Entity_destructor(Entity this);
SmallRightCuboid Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition);
VBVec3D* Entity_calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, VBVec3D environmentPosition, const char* childName);
Entity Entity_instantiate(const EntityDefinition* const entityDefinition, int id, const char* const name, void* extraInfo);
Entity Entity_loadEntity(const PositionedEntity* const positionedEntity, s16 id);
Entity Entity_loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 id);
void Entity_initialize(Entity this, u32 recursive);
void Entity_ready(Entity this, u32 recursive);
void Entity_addChildEntities(Entity this, const PositionedEntity* childrenDefinitions);
void Entity_addChildEntitiesDeferred(Entity this, const PositionedEntity* childrenDefinitions);
u32 Entity_areAllChildrenInstantiated(Entity this);
u32 Entity_areAllChildrenInitialized(Entity this);
u32 Entity_areAllChildrenTransformed(Entity this);
u32 Entity_areAllChildrenReady(Entity this);
Entity Entity_addChildEntity(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3D* position, void* extraInfo);
void Entity_setExtraInfo(Entity this, void* extraInfo);
void Entity_setAnimation(Entity this, void (*animation)(Entity this));
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition);
void Entity_initialTransform(Entity this, Transformation* environmentTransform, u32 recursive);
void Entity_transform(Entity this, const Transformation* environmentTransform);
void Entity_updateVisualRepresentation(Entity this);
void Entity_setLocalPosition(Entity this, const VBVec3D* position);
EntityDefinition* Entity_getEntityDefinition(Entity this);
const VBVec3D* Entity_getPosition(Entity this);
int Entity_getMapParallax(Entity this);
void Entity_setCollisionGap(Entity this, int upGap, int downGap, int leftGap, int rightGap);
VirtualList Entity_getSprites(Entity this);
bool Entity_handleMessage(Entity this, Telegram telegram);
int Entity_getWidth(Entity this);
int Entity_getHeight(Entity this);
int Entity_getDepth(Entity this);
Gap Entity_getGap(Entity this);
bool Entity_isVisible(Entity this, int pad, bool recursive);
bool Entity_updateSpritePosition(Entity this);
bool Entity_updateSpriteScale(Entity this);
bool Entity_updateSpriteRotation(Entity this);
void Entity_setSpritesDirection(Entity this, int axis, int direction);
Shape Entity_getShape(Entity this);
void Entity_show(Entity this);
void Entity_hide(Entity this);
void Entity_suspend(Entity this);
void Entity_resume(Entity this);
int Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration);
u32 Entity_getAxisForFlipping(Entity this);

#endif
