/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef ENTITY_H_
#define ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Sprite.h>
#include <StateMachine.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Entity_METHODS															\
		Container_METHODS														\
		__VIRTUAL_DEC(getScale);												\
		__VIRTUAL_DEC(isVisible);												\
		__VIRTUAL_DEC(setExtraInfo);											\
		__VIRTUAL_DEC(updateSpritePosition);									\
		__VIRTUAL_DEC(updateSpriteScale);										\
		__VIRTUAL_DEC(getPosition);												\
		__VIRTUAL_DEC(getWidth);												\
		__VIRTUAL_DEC(getHeight);												\
		__VIRTUAL_DEC(getDeep);													\
		__VIRTUAL_DEC(getGap);													\
		__VIRTUAL_DEC(getShapeType);											\
		__VIRTUAL_DEC(moves);													\
		__VIRTUAL_DEC(getPreviousPosition);										\
		__VIRTUAL_DEC(getShape);												\

#define Entity_SET_VTABLE(ClassName)											\
		Container_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Entity, initialTransform);						\
		__VIRTUAL_SET(ClassName, Entity, transform);							\
		__VIRTUAL_SET(ClassName, Entity, handleMessage);						\
		__VIRTUAL_SET(ClassName, Entity, getScale);								\
		__VIRTUAL_SET(ClassName, Entity, isVisible);							\
		__VIRTUAL_SET(ClassName, Entity, setExtraInfo);							\
		__VIRTUAL_SET(ClassName, Entity, updateSpritePosition);					\
		__VIRTUAL_SET(ClassName, Entity, updateSpriteScale);					\
		__VIRTUAL_SET(ClassName, Entity, setLocalPosition);						\
		__VIRTUAL_SET(ClassName, Entity, getPosition);							\
		__VIRTUAL_SET(ClassName, Entity, getWidth);								\
		__VIRTUAL_SET(ClassName, Entity, getHeight);							\
		__VIRTUAL_SET(ClassName, Entity, getDeep);								\
		__VIRTUAL_SET(ClassName, Entity, getGap);								\
		__VIRTUAL_SET(ClassName, Entity, getShapeType);							\
		__VIRTUAL_SET(ClassName, Entity, moves);								\
		__VIRTUAL_SET(ClassName, Entity, getPreviousPosition);					\
		__VIRTUAL_SET(ClassName, Entity, getShape);								\
		__VIRTUAL_SET(ClassName, Entity, suspend);								\
		__VIRTUAL_SET(ClassName, Entity, resume);								\

#define Entity_ATTRIBUTES														\
																				\
	/* it is derivated from*/													\
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

// defines a Entity in ROM memory
typedef struct EntityDefinition
{
	// the class type
	void* allocator;

	// the sprite
	const SpriteDefinition* spritesDefinitions;

} EntityDefinition;

typedef const EntityDefinition EntityROMDef;

// a actor asociated with a position
typedef const struct PositionedEntity
{
	// pointer to the entity definition in ROM
	EntityDefinition* entityDefinition;

	// position in the world
	VBVec3DReal position;
	
	// name
	char* name;

	// the children
	struct PositionedEntity* childrenDefinitions;

	// extra info
	void* extraInfo;

} PositionedEntity;

typedef const PositionedEntity PositionedEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 ID);
void Entity_destructor(Entity this);
Entity Entity_load(const EntityDefinition* entityDefinition, int ID, void* extraInfo);
Entity Entity_loadFromDefinition(const PositionedEntity* positionedEntity, const Transformation* environmentTransform, s16 id);
void Entity_addChildren(Entity this, const PositionedEntity* childrenDefinitions, const Transformation* environmentTransform);
Entity Entity_addChildFromDefinition(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3DReal* position, void* extraInfo);
void Entity_setExtraInfo(Entity this, void* extraInfo);
void Entity_setAnimation(Entity this, void (*animation)(Entity this));
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition);
void Entity_translateSprites(Entity this, int updateSpriteScale, int updateSpritePosition);
void Entity_initialTransform(Entity this, Transformation* environmentTransform);
void Entity_transform(Entity this, Transformation* environmentTransform);
EntityDefinition* Entity_getEntityDefinition(Entity this);
Scale Entity_getScale(Entity this);
void Entity_setLocalPosition(Entity this, VBVec3D position);
VBVec3D Entity_getPosition(Entity this);
VBVec3D Entity_getLocalPosition(Entity this);
int Entity_getMapParallax(Entity this);
void Entity_setCollisionGap(Entity this, int upGap, int downGap, int leftGap, int rightGap);
int Entity_getInGameType(Entity this);
VirtualList Entity_getSprites(Entity this);
bool Entity_handleMessage(Entity this, Telegram telegram);
u16 Entity_getWidth(Entity this);
u16 Entity_getHeight(Entity this);
u16 Entity_getDeep(Entity this);
Gap Entity_getGap(Entity this);
int Entity_getShapeType(Entity this);
int Entity_doesMove(Entity this);
bool Entity_isVisible(Entity this, int pad);
int Entity_updateSpritePosition(Entity this);
int Entity_updateSpriteScale(Entity this);
void Entity_setSpritesDirection(Entity this, int axis, int direction);
bool Entity_moves(Entity this);
const VBVec3D* Entity_getPreviousPosition(Entity this);
void Entity_show(Entity this);
void Entity_hide(Entity this);
void Entity_suspend(Entity this);
void Entity_resume(Entity this);


#endif