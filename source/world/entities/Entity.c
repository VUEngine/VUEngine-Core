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
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions);

// set sprites' visual properties
static void Entity_translateSprites(Entity this, int updateSpriteScale, int updateSpritePosition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id)
{
	ASSERT(this, "Entity::constructor: null this");

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Entity);

	// construct base Container
	__CONSTRUCT_BASE(Container, __ARGUMENTS(id));

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
		Entity_addSprites(this, entityDefinition->spritesDefinitions);
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

// create an entity in gameengine's memory
Entity Entity_load(const EntityDefinition* entityDefinition, int ID, void* extraInfo)
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

// load an entity from a PositionedEntity definition
Entity Entity_loadFromDefinition(const PositionedEntity* positionedEntity, const Transformation* environmentTransform, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");
	
	if (positionedEntity)
	{
		Entity entity = Entity_load(positionedEntity->entityDefinition, id, positionedEntity->extraInfo);

		if(entity)
		{
			VBVec3D position3D =
			{
					FTOFIX19_13(positionedEntity->position.x),
					FTOFIX19_13(positionedEntity->position.y),
					FTOFIX19_13(positionedEntity->position.z)
			};
	
			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(position3D));
	
			// apply transformations
			__VIRTUAL_CALL(void, Container, initialTransform, (Container)entity, __ARGUMENTS(environmentTransform));
	
			// add children if defined
			Entity_addChildren(entity, positionedEntity->childrenDefinitions, environmentTransform);
	
			return entity;
		}
	}

	return NULL;
}

// load children
void Entity_addChildren(Entity this, const PositionedEntity* childrenDefinitions, const Transformation* environmentTransform)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if (childrenDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for (; childrenDefinitions[i].entityDefinition; i++)
	    {
			Entity entity = Entity_loadFromDefinition(&childrenDefinitions[i], environmentTransform, this->id + Container_getChildCount((Container)this));

			// create the entity and add it to the world
			Container_addChild((Container)this, (Container)entity);
		}
	}	
}

// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo)
{
	ASSERT(this, "Entity::setExtraInfo: null this");
}

// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions)
{
	ASSERT(this, "Entity::addSprites: null this");

	if (spritesDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for (; spritesDefinitions[i].allocator; i++)
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

	ASSERT(spriteDefinition->allocator, "Entity::load: no sprite allocator defined");

	if (spriteDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		sprite = (Sprite)((Sprite (*)(SpriteDefinition*, ...)) spriteDefinition->allocator)(0, spriteDefinition, this);
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

	if (this->sprites)
	{
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
				this->size.x = this->size.y = 0;
			}
	
			//if screen is moving
			if (updateSpritePosition)
		    {
				//update sprite's 2D position
				Sprite_setPosition(sprite, &this->transform.globalPosition);
			}
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

	if (!this->sprites)
	{
		Scale scale = 
		{
			1,
			1
		};
		
		return scale;
	}
	
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
bool Entity_handleMessage(Entity this, Telegram telegram)
{
	ASSERT(this, "Entity::handleMessage: null this");

	return false;
}

// get width
u16 Entity_getWidth(Entity this)
{
	ASSERT(this, "Entity::getWidth: null this");

	if (!this->size.x && this->sprites)
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

	if (!this->size.y && this->sprites)
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
bool Entity_isVisible(Entity this, int pad)
{
	ASSERT(this, "Entity::isVisible: null this");

	if (!this->sprites)
	{
		return true;
	}

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return sprite ? Optics_isVisible(this->transform.globalPosition,
			Entity_getWidth(this),
			Entity_getHeight(this),
			Sprite_getDrawSpec(sprite).position.parallax,
			pad) : true;
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
bool Entity_moves(Entity this)
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