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
__CLASS_DEFINITION(Entity, Container);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
const extern VBVec3D* _screenDisplacement;
extern const Optical* _optical;

static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions[]);
static void Entity_releaseSprites(Entity this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id)
{
	ASSERT(this, "Entity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(id);

	// save definition
	this->entityDefinition = entityDefinition;

	/* the sprite must be initializated in the derivated class */
	this->sprites = NULL;

	this->shape = NULL;

	this->size.x = 0;
	this->size.y = 0;
	this->size.z = 0;
}

// class's destructor
void Entity_destructor(Entity this)
{
	ASSERT(this, "Entity::destructor: null this");

	// better to do it here than forget in other classes
	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);

	this->shape = NULL;

	Entity_releaseSprites(this);

	// destroy the super Container
	__DESTROY_BASE;
}

// release sprites
static void Entity_releaseSprites(Entity this)
{
	ASSERT(this, "Entity::releaseSprites: null this");

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for (; node ; node = VirtualNode_getNext(node))
	    {
			__DELETE(__UPCAST(Sprite, VirtualNode_getData(node)));
		}

		// delete the sprites
		__DELETE(this->sprites);

		this->sprites = NULL;
	}
}

static void Entity_getSizeFromChildren(Entity this, SmallRightcuboid* rightcuboid)
{
	ASSERT(this, "Entity::getSizeFromChildren: null this");

	u16 halfWidth = 0;
	u16 halfHeight = 0;
	u16 halfDeep = 10;

	if (this->sprites)
	{
		Sprite sprite = __UPCAST(Sprite, VirtualList_front(this->sprites));
		Texture texture = Sprite_getTexture(sprite);

		if(texture)
		{
			halfWidth = Optics_calculateRealSize(((u16)Texture_getCols(texture)) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).x)) >> 1;
			halfHeight = Optics_calculateRealSize(((u16)Texture_getRows(texture)) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).y)) >> 1;
			halfDeep = 10;
		}
	}
	else if(!this->children || 0 == VirtualList_getSize(this->children))
	{
		halfWidth = __SCREEN_WIDTH >> 1;
		halfHeight = __SCREEN_HEIGHT >> 1;
		halfDeep = 10;
	}
	
	int x = FIX19_13TOI(this->transform.globalPosition.x);
	int y = FIX19_13TOI(this->transform.globalPosition.y);
	int z = FIX19_13TOI(this->transform.globalPosition.z);
	
	if(0 == rightcuboid->x1 || halfWidth + x > rightcuboid->x1)
	{
		rightcuboid->x1 = halfWidth + x;
	}
	
	if(0 == rightcuboid->x0 || x - halfWidth < rightcuboid->x0)
	{
		rightcuboid->x0 = x - halfWidth;
	}
	
	if(0 == rightcuboid->y1 || halfHeight + y > rightcuboid->y1)
	{
		rightcuboid->y1 = halfHeight + y;
	}
	
	if(0 == rightcuboid->y0 || y - halfHeight < rightcuboid->y0)
	{
		rightcuboid->y0 = y - halfHeight;
	}

	if(0 == rightcuboid->z1 || halfDeep + z > rightcuboid->z1)
	{
		rightcuboid->z1 = halfDeep + z;
	}
	
	if(0 == rightcuboid->z0 || z - halfDeep < rightcuboid->z0)
	{
		rightcuboid->z0 = z - halfHeight;
	}

	if(this->children)
	{
		VirtualNode node = VirtualList_begin(this->children);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			Entity_getSizeFromChildren(__UPCAST(Entity, VirtualNode_getData(node)), rightcuboid);
		}
	}
}

// calculate my size based on me and my children
void Entity_calculateSize(Entity this)
{
	SmallRightcuboid rightcuboid = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromChildren(this, &rightcuboid);

	rightcuboid.x0 = FIX19_13TOI(this->transform.globalPosition.x) - rightcuboid.x0;
	rightcuboid.x1 = rightcuboid.x1 - FIX19_13TOI(this->transform.globalPosition.x);
	rightcuboid.y0 = FIX19_13TOI(this->transform.globalPosition.y) - rightcuboid.y0;
	rightcuboid.y1 = rightcuboid.y1 - FIX19_13TOI(this->transform.globalPosition.y);
	rightcuboid.z0 = FIX19_13TOI(this->transform.globalPosition.z) - rightcuboid.z0;
	rightcuboid.z1 = rightcuboid.z1 - FIX19_13TOI(this->transform.globalPosition.z);

	this->size.x = rightcuboid.x0 + rightcuboid.x1;
	this->size.y = rightcuboid.y0 + rightcuboid.y1;
	this->size.z = rightcuboid.z0 + rightcuboid.z1;
}

static void Entity_getSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition, SmallRightcuboid* rightcuboid)
{
	ASSERT(positionedEntity, "Entity::getSizeFromDefinition: null positionedEntity");
	ASSERT(positionedEntity->entityDefinition, "Entity::getSizeFromDefinition: null entityDefinition");

	VBVec3D globalPosition3D =
	{
		environmentPosition->x + positionedEntity->position.x,
		environmentPosition->y + positionedEntity->position.y,
		environmentPosition->z + positionedEntity->position.z
	};

	s16 halfWidth = 0;
	s16 halfHeight = 0;
	s16 halfDeep = 10;

	if(positionedEntity->entityDefinition->spritesDefinitions && positionedEntity->entityDefinition->spritesDefinitions[0] && positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition)
	{
		halfWidth = positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition->cols << 2;
		halfHeight = positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition->rows << 2;
		halfDeep = 10;

		if(positionedEntity->entityDefinition->spritesDefinitions && 
				positionedEntity->entityDefinition->spritesDefinitions[0] && 
				WRLD_AFFINE == positionedEntity->entityDefinition->spritesDefinitions[0]->bgmapMode)
		{
			fix7_9 scale = FIX19_13TOFIX7_9(ITOFIX19_13(1) -
				       FIX19_13_DIV(positionedEntity->position.z, _optical->maximumViewDistance));
		
			halfWidth = FIX19_13TOI(FIX19_13_DIV(ITOFIX19_13(halfWidth), FIX7_9TOFIX19_13(scale)));
			halfHeight = FIX19_13TOI(FIX19_13_DIV(ITOFIX19_13(halfHeight), FIX7_9TOFIX19_13(scale)));
		}
	}
	else if(!positionedEntity->childrenDefinitions)
	{
		halfWidth = __SCREEN_WIDTH >> 1;
		halfHeight = __SCREEN_HEIGHT >> 1;
		halfDeep = 10;
	}

	int x = FIX19_13TOI(globalPosition3D.x);
	int y = FIX19_13TOI(globalPosition3D.y);
	int z = FIX19_13TOI(globalPosition3D.z);
	
	if(0 == rightcuboid->x1 || halfWidth + x > rightcuboid->x1)
	{
		rightcuboid->x1 = halfWidth + x;
	}
	
	if(0 == rightcuboid->x0 || x - halfWidth < rightcuboid->x0)
	{
		rightcuboid->x0 = x - halfWidth;
	}
	
	if(0 == rightcuboid->y1 || halfHeight + y > rightcuboid->y1)
	{
		rightcuboid->y1 = halfHeight + y;
	}
	
	if(0 == rightcuboid->y0 || y - halfHeight < rightcuboid->y0)
	{
		rightcuboid->y0 = y - halfHeight;
	}

	if(0 == rightcuboid->z1 || halfDeep + z > rightcuboid->z1)
	{
		rightcuboid->z1 = halfDeep + z;
	}
	
	if(0 == rightcuboid->z0 || z - halfDeep < rightcuboid->z0)
	{
		rightcuboid->z0 = z - halfHeight;
	}

	if(positionedEntity->childrenDefinitions)
	{
		int i = 0;
		for(; positionedEntity->childrenDefinitions[i].entityDefinition; i++)
		{
			Entity_getSizeFromDefinition(&positionedEntity->childrenDefinitions[i], &globalPosition3D, rightcuboid);
		}
	}
}

// calculate total size from definition
SmallRightcuboid Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition)
{
	SmallRightcuboid rightcuboid = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromDefinition(positionedEntity, (VBVec3D*)environmentPosition, &rightcuboid);

	rightcuboid.x0 = (s16)FIX19_13TOI(positionedEntity->position.x) - rightcuboid.x0;
	rightcuboid.x1 = rightcuboid.x1 - (s16)FIX19_13TOI(positionedEntity->position.x);
	rightcuboid.y0 = (s16)FIX19_13TOI(positionedEntity->position.y) - rightcuboid.y0;
	rightcuboid.y1 = rightcuboid.y1 - (s16)FIX19_13TOI(positionedEntity->position.y);

	return rightcuboid;
}

// create an entity in gameengine's memory
Entity Entity_load(const EntityDefinition* entityDefinition, int id, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::load: null definition");
	ASSERT(entityDefinition->allocator, "Entity::load: no allocator defined");

	if (entityDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		Entity entity = ((Entity (*)(EntityDefinition*, ...)) entityDefinition->allocator)((EntityDefinition*)entityDefinition, id);

		// setup entity if allocated and constructed
		if (entity)
	    {
			// process extra info
			if (extraInfo)
	        {
				__VIRTUAL_CALL(void, Entity, setExtraInfo, entity, extraInfo);
			}

			return entity;
		}
	}

	return NULL;
}

// load an entity from a PositionedEntity definition
Entity Entity_loadFromDefinition(const PositionedEntity* positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");
	
	if (positionedEntity)
	{
		Entity entity = Entity_load(positionedEntity->entityDefinition, id, positionedEntity->extraInfo);

		if(entity)
		{
			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, positionedEntity->position);
	
			// add children if defined
			if (positionedEntity->childrenDefinitions)
			{
				Entity_addChildren(entity, positionedEntity->childrenDefinitions);
			}	
			
			return entity;
		}
	}

	return NULL;
}

// load children
void Entity_addChildrenWithoutInitilization(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if (childrenDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for (; childrenDefinitions[i].entityDefinition; i++)
	    {
			Entity entity = Entity_loadFromDefinitionWithoutInitilization(&childrenDefinitions[i], this->id + Container_getChildCount(__UPCAST(Container, this)));

			// create the entity and add it to the world
			Container_addChild(__UPCAST(Container, this), __UPCAST(Container, entity));
		}
	}	
}

// load an entity from a PositionedEntity definition
Entity Entity_loadFromDefinitionWithoutInitilization(const PositionedEntity* positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");
	
	if (positionedEntity)
	{
		Entity entity = Entity_load(positionedEntity->entityDefinition, id, positionedEntity->extraInfo);
		
		if(entity)
		{
			if(positionedEntity->name)
			{
				Container_setName(__UPCAST(Container, entity), positionedEntity->name);
			}

			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, positionedEntity->position);
			
			// add children if defined
			if (positionedEntity->childrenDefinitions)
			{
				Entity_addChildrenWithoutInitilization(entity, positionedEntity->childrenDefinitions);
			}	
		
			return entity;
		}
	}

	return NULL;
}

// initialize from definition
void Entity_initialize(Entity this)
{
	ASSERT(this, "Entity::initialize: null this");
	
	if (!this->sprites)
	{
		Entity_addSprites(this, this->entityDefinition->spritesDefinitions);
	}

	if(this->children)
	{
		VirtualNode node = VirtualList_begin(this->children);

		for(; node; node = VirtualNode_getNext(node))
		{
			__VIRTUAL_CALL(void, Entity, initialize, __UPCAST(Entity, VirtualNode_getData(node)));
		}
	}
	
	if(this->shape)
	{
		Shape_setActive(this->shape, true);
	}
}

// load children
void Entity_addChildren(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if (childrenDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for (; childrenDefinitions[i].entityDefinition; i++)
	    {
			Entity entity = Entity_loadFromDefinition(&childrenDefinitions[i], this->id + Container_getChildCount(__UPCAST(Container, this)));

			// create the entity and add it to the world
			Container_addChild(__UPCAST(Container, this), __UPCAST(Container, entity));
		}
	}	
}

// add child from definition
Entity Entity_addChildFromDefinition(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3D* position, void* extraInfo)
{
	PositionedEntity positionedEntity = 
	{
		(EntityDefinition*)entityDefinition, 
		{position->x, position->y, position->z}, 
		(char*)name,
		NULL, 
		extraInfo
	};

    // create the hint entity and add it to the hero as a child entity
	Entity childEntity = Entity_loadFromDefinition(&positionedEntity, 0 > id? id: this->id + Container_getChildCount(__UPCAST(Container, this)));

	if(childEntity)
	{
		// must initialize after adding the children
		__VIRTUAL_CALL(void, Entity, initialize, childEntity);
	
		Transformation environmentTransform = Container_getEnvironmentTransform(__UPCAST(Container, this));

		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, childEntity, &environmentTransform);

		// create the entity and add it to the world
		Container_addChild(__UPCAST(Container, this), __UPCAST(Container, childEntity));
	}

	return childEntity;
}

// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo)
{
	ASSERT(this, "Entity::setExtraInfo: null this");
}

// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions[])
{
	ASSERT(this, "Entity::addSprites: null this");

	if (spritesDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for (; spritesDefinitions[i]; i++)
	    {
			Entity_addSprite(this, spritesDefinitions[i]);
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
		sprite = ((Sprite (*)(SpriteDefinition*, ...)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, this);
	}

	if (sprite)
	{
		if (!this->sprites)
	    {
			this->sprites = __NEW(VirtualList);
		}

		VirtualList_pushBack(this->sprites, (void*)sprite);
	}
	else 
	{
		ASSERT(false, "Entity::addSprite: sprite not created");
	}
}

// transform sprites
void Entity_translateSprites(Entity this, bool updateSpriteScale, bool updateSpritePosition)
{
	ASSERT(this, "Entity::transform: null this");

	if (this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		if(updateSpriteScale && updateSpritePosition)
		{
			// move each child to a temporary list
			for (; node ; node = VirtualNode_getNext(node))
			{
				Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(node));
		
				// calculate the scale
				Sprite_calculateScale(sprite, this->transform.globalPosition.z);
	
				// calculate sprite's parallax
				Sprite_calculateParallax(sprite, this->transform.globalPosition.z);

				//update sprite's 2D position
				__VIRTUAL_CALL(void, Sprite, setPosition, sprite, this->transform.globalPosition);
			}
		}
		else if(!updateSpriteScale && updateSpritePosition)
		{
			// move each child to a temporary list
			for (; node ; node = VirtualNode_getNext(node))
			{
				Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(node));
		
				//update sprite's 2D position
				__VIRTUAL_CALL(void, Sprite, setPosition, sprite, this->transform.globalPosition);
			}
		}
		else if(updateSpriteScale && !updateSpritePosition)
		{
			// move each child to a temporary list
			for (; node ; node = VirtualNode_getNext(node))
			{
				Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(node));
		
				// calculate the scale
				Sprite_calculateScale(sprite, this->transform.globalPosition.z);
	
				// calculate sprite's parallax
				Sprite_calculateParallax(sprite, this->transform.globalPosition.z);
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

	// call base class's transform method
	Container_initialTransform(__UPCAST(Container, this), environmentTransform);

	// force sprite translation
	Entity_translateSprites(this, true, true);

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

	bool updateSpritePosition = __VIRTUAL_CALL(bool, Entity, updateSpritePosition, this);
	bool updateSpriteScale = __VIRTUAL_CALL(bool, Entity, updateSpriteScale, this);

	if (this->invalidateGlobalPosition.x ||
		this->invalidateGlobalPosition.y ||
		this->invalidateGlobalPosition.z ||
		this->children)
	{
		// call base class's transform method
		Container_transform(__UPCAST(Container, this), environmentTransform);
	}

	if(updateSpritePosition || updateSpriteScale)
	{
		// update graphical representation
		Entity_translateSprites(this, updateSpriteScale, updateSpritePosition);
	}
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
	
	return Sprite_getScale(__UPCAST(Sprite, VirtualNode_getData(VirtualList_begin(this->sprites))));
}

// set local position
void Entity_setLocalPosition(Entity this, VBVec3D position)
{
	Container_setLocalPosition(__UPCAST(Container, this), position);
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

	if (!this->size.x)
	{
		Entity_calculateSize(this);
	}

	// must calculate based on the scale because not affine Container must be enlarged
	return this->size.x;
}

// get height
u16 Entity_getHeight(Entity this)
{
	ASSERT(this, "Entity::getHeight: null this");

	if (!this->size.y)
	{
		Entity_calculateSize(this);
	}
	
	return this->size.y;
}

// get deep
u16 Entity_getDeep(Entity this)
{
	ASSERT(this, "Entity::getDeep: null this");

	if (!this->size.z)
	{
		Entity_calculateSize(this);
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

	if(this->children)
	{
		VirtualNode childNode = VirtualList_begin(this->children);
		
		for(; childNode; childNode = VirtualNode_getNext(childNode))
		{
			if(__VIRTUAL_CALL(bool, Entity, isVisible, VirtualNode_getData(childNode), pad))
			{
				return true;
			}
		}
	}
	
	int lowLimit = 0 - pad;
	int highLimit = __SCREEN_WIDTH + pad;
	int x = 0;
	int y = 0;
	int z = 0;
	
	if (this->sprites)
	{
		static Sprite sprite = NULL;
		sprite = __UPCAST(Sprite, VirtualNode_getData(VirtualList_begin(this->sprites)));

		ASSERT(sprite, "Entity:isVisible: null sprite");
		
		DrawSpec drawSpec = Sprite_getDrawSpec(sprite);
		lowLimit -= drawSpec.position.parallax;
		highLimit += drawSpec.position.parallax;
		
		x = FIX19_13TOI(drawSpec.position.x);
		y = FIX19_13TOI(drawSpec.position.y);
		z = FIX19_13TOI(this->transform.globalPosition.z - _screenPosition->z);
	}
	else 
	{
		VBVec3D position3D = this->transform.globalPosition;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		x = FIX19_13TOI(position3D.x) - (this->size.x >> 1);
		y = FIX19_13TOI(position3D.y) - (this->size.y >> 1);
		z = FIX19_13TOI(position3D.z);
	}

	// check x visibility
	if (x + this->size.x < lowLimit || x > highLimit)
	{
		return false;
	}

	lowLimit = -pad;
	highLimit = __SCREEN_HEIGHT + pad;

	// check y visibility
	if (y + this->size.y < lowLimit || y > highLimit)
	{
		return false;
	}

	lowLimit = -pad;
	highLimit = _optical->maximumViewDistance + pad;

	// check y visibility
	if (z + this->size.z < lowLimit || z > highLimit)
	{
		return false;
	}

	return true;
}

// check if must update sprite's position
bool Entity_updateSpritePosition(Entity this)
{
	ASSERT(this, "Entity::updateSpritePosition: null this");
	return ((_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z) || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z);
}

// check if must update sprite's scale
bool Entity_updateSpriteScale(Entity this)
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

		for (; node ; node = VirtualNode_getNext(node))
	    {
			Sprite_setDirection(__UPCAST(Sprite, VirtualNode_getData(node)), axis, direction);
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
			Sprite_show(__UPCAST(Sprite, VirtualNode_getData(node)));
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
			Sprite_hide(__UPCAST(Sprite, VirtualNode_getData(node)));
		}
	}
}

// suspend for pause
void Entity_suspend(Entity this)
{
	ASSERT(this, "Entity::suspend: null this");

	Container_suspend(__UPCAST(Container, this));
	Entity_releaseSprites(this);
}

// resume after pause
void Entity_resume(Entity this)
{
	ASSERT(this, "Entity::resume: null this");

	Container_resume(__UPCAST(Container, this));

	// initialize sprites
	if (this->entityDefinition)
	{
		Entity_addSprites(this, this->entityDefinition->spritesDefinitions);
	}
}

// defaults to true
bool Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration)
{
	return true;
}