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

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <AnimatedSprite.h>
#include <Shape.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Entity
__CLASS_DEFINITION(Entity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
const extern VBVec3D* _screenDisplacement;

// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions, int numberOfSprites);

// set sprites' visual properties
static void Entity_translateSprites(Entity this, int updateSpriteScale, int updateSpritePosition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 ID)
{
	ASSERT(this, "Entity::constructor: null this");

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Entity);

	// construct base Container
	__CONSTRUCT_BASE(Container, __ARGUMENTS(ID));

	// save definition
	this->entityDefinition = entityDefinition;

	/* the sprite must be initializated in the derivated class */
	this->sprites = NULL;

	this->shape = NULL;

	this->size.x = 0;
	this->size.y = 0;
	this->size.z = 0;

	// initialize sprites
	if (entityDefinition)
	{
		Entity_addSprites(this, entityDefinition->spritesDefinitions, entityDefinition->numberOfSprites);
	}
}

// class's destructor
void Entity_destructor(Entity this)
{
	ASSERT(this, "Entity::destructor: null this");

	// better to do it here than forget in other classes
	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);

	this->shape = NULL;

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for (; node ; node = VirtualNode_getNext(node))
	    {
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			__DELETE(sprite);
		}

		// delete the sprites
		__DELETE(this->sprites);

		this->sprites = NULL;
	}

	// destroy the super Container
	__DESTROY_BASE(Container);
}

// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo)
{
	ASSERT(this, "Entity::setExtraInfo: null this");
}

// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions, int numberOfSprites)
{
	ASSERT(this, "Entity::addSprites: null this");

	if (spritesDefinitions)
	{
		int i = numberOfSprites;

		//go through n sprites in entity's definition
		for (; i-- && i < __MAX_SPRITES_PER_ENTITY;)
	    {
			Entity_addSprite(this, &spritesDefinitions[i]);
		}
	}
}

// add sprite
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition)
{
	ASSERT(this, "Entity::addSprite: null this");

	Sprite sprite = NULL;

	switch (spriteDefinition->textureDefinition->charGroupDefinition.allocationType)
	{
		case __ANIMATED:
		case __ANIMATED_SHARED:

			// create the animated sprite
			sprite = (Sprite)__NEW(AnimatedSprite, __ARGUMENTS((void*)this, spriteDefinition));

			break;

		default:

			// create the sprite
			sprite = __NEW(Sprite, __ARGUMENTS(spriteDefinition));

			break;
	}

	if (sprite)
	{
		if (!this->sprites)
	    {
			this->sprites = __NEW(VirtualList);
		}

		VirtualList_pushBack(this->sprites, (void*)sprite);
	}
}

// transform sprites
static void Entity_translateSprites(Entity this, int updateSpriteScale, int updateSpritePosition)
{
	ASSERT(this, "Entity::transform: null this");
	ASSERT(this->sprites, "Entity::transform: null sprites");

	VirtualNode node = VirtualList_begin(this->sprites);

	// move each child to a temporary list
	for (; node ; node = VirtualNode_getNext(node))
	{
		Sprite sprite = (Sprite)VirtualNode_getData(node);

		// update scale if needed
		if (updateSpriteScale)
	    {
			// calculate the scale
			Sprite_calculateScale(sprite, this->transform.globalPosition.z);

			// scale the sprite
			Sprite_scale(sprite);

			// calculate sprite's parallax
			Sprite_calculateParallax(sprite, this->transform.globalPosition.z);

			// reset size so it is recalculated
			this->size.x = this->size.y = this->size.z = 0;
		}

		//if screen is moving
		if (updateSpritePosition)
	    {
			//update sprite's 2D position
			Sprite_setPosition(sprite, &this->transform.globalPosition);
		}
	}
}

// draw class
void Entity_initialTransform(Entity this, Transformation* environmentTransform)
{
	ASSERT(this, "InGameEntity::transform: null this");

	this->invalidateGlobalPosition.x = true;
	this->invalidateGlobalPosition.y = true;
	this->invalidateGlobalPosition.z = true;

	// call base
	int updateSpritePosition = __VIRTUAL_CALL(int, Entity, updateSpritePosition, this);
	int updateSpriteScale = __VIRTUAL_CALL(int, Entity, updateSpriteScale, this);

	// call base class's transform method
	Container_initialTransform((Container)this, environmentTransform);

	Entity_translateSprites(this, updateSpriteScale, updateSpritePosition);

	if (this->shape)
	{
		// setup shape
		__VIRTUAL_CALL(void, Shape, setup, this->shape);

		if (__VIRTUAL_CALL(int, Entity, moves, this))
	    {
			__VIRTUAL_CALL(void, Shape, positione, this->shape);
		}
	}
}

// transform class
void Entity_transform(Entity this, Transformation* environmentTransform)
{
	ASSERT(this, "Entity::transform: null this");

	int updateSpritePosition = __VIRTUAL_CALL(int, Entity, updateSpritePosition, this);
	int updateSpriteScale = __VIRTUAL_CALL(int, Entity, updateSpriteScale, this);

	// call base class's transform method
	Container_transform((Container)this, environmentTransform);

	Entity_translateSprites(this, updateSpriteScale, updateSpritePosition);
}

// retrieve EntityDefinition
EntityDefinition* Entity_getEntityDefinition(Entity this)
{
	ASSERT(this, "Entity::getEntityDefinition: null this");

	return this->entityDefinition;
}


// retrieve class's scale
Scale Entity_getScale(Entity this)
{
	ASSERT(this, "Entity::getScale: null this");
	ASSERT(this->sprites, "Entity::getScale: null sprites");

	return Sprite_getScale((Sprite)VirtualNode_getData(VirtualList_begin(this->sprites)));
}

// retrieve position
VBVec3D Entity_getPosition(Entity this)
{
	ASSERT(this, "Entity::getPosition: null this");

	return this->transform.globalPosition;
}

// retrieve local position
VBVec3D Entity_getLocalPosition(Entity this)
{
	ASSERT(this, "Entity::getLocalPosition: null this");

	return this->transform.localPosition;
}

// retrieve sprite
VirtualList Entity_getSprites(Entity this)
{
	ASSERT(this, "Entity::getSprites: null this");

	return this->sprites;
}

// process a telegram
int Entity_handleMessage(Entity this, Telegram telegram)
{
	ASSERT(this, "Entity::handleMessage: null this");

	return false;
}

// get width
u16 Entity_getWidth(Entity this)
{
	ASSERT(this, "Entity::getWidth: null this");
	ASSERT(this->sprites, "Entity::getWidth: null sprites");

	if (!this->size.x)
	{
		Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));
		Texture texture = Sprite_getTexture(sprite);

		this->size.x = Optics_calculateRealSize(((u16)Texture_getCols(texture)) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).x));
	}

	// must calculate based on the scale because not affine Container must be enlarged
	return this->size.x;
}

// get height
u16 Entity_getHeight(Entity this)
{
	ASSERT(this, "Entity::getHeight: null this");
	ASSERT(this->sprites, "Entity::getHeight: null sprites");

	if (!this->size.y)
	{
		Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));
		Texture texture = Sprite_getTexture(sprite);

		this->size.y = Optics_calculateRealSize(((u16)Texture_getRows(texture)) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).y));
	}

	return this->size.y;
}

// get deep
u16 Entity_getDeep(Entity this)
{
	ASSERT(this, "Entity::getDeep: null this");

	if (!this->size.z)
	{
		this->size.z = 1;
	}

	// must calculate based on the scale because not affine object must be enlarged
	return this->size.z;
}

// retrieve gap
Gap Entity_getGap(Entity this)
{
	ASSERT(this, "InGameEntity::getGap: null this");

	Gap gap = {0, 0, 0, 0};
	return gap;
}

// get entity's shape type
int Entity_getShapeType(Entity this)
{
	return kCuboid;
}

// whether it is visible
int Entity_isVisible(Entity this, int pad)
{
	ASSERT(this, "Entity::isVisible: null this");

	ASSERT(this->sprites, "Entity::isVisible: null sprites");

	if (!this->sprites)
	{
		return true;
	}

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return sprite? Optics_isVisible(this->transform.globalPosition,
			Entity_getWidth(this),
			Entity_getHeight(this),
			Sprite_getDrawSpec(sprite).position.parallax,
			pad): true;
}

// create an entity in gameengine's memory
Entity Entity_load(EntityDefinition* entityDefinition, int ID, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::load: null definition");
	ASSERT(entityDefinition->allocator, "Entity::load: no allocator defined");

	if (entityDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		Entity entity = (Entity)((Entity (*)(EntityDefinition*, ...)) entityDefinition->allocator)(0, entityDefinition, ID);

		// setup entity if allocated and constructed
		if (entity)
	    {
			// process extra info
			if (extraInfo)
	        {
				__VIRTUAL_CALL(void, Entity, setExtraInfo, entity, __ARGUMENTS(extraInfo));
			}

			return entity;
		}
	}

	return NULL;
}

// check if must update sprite's position
int Entity_updateSpritePosition(Entity this)
{
	ASSERT(this, "Entity::updateSpritePosition: null this");
	return ((_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z) || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z);
}

// check if must update sprite's scale
int Entity_updateSpriteScale(Entity this)
{
	ASSERT(this, "Entity::updateSpriteScale: null this");

	return (_screenDisplacement->z || this->invalidateGlobalPosition.z);
}

// set the direction
void Entity_setSpritesDirection(Entity this, int axis, int direction)
{
	ASSERT(this, "Entity::setSpritesDirection: null this");

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for (; node ; node = VirtualNode_getNext(node))
	    {
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			if (sprite)
	        {
				Sprite_setDirection(sprite, axis, direction);
			}
		}
	}
}

// does it moves?
u8 Entity_moves(Entity this)
{
	return false;
}

// retrieve previous position
const VBVec3D* Entity_getPreviousPosition(Entity this)
{
	static VBVec3D position = {0, 0, 0};
	return &position;
}

// retrieve shape
Shape Entity_getShape(Entity this)
{
	ASSERT(this, "Entity::getShape: null this");

	return this->shape;
}

// make it visible
void Entity_show(Entity this)
{
	ASSERT(this, "Entity::hide: null this");

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for (; node ; node = VirtualNode_getNext(node))
	    {
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			if (sprite)
	        {
				Sprite_show(sprite);
			}
		}
	}
}

// make it invisible
void Entity_hide(Entity this)
{
	ASSERT(this, "Entity::hide: null this");

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for (; node ; node = VirtualNode_getNext(node))
	    {
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			if (sprite)
	        {
				Sprite_hide(sprite);
			}
		}
	}
}