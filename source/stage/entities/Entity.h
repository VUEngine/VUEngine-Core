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

#ifndef ENTITY_H_
#define ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <EntityFactory.h>
#include <BgmapSprite.h>
#include <ObjectSprite.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_SPRITE_TRANSFORMATION		0x07
#define __UPDATE_SPRITE_POSITION			0x01
#define __UPDATE_SPRITE_SCALE				0x02
#define __UPDATE_SPRITE_ROTATION			0x04


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Entity_METHODS(ClassName)																		\
		Container_METHODS(ClassName)																	\
		__VIRTUAL_DEC(ClassName, bool, isVisible, int, bool);											\
		__VIRTUAL_DEC(ClassName, void, setExtraInfo, void*);											\
		__VIRTUAL_DEC(ClassName, void, initialize, bool);														\
		__VIRTUAL_DEC(ClassName, void, ready, bool);														\
		__VIRTUAL_DEC(ClassName, u32, getAxisForFlipping);												\

#define Entity_SET_VTABLE(ClassName)																	\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Entity, initialTransform);												\
		__VIRTUAL_SET(ClassName, Entity, transform);													\
		__VIRTUAL_SET(ClassName, Entity, setupGraphics);												\
		__VIRTUAL_SET(ClassName, Entity, releaseGraphics);												\
		__VIRTUAL_SET(ClassName, Entity, updateVisualRepresentation);									\
		__VIRTUAL_SET(ClassName, Entity, setLocalPosition);												\
		__VIRTUAL_SET(ClassName, Entity, handleMessage);												\
		__VIRTUAL_SET(ClassName, Entity, isVisible);													\
		__VIRTUAL_SET(ClassName, Entity, setExtraInfo);													\
		__VIRTUAL_SET(ClassName, Entity, getPosition);													\
		__VIRTUAL_SET(ClassName, Entity, getWidth);														\
		__VIRTUAL_SET(ClassName, Entity, getHeight);													\
		__VIRTUAL_SET(ClassName, Entity, getDepth);														\
		__VIRTUAL_SET(ClassName, Entity, getGap);														\
		__VIRTUAL_SET(ClassName, Entity, getShape);														\
		__VIRTUAL_SET(ClassName, Entity, suspend);														\
		__VIRTUAL_SET(ClassName, Entity, resume);														\
		__VIRTUAL_SET(ClassName, Entity, canMoveOverAxis);												\
		__VIRTUAL_SET(ClassName, Entity, initialize);														\
		__VIRTUAL_SET(ClassName, Entity, ready);														\
		__VIRTUAL_SET(ClassName, Entity, getAxisForFlipping);											\
		__VIRTUAL_SET(ClassName, Entity, hide);															\

#define Entity_ATTRIBUTES																				\
		Container_ATTRIBUTES																			\
		/**
		 * @var EntityFactory		entityFactory
		 * @brief					Entity factory
		 * @memberof				Entity
		 */ 																							\
		EntityFactory entityFactory;																	\
		/**
		 * @var VirtualList		 	sprites
		 * @brief					Sprites list
		 * @memberof				Entity
		 */ 																							\
		VirtualList sprites;																			\
		/**
		 * @var Shape				shape
		 * @brief					Shape for collision detection
		 * @memberof				Entity
		 */ 																							\
		Shape shape;																					\
		/**
		 * @var Size				size
		 * @brief					Size
		 * @memberof				Entity
		 */ 																							\
		Size size;																						\
		/**
		 * @var EntityDefinition*	entityDefinition
		 * @brief					Entity's definition
		 * @memberof				Entity
		 */ 																							\
		EntityDefinition* entityDefinition;																\
		/**
		 * @var VBVec3D*			centerDisplacement
		 * @brief					Center displacement
		 * @memberof				Entity
		 */ 																							\
		VBVec3D* centerDisplacement;																	\
		/**
		 * @var s16				 	id
		 * @brief					Entity's id, set by the user
		 * @memberof				Entity
		 */ 																							\
		s16 id;																							\
		/**
		 * @var s16				 	internalId
		 * @brief					Entity's internal id, set by the engine
		 * @memberof				Entity
		 */ 																							\
		s16 internalId;																					\
		/**
		 * @var bool				updateSprites
		 * @brief					Flag to update sprites' attributes
		 * @memberof				Entity
		 */ 																							\
		bool updateSprites;																				\

__CLASS(Entity);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Entity, EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name);

void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name);
void Entity_destructor(Entity this);

void Entity_addChildEntities(Entity this, const PositionedEntity* childrenDefinitions);
void Entity_addChildEntitiesDeferred(Entity this, const PositionedEntity* childrenDefinitions);
bool Entity_addSprite(Entity this, int spriteDefinitionIndex);
Entity Entity_addChildEntity(Entity this, const EntityDefinition* entityDefinition, int internalId, const char* name, const VBVec3D* position, void* extraInfo);
u32 Entity_areAllChildrenInstantiated(Entity this);
u32 Entity_areAllChildrenInitialized(Entity this);
u32 Entity_areAllChildrenTransformed(Entity this);
u32 Entity_areAllChildrenReady(Entity this);
VBVec3D* Entity_calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, VBVec3D environmentPosition, const char* childName);
int Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration);
u32 Entity_getAxisForFlipping(Entity this);
Entity Entity_getChildById(Entity this, s16 id);
int Entity_getDepth(Entity this);
EntityDefinition* Entity_getEntityDefinition(Entity this);
Gap Entity_getGap(Entity this);
int Entity_getHeight(Entity this);
int Entity_getMapParallax(Entity this);
s16 Entity_getId(Entity this);
s16 Entity_getInternalId(Entity this);
const VBVec3D* Entity_getPosition(Entity this);
Shape Entity_getShape(Entity this);
VirtualList Entity_getSprites(Entity this);
SmallRightCuboid Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition);
int Entity_getWidth(Entity this);
bool Entity_handleMessage(Entity this, Telegram telegram);
void Entity_hide(Entity this);
void Entity_initialTransform(Entity this, Transformation* environmentTransform, u32 recursive);
Entity Entity_instantiate(const EntityDefinition* const entityDefinition, s16 id, s16 internalId, const char* const name, void* extraInfo);
bool Entity_isVisible(Entity this, int pad, bool recursive);
Entity Entity_loadEntity(const PositionedEntity* const positionedEntity, s16 internalId);
Entity Entity_loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 internalId);
void Entity_initialize(Entity this, bool recursive);
void Entity_ready(Entity this, bool recursive);
void Entity_resume(Entity this);
void Entity_setupGraphics(Entity this);
void Entity_releaseGraphics(Entity this);
void Entity_setAnimation(Entity this, void (*animation)(Entity this));
void Entity_setCollisionGap(Entity this, int upGap, int downGap, int leftGap, int rightGap);
void Entity_setDefinition(Entity this, EntityDefinition* entityDefinition);
void Entity_setExtraInfo(Entity this, void* extraInfo);
void Entity_setLocalPosition(Entity this, const VBVec3D* position);
void Entity_setSpritesDirection(Entity this, int axis, int direction);
void Entity_show(Entity this);
void Entity_suspend(Entity this);
void Entity_transform(Entity this, const Transformation* environmentTransform);
bool Entity_updateSpritePosition(Entity this);
bool Entity_updateSpriteRotation(Entity this);
bool Entity_updateSpriteScale(Entity this);
void Entity_updateVisualRepresentation(Entity this);


#endif
