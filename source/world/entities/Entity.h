/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Container.h>
#include <Sprite.h>
#include <StateMachine.h>
#include <Telegram.h>



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Entity_METHODS								\
		Container_METHODS							\
		__VIRTUAL_DEC(getScale);					\
		__VIRTUAL_DEC(getInGameState);				\
		__VIRTUAL_DEC(isVisible);					\
		__VIRTUAL_DEC(setExtraInfo);				\
		__VIRTUAL_DEC(updateSpritePosition);		\
		__VIRTUAL_DEC(updateSpriteScale);			\
	

#define Entity_SET_VTABLE(ClassName)								\
		Container_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Entity, render);					\
		__VIRTUAL_SET(ClassName, Entity, handleMessage);			\
		__VIRTUAL_SET(ClassName, Entity, getScale);					\
		__VIRTUAL_SET(ClassName, Entity, getInGameState);			\
		__VIRTUAL_SET(ClassName, Entity, isVisible);				\
		__VIRTUAL_SET(ClassName, Entity, setExtraInfo);				\
		__VIRTUAL_SET(ClassName, Entity, updateSpritePosition);		\
		__VIRTUAL_SET(ClassName, Entity, updateSpriteScale);		\
	

// A Entity which represent a generic Container inside a Stage

#define Entity_ATTRIBUTES							\
													\
	/* it is derivated from*/						\
	Container_ATTRIBUTES							\
													\
	/* sprites' list */								\
	VirtualList sprites;

__CLASS(Entity);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// sprites' list
typedef struct SpritesDescription{
	
	// animation functions
	SpriteDefinition spritesDefinitions[__MAX_SPRITES_PER_ENTITY];
	
}SpritesDescription;

typedef const SpritesDescription SpritesDescriptionROMDef;

// defines a Entity in ROM memory
typedef struct EntityDefinition{

	// the class type
	void* allocator;
	
	// the sprite
	const SpriteDefinition* spritesDefinitions;
	
	// number of sprites
	int numberOfSprites;
	
}EntityDefinition;

typedef const EntityDefinition EntityROMDef;

#define __SPRITE_ARRAY(SpritesDefintionArray) SpritesDefintionArray, sizeof(SpritesDefintionArray) / sizeof(SpriteDefinition)

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, int ID);

// class's destructor
void Entity_destructor(Entity this);

// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo);

//set class's animation
void Entity_setAnimation(Entity this, void (*animation)(Entity this));

//get map's type
int Entity_getMapType(Entity this);

//retrieve class's map's mode
int Entity_getMapMode(Entity this);

//grite Entity to graphic memory
void Entity_write(Entity this);

// allocate a write in graphic memory again
void Entity_resetMemoryState(Entity this, int worldLayer);

// add sprite
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition);

//render class
void Entity_render(Entity this, Transformation environmentTransform);

//retrieve class's scale
Scale Entity_getScale(Entity this);

// retrieve global position
VBVec3D Entity_getPosition(Entity this);

// retrieve local position
VBVec3D Entity_getLocalPosition(Entity this);

//retrieve class's map's paralax
int Entity_getMapParallax(Entity this);

// retrieve Entity's friction
float Entity_getFrictionFactor(Entity this);

// set graphical gap
void Entity_setCollisionGap(Entity this, int upGap, int downGap, int leftGap, int rightGap);

// retrieve in game type
int Entity_getInGameType(Entity this);

// retrieve sprite
VirtualList Entity_getSprites(Entity this);

// process a telegram
int Entity_handleMessage(Entity this, Telegram telegram);

// get width
int Entity_getWidth(Entity this);

// get height
int Entity_getHeight(Entity this);

// whether it is visible
int Entity_isVisible(Entity this, int pad);

// determine if the entity is outside the game
int Entity_isOutsideGame(Entity this);

// retrieve state when unloading the entity 
int Entity_getInGameState(Entity this);

// create an entity in gameengine's memory
Entity Entity_load(EntityDefinition* entityDefinition, VBVec3D* position, int ID, void* extraInfo);

// check if must update sprite's position
int Entity_updateSpritePosition(Entity this);

// check if must update sprite's scale
int Entity_updateSpriteScale(Entity this);

// set the direction
void Entity_setSpritesDirection(Entity this, int axis, int direction);


#endif /*ENTITY_H_*/
