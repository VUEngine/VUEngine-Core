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
#include <BgmapSprite.h>
#include <ObjectSprite.h>
#include <StateMachine.h>
#include <Telegram.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Entity_METHODS															\
		Container_METHODS														\
		__VIRTUAL_DEC(isVisible);												\
		__VIRTUAL_DEC(setExtraInfo);											\
		__VIRTUAL_DEC(updateSpritePosition);									\
		__VIRTUAL_DEC(updateSpriteTransformations);								\
		__VIRTUAL_DEC(initialize);												\
		__VIRTUAL_DEC(ready);													\

#define Entity_SET_VTABLE(ClassName)											\
		Container_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Entity, initialTransform);						\
		__VIRTUAL_SET(ClassName, Entity, transform);							\
		__VIRTUAL_SET(ClassName, Entity, handleMessage);						\
		__VIRTUAL_SET(ClassName, Entity, isVisible);							\
		__VIRTUAL_SET(ClassName, Entity, setExtraInfo);							\
		__VIRTUAL_SET(ClassName, Entity, updateSpritePosition);					\
		__VIRTUAL_SET(ClassName, Entity, updateSpriteTransformations);			\
		__VIRTUAL_SET(ClassName, Entity, getPosition);							\
		__VIRTUAL_SET(ClassName, Entity, getWidth);								\
		__VIRTUAL_SET(ClassName, Entity, getHeight);							\
		__VIRTUAL_SET(ClassName, Entity, getDepth);								\
		__VIRTUAL_SET(ClassName, Entity, getGap);								\
		__VIRTUAL_SET(ClassName, Entity, getShape);								\
		__VIRTUAL_SET(ClassName, Entity, suspend);								\
		__VIRTUAL_SET(ClassName, Entity, resume);								\
		__VIRTUAL_SET(ClassName, Entity, canMoveOverAxis);						\
		__VIRTUAL_SET(ClassName, Entity, initialize);							\
		__VIRTUAL_SET(ClassName, Entity, ready);								\

#define Entity_ATTRIBUTES														\
																				\
	/* it is derivated from */													\
	Container_ATTRIBUTES														\
																				\
	/* sprites' list */															\
	VirtualList sprites;														\
																				\
	/* shape for collision detection */											\
	Shape shape;																\
																				\
	/* shape for collision detection */											\
	Size size;																	\
																				\
	/* entity's definition */													\
	EntityDefinition *entityDefinition;											\

__CLASS(Entity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines an entity in ROM memory
typedef struct EntityDefinition
{
	// the class type
	void* allocator;

	// the sprite
	const SpriteDefinition** spritesDefinitions;

} EntityDefinition;

typedef const EntityDefinition EntityROMDef;

// an actor associated with a position
typedef const struct PositionedEntity
{
	// pointer to the entity definition in ROM
	EntityDefinition* entityDefinition;

	// position in the world
	VBVec3D position;
	
	// name
	char* name;

	// the children
	struct PositionedEntity* childrenDefinitions;

	// extra info
	void* extraInfo;
	
	// force load
	bool loadRegardlessOfPosition;

} PositionedEntity;

typedef const PositionedEntity PositionedEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id, const char* const name);
void Entity_destructor(Entity this);
SmallRightcuboid Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition);
Entity Entity_load(const EntityDefinition* const entityDefinition, int id, const char* const name, void* extraInfo);
Entity Entity_loadFromDefinition(const PositionedEntity* const positionedEntity, s16 id);
Entity Entity_loadFromDefinitionWithoutInitilization(const PositionedEntity* const positionedEntity, s16 id);
void Entity_initialize(Entity this);
void Entity_ready(Entity this);
void Entity_addChildren(Entity this, const PositionedEntity* childrenDefinitions);
void Entity_addChildrenWithoutInitilization(Entity this, const PositionedEntity* childrenDefinitions);
Entity Entity_addChildFromDefinition(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3D* position, void* extraInfo);
void Entity_setExtraInfo(Entity this, void* extraInfo);
void Entity_setAnimation(Entity this, void (*animation)(Entity this));
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition);
void Entity_translateSprites(Entity this, bool updateSpriteTransformations, bool updateSpritePosition);
void Entity_initialTransform(Entity this, Transformation* environmentTransform);
void Entity_transform(Entity this, const Transformation* environmentTransform);
EntityDefinition* Entity_getEntityDefinition(Entity this);
const VBVec3D* Entity_getPosition(Entity this);
int Entity_getMapParallax(Entity this);
void Entity_setCollisionGap(Entity this, int upGap, int downGap, int leftGap, int rightGap);
int Entity_getInGameType(Entity this);
VirtualList Entity_getSprites(Entity this);
bool Entity_handleMessage(Entity this, Telegram telegram);
u16 Entity_getWidth(Entity this);
u16 Entity_getHeight(Entity this);
u16 Entity_getDepth(Entity this);
Gap Entity_getGap(Entity this);
bool Entity_isVisible(Entity this, int pad);
bool Entity_updateSpritePosition(Entity this);
bool Entity_updateSpriteTransformations(Entity this);
void Entity_setSpritesDirection(Entity this, int axis, int direction);
void Entity_show(Entity this);
void Entity_hide(Entity this);
void Entity_suspend(Entity this);
void Entity_resume(Entity this);
bool Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration);


#endif