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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Entity.h>
#include <InGameEntity.h>
#include <InanimatedInGameEntity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Entity
__CLASS_DEFINITION(Entity, Container);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


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
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "Entity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(id, name);

	// save definition
	this->entityDefinition = entityDefinition;

	// the sprite must be initialized in the derived class
	this->sprites = NULL;

	this->shape = NULL;

	// initialize to 0 for the engine to know that
	// size must be set
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
	if(this->shape)
	{
		CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);
	
		this->shape = NULL;
	}

	Entity_releaseSprites(this);

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// release sprites
static void Entity_releaseSprites(Entity this)
{
	ASSERT(this, "Entity::releaseSprites: null this");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// move each child to a temporary list
		for(; node ; node = node->next)
	    {
			__DELETE(node->data);
		}

		// delete the sprites
		__DELETE(this->sprites);

		this->sprites = NULL;
	}
}

static void Entity_getSizeFromChildren(Entity this, SmallRightCuboid* rightCuboid)
{
	ASSERT(this, "Entity::getSizeFromChildren: null this");

	u16 halfWidth = 0;
	u16 halfHeight = 0;
	u16 halfDepth = 10;

	if(this->sprites)
	{
		Sprite sprite = __SAFE_CAST(Sprite, VirtualList_front(this->sprites));
		Texture texture = Sprite_getTexture(sprite);

		if(texture)
		{
			halfWidth = Optics_calculateRealSize(((u16)Texture_getCols(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL_UNSAFE(Scale, Sprite, getScale, sprite).x)) >> 1;
			halfHeight = Optics_calculateRealSize(((u16)Texture_getRows(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL_UNSAFE(Scale, Sprite, getScale, sprite).y)) >> 1;
			halfDepth = this->size.z >> 1;
		}
	}
	else if(!this->children || !this->children->head)
	{
		halfWidth = this->size.x >> 1;
		halfHeight = this->size.y >> 1;
		halfDepth = this->size.z >> 1;
	}
	
	int x = FIX19_13TOI(this->transform.globalPosition.x);
	int y = FIX19_13TOI(this->transform.globalPosition.y);
	int z = FIX19_13TOI(this->transform.globalPosition.z);
	
	if(0 == rightCuboid->x1 || halfWidth + x > rightCuboid->x1)
	{
		rightCuboid->x1 = halfWidth + x;
	}
	
	if(0 == rightCuboid->x0 || x - halfWidth < rightCuboid->x0)
	{
		rightCuboid->x0 = x - halfWidth;
	}
	
	if(0 == rightCuboid->y1 || halfHeight + y > rightCuboid->y1)
	{
		rightCuboid->y1 = halfHeight + y;
	}
	
	if(0 == rightCuboid->y0 || y - halfHeight < rightCuboid->y0)
	{
		rightCuboid->y0 = y - halfHeight;
	}

	if(0 == rightCuboid->z1 || halfDepth + z > rightCuboid->z1)
	{
		rightCuboid->z1 = halfDepth + z;
	}
	
	if(0 == rightCuboid->z0 || z - halfDepth < rightCuboid->z0)
	{
		rightCuboid->z0 = z - halfHeight;
	}

	if(this->children)
	{
		VirtualNode node = this->children->head;
		
		for(; node; node = node->next)
		{
			Entity_getSizeFromChildren(__SAFE_CAST(Entity, node->data), rightCuboid);
		}
	}
}

// calculate my size based on me and my children
void Entity_calculateSize(Entity this)
{
	SmallRightCuboid rightCuboid = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromChildren(this, &rightCuboid);

	rightCuboid.x0 = FIX19_13TOI(this->transform.globalPosition.x) - rightCuboid.x0;
	rightCuboid.x1 = rightCuboid.x1 - FIX19_13TOI(this->transform.globalPosition.x);
	rightCuboid.y0 = FIX19_13TOI(this->transform.globalPosition.y) - rightCuboid.y0;
	rightCuboid.y1 = rightCuboid.y1 - FIX19_13TOI(this->transform.globalPosition.y);
	rightCuboid.z0 = FIX19_13TOI(this->transform.globalPosition.z) - rightCuboid.z0;
	rightCuboid.z1 = rightCuboid.z1 - FIX19_13TOI(this->transform.globalPosition.z);

	this->size.x = rightCuboid.x0 + rightCuboid.x1;
	this->size.y = rightCuboid.y0 + rightCuboid.y1;
	this->size.z = rightCuboid.z0 + rightCuboid.z1;
}

static void Entity_getSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition, SmallRightCuboid* rightCuboid)
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
	s16 halfDepth = 10;

	if(positionedEntity->entityDefinition->spritesDefinitions && positionedEntity->entityDefinition->spritesDefinitions[0] && positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition)
	{
		halfWidth = positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition->cols << 2;
		halfHeight = positionedEntity->entityDefinition->spritesDefinitions[0]->textureDefinition->rows << 2;
		halfDepth = 10;
/*
		if(positionedEntity->entityDefinition->spritesDefinitions && 
				positionedEntity->entityDefinition->spritesDefinitions[0] && 
				WRLD_AFFINE == positionedEntity->entityDefinition->spritesDefinitions[0]->bgmapMode)
		{
			fix7_9 scale = FIX19_13TOFIX7_9(ITOFIX19_13(1) -
				       (positionedEntity->position.z >> _optical->maximumViewDistancePower));
		
			halfWidth = FIX19_13TOI(FIX19_13_DIV(ITOFIX19_13(halfWidth), FIX7_9TOFIX19_13(scale)));
			halfHeight = FIX19_13TOI(FIX19_13_DIV(ITOFIX19_13(halfHeight), FIX7_9TOFIX19_13(scale)));
		}
		
		*/
	}
	else if(!positionedEntity->childrenDefinitions)
	{
		// TODO: there should be a class which handles these special cases
		if((int)__TYPE(InGameEntity) == (int)positionedEntity->entityDefinition->allocator || 
			(int)__TYPE(InanimatedInGameEntity) == (int)positionedEntity->entityDefinition->allocator 
		)
		{
			halfWidth = ((InGameEntityDefinition*)positionedEntity->entityDefinition)->width >> 1;
			halfHeight = ((InGameEntityDefinition*)positionedEntity->entityDefinition)->height >> 1;
			halfDepth = ((InGameEntityDefinition*)positionedEntity->entityDefinition)->depth >> 1;
		}
		else
		{
			halfWidth = 1;
			halfHeight = 1;
			halfDepth = 1;
		}
	}

	int x = FIX19_13TOI(globalPosition3D.x);
	int y = FIX19_13TOI(globalPosition3D.y);
	int z = FIX19_13TOI(globalPosition3D.z);
	
	if(0 == rightCuboid->x1 || halfWidth + x > rightCuboid->x1)
	{
		rightCuboid->x1 = halfWidth + x;
	}
	
	if(0 == rightCuboid->x0 || x - halfWidth < rightCuboid->x0)
	{
		rightCuboid->x0 = x - halfWidth;
	}
	
	if(0 == rightCuboid->y1 || halfHeight + y > rightCuboid->y1)
	{
		rightCuboid->y1 = halfHeight + y;
	}
	
	if(0 == rightCuboid->y0 || y - halfHeight < rightCuboid->y0)
	{
		rightCuboid->y0 = y - halfHeight;
	}

	if(0 == rightCuboid->z1 || halfDepth + z > rightCuboid->z1)
	{
		rightCuboid->z1 = halfDepth + z;
	}
	
	if(0 == rightCuboid->z0 || z - halfDepth < rightCuboid->z0)
	{
		rightCuboid->z0 = z - halfHeight;
	}

	if(positionedEntity->childrenDefinitions)
	{
		int i = 0;
		for(; positionedEntity->childrenDefinitions[i].entityDefinition; i++)
		{
			Entity_getSizeFromDefinition(&positionedEntity->childrenDefinitions[i], &globalPosition3D, rightCuboid);
		}
	}
}

// calculate total size from definition
SmallRightCuboid Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const VBVec3D* environmentPosition)
{
	SmallRightCuboid rightCuboid = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromDefinition(positionedEntity, (VBVec3D*)environmentPosition, &rightCuboid);

	rightCuboid.x0 = (s16)FIX19_13TOI(positionedEntity->position.x) - rightCuboid.x0;
	rightCuboid.x1 = rightCuboid.x1 - (s16)FIX19_13TOI(positionedEntity->position.x);
	rightCuboid.y0 = (s16)FIX19_13TOI(positionedEntity->position.y) - rightCuboid.y0;
	rightCuboid.y1 = rightCuboid.y1 - (s16)FIX19_13TOI(positionedEntity->position.y);

	return rightCuboid;
}

// find child by name in given list
VBVec3D* Entity_calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, VBVec3D environmentPosition, const char* childName)
{
	ASSERT(childrenDefinitions, "Entity::calculatGlobalPositionFromDefinitionByName: null positionedEntity");

	static VBVec3D position;
	
	int i = 0;
	for(; childrenDefinitions[i].entityDefinition; i++)
	{
		if(!strncmp(childName, childrenDefinitions[i].name, __MAX_CONTAINER_NAME_LENGTH))
		{
			position.x = environmentPosition.x + childrenDefinitions[i].position.x;
			position.y = environmentPosition.y + childrenDefinitions[i].position.y;
			position.z = environmentPosition.z + childrenDefinitions[i].position.z;
			return &position;
		}

		if(childrenDefinitions[i].childrenDefinitions)
		{
			VBVec3D concatenatedEnvironmentPosition = environmentPosition;
			concatenatedEnvironmentPosition.x += childrenDefinitions[i].position.x;
			concatenatedEnvironmentPosition.y += childrenDefinitions[i].position.y;
			concatenatedEnvironmentPosition.z += childrenDefinitions[i].position.z;

			VBVec3D* position = Entity_calculateGlobalPositionFromDefinitionByName(childrenDefinitions[i].childrenDefinitions, concatenatedEnvironmentPosition, childName);
			
			if(position)
			{
				return position;
			}
		}
	}

    return NULL;
}

// create an entity in gameengine's memory
Entity Entity_load(const EntityDefinition* const entityDefinition, int id, const char* const name, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::load: null definition");
	ASSERT(entityDefinition->allocator, "Entity::load: no allocator defined");

	if(entityDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		Entity entity = ((Entity (*)(EntityDefinition*, ...)) entityDefinition->allocator)((EntityDefinition*)entityDefinition, id, name);

		// setup entity if allocated and constructed
		if(entity)
	    {
			// process extra info
			if(extraInfo)
	        {
				__VIRTUAL_CALL(void, Entity, setExtraInfo, entity, extraInfo);
			}

			return entity;
		}
	}

	return NULL;
}

// load an entity from a PositionedEntity definition
Entity Entity_loadFromDefinition(const PositionedEntity* const positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");
	
	if(positionedEntity)
	{
		Entity entity = Entity_load(positionedEntity->entityDefinition, id, positionedEntity->name, positionedEntity->extraInfo);

		if(entity)
		{
			// set spatial position
			__VIRTUAL_CALL(void, Container, setLocalPosition, entity, &positionedEntity->position);
	
			// add children if defined
			if(positionedEntity->childrenDefinitions)
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

	if(childrenDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for(; childrenDefinitions[i].entityDefinition; i++)
	    {
			Entity entity = Entity_loadFromDefinitionWithoutInitilization(&childrenDefinitions[i], this->id + Container_getChildCount(__SAFE_CAST(Container, this)));

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));
		}
	}	
}

// load an entity from a PositionedEntity definition
Entity Entity_loadFromDefinitionWithoutInitilization(const PositionedEntity* const positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinitionWithoutInitilization: null positionedEntity");
	
	if(positionedEntity)
	{
		Entity entity = Entity_load(positionedEntity->entityDefinition, id, positionedEntity->name, positionedEntity->extraInfo);
		
		if(entity)
		{
			if(positionedEntity->name)
			{
				Container_setName(__SAFE_CAST(Container, entity), positionedEntity->name);
			}

			// set spatial position
			__VIRTUAL_CALL(void, Container, setLocalPosition, entity, &positionedEntity->position);
			
			// add children if defined
			if(positionedEntity->childrenDefinitions)
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
	
	if(!this->sprites)
	{
		Entity_addSprites(this, this->entityDefinition->spritesDefinitions);
	}

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Entity, initialize, __SAFE_CAST(Entity, node->data));
		}
	}
	
	if(this->shape)
	{
		Shape_setActive(this->shape, true);
	}
	
	if(!this->size.x || !this->size.y || !this->size.z)
	{
		// must force size calculation now
		Entity_calculateSize(this);
	}
}

// entity is initialized
void Entity_ready(Entity this)
{
	ASSERT(this, "Entity::initialize: null this");
	
	if(this->children)
	{
		// call ready method on children
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Entity, ready, __SAFE_CAST(Entity, node->data));
		}
	}
}

// load children
void Entity_addChildren(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if(childrenDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for(; childrenDefinitions[i].entityDefinition; i++)
	    {
			Entity entity = Entity_loadFromDefinition(&childrenDefinitions[i], this->id + Container_getChildCount(__SAFE_CAST(Container, this)));

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));
		}
	}	
}

// add child from definition
Entity Entity_addChildFromDefinition(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3D* position, void* extraInfo)
{
	ASSERT(this, "Entity::addChildFromDefinition: null this");

	PositionedEntity positionedEntity = 
	{
		(EntityDefinition*)entityDefinition, 
		{position->x, position->y, position->z}, 
		(char*)name,
		NULL, 
		extraInfo
	};

    // create the hint entity and add it to the hero as a child entity
	Entity childEntity = Entity_loadFromDefinition(&positionedEntity, 0 > id? id: this->id + Container_getChildCount(__SAFE_CAST(Container, this)));

	if(childEntity)
	{
		// must initialize after adding the children
		__VIRTUAL_CALL(void, Entity, initialize, childEntity);

		// if already initialized
		if(0 <= this->size.x && 0 <= this->size.y && 0 <= this->size.z)
		{
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	
			 // apply transformations
			__VIRTUAL_CALL(void, Container, initialTransform, childEntity, &environmentTransform);
		}
		
		// create the entity and add it to the world
		Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, childEntity));
		
		__VIRTUAL_CALL(void, Entity, ready, childEntity);
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

	if(spritesDefinitions)
	{
		int i = 0;

		//go through n sprites in entity's definition
		for(; spritesDefinitions[i]; i++)
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

	if(spriteDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		sprite = ((Sprite (*)(SpriteDefinition*, ...)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, this);
	}

	if(sprite)
	{
		if(!this->sprites)
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
void Entity_translateSprites(Entity this, bool updateSpriteTransformations, bool updateSpritePosition)
{
	ASSERT(this, "Entity::transform: null this");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		if(updateSpriteTransformations && updateSpritePosition)
		{
			// move each child to a temporary list
			for(; node ; node = node->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, node->data);
		
				// calculate the scale
				__VIRTUAL_CALL(void, Sprite, resize, sprite, this->transform.globalScale, this->transform.globalPosition.z);

				// calculate sprite's parallax
				__VIRTUAL_CALL(void, Sprite, calculateParallax, sprite, this->transform.globalPosition.z);
				
				// update sprite's 2D position
				__VIRTUAL_CALL(void, Sprite, position, sprite, &this->transform.globalPosition);

				// update sprite's 2D rotation
				__VIRTUAL_CALL(void, Sprite, rotate, sprite, &this->transform.globalRotation);
			}
		}
		else if(!updateSpriteTransformations && updateSpritePosition)
		{
			// move each child to a temporary list
			for(; node ; node = node->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, node->data);
		
				//update sprite's 2D position
				__VIRTUAL_CALL(void, Sprite, position, sprite, &this->transform.globalPosition);

				// update sprite's 2D rotation
				__VIRTUAL_CALL(void, Sprite, rotate, sprite, &this->transform.globalRotation);
			}
		}
		else if(updateSpriteTransformations && !updateSpritePosition)
		{
			// move each child to a temporary list
			for(; node ; node = node->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, node->data);
		
				// calculate the scale
				__VIRTUAL_CALL(void, Sprite, resize, sprite, this->transform.globalScale, this->transform.globalPosition.z);
	
				// calculate sprite's parallax
				__VIRTUAL_CALL(void, Sprite, calculateParallax, sprite, this->transform.globalPosition.z);
			}
		}
	}
}

// initial transformation
void Entity_initialTransform(Entity this, Transformation* environmentTransform)
{
	ASSERT(this, "Entity::initialTransform: null this");

	this->invalidateGlobalPosition.x = true;
	this->invalidateGlobalPosition.y = true;
	this->invalidateGlobalPosition.z = true;

	// call base class's transform method
	Container_initialTransform(__SAFE_CAST(Container, this), environmentTransform);

	// force sprite translation
	Entity_translateSprites(this, true, true);

	if(this->hidden)
	{
		Entity_hide(this);
	}

	if(this->shape)
	{
		// setup shape
		__VIRTUAL_CALL(void, Shape, setup, this->shape);

		if(__VIRTUAL_CALL(int, Entity, moves, this))
	    {
			__VIRTUAL_CALL(void, Shape, position, this->shape);
		}
	}
}

// transform class
void Entity_transform(Entity this, const Transformation* environmentTransform)
{
	ASSERT(this, "Entity::transform: null this");

	bool updateSpritePosition = false;
	bool updateSpriteTransformations = false;

	if(this->invalidateGlobalPosition.x ||
		this->invalidateGlobalPosition.y ||
		this->invalidateGlobalPosition.z ||
		this->children)
	{
		updateSpritePosition = true;
		updateSpriteTransformations = this->invalidateGlobalPosition.z;

		// call base class's transform method
		Container_transform(__SAFE_CAST(Container, this), environmentTransform);
	}
	else
	{
		updateSpritePosition = __VIRTUAL_CALL(bool, Entity, updateSpritePosition, this);
		updateSpriteTransformations = __VIRTUAL_CALL(bool, Entity, updateSpriteTransformations, this);
	}

	if(updateSpritePosition || updateSpriteTransformations)
	{
		// update graphical representation
		Entity_translateSprites(this, updateSpriteTransformations, updateSpritePosition);
	}
}

void Entity_setLocalPosition(Entity this, const VBVec3D* position)
{
	ASSERT(this, "Entity::setLocalPosition: null this");

	Container_setLocalPosition(__SAFE_CAST(Container, this), position);
}


// retrieve EntityDefinition
EntityDefinition* Entity_getEntityDefinition(Entity this)
{
	ASSERT(this, "Entity::getEntityDefinition: null this");

	return this->entityDefinition;
}

// retrieve position
const VBVec3D* Entity_getPosition(Entity this)
{
	ASSERT(this, "Entity::getPosition: null this");

	return &this->transform.globalPosition;
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

	if(!this->size.x)
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

	if(!this->size.y)
	{
		Entity_calculateSize(this);
	}
	
	return this->size.y;
}

// get depth
u16 Entity_getDepth(Entity this)
{
	ASSERT(this, "Entity::getDepth: null this");

	if(!this->size.z)
	{
		Entity_calculateSize(this);
	}

	// must calculate based on the scale because not affine object must be enlarged
	return this->size.z;
}

// retrieve gap
Gap Entity_getGap(Entity this)
{
	ASSERT(this, "Entity::getGap: null this");

	Gap gap = {0, 0, 0, 0};
	return gap;
}

// whether it is visible
bool Entity_isVisible(Entity this, int pad)
{
	ASSERT(this, "Entity::isVisible: null this");

	int lowLimit = 0 - pad;
	int highLimit = __SCREEN_WIDTH + pad;
	int x = 0;
	int y = 0;
	int z = 0;
	
	if(this->sprites)
	{
		static Sprite sprite = NULL;
		sprite = __SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head));

		ASSERT(sprite, "Entity:isVisible: null sprite");
		
		const VBVec2D* spritePosition = __VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, sprite);
		lowLimit -= spritePosition->parallax;
		highLimit += spritePosition->parallax;
		
		x = FIX19_13TOI(spritePosition->x);
		y = FIX19_13TOI(spritePosition->y);
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
	if(x + this->size.x >= lowLimit && x <= highLimit)
	{
		lowLimit = -pad;
		highLimit = __SCREEN_HEIGHT + pad;

		// check y visibility
		if(y + this->size.y >= lowLimit && y <= highLimit)
		{
			
			lowLimit = -pad;
			highLimit = (1 << _optical->maximumViewDistancePower) + pad;

			// check z visibility
			return z + this->size.z >= lowLimit && z <= highLimit;
		}
	}

	if(this->children)
	{
		VirtualNode childNode = this->children->head;
		
		for(; childNode; childNode = childNode->next)
		{
			if(__VIRTUAL_CALL(bool, Entity, isVisible, VirtualNode_getData(childNode), pad))
			{
				return true;
			}
		}
	}
	
	return false;
}

// check if necessary to update sprite's position
bool Entity_updateSpritePosition(Entity this)
{
	ASSERT(this, "Entity::updateSpritePosition: null this");
	return ((_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z) || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z);
}

// check if necessary to update sprite's scale
bool Entity_updateSpriteTransformations(Entity this)
{
	ASSERT(this, "Entity::updateSpriteTransformations: null this");

	return (_screenDisplacement->z || this->invalidateGlobalPosition.z);
}

// set the direction
void Entity_setSpritesDirection(Entity this, int axis, int direction)
{
	ASSERT(this, "Entity::setSpritesDirection: null this");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(void, Sprite, setDirection, __SAFE_CAST(Sprite, node->data), axis, direction);
		}
	}
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
	ASSERT(this, "Entity::show: null this");

	Container_show(__SAFE_CAST(Container, this));
	
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// move each child to a temporary list
		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(void, Sprite, show, __SAFE_CAST(Sprite, node->data));
		}
	}
}

// make it invisible
void Entity_hide(Entity this)
{
	ASSERT(this, "Entity::hide: null this");

	Container_hide(__SAFE_CAST(Container, this));
	
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// move each child to a temporary list
		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(void, Sprite, hide, __SAFE_CAST(Sprite, node->data));
		}
	}
}

// suspend for pause
void Entity_suspend(Entity this)
{
	ASSERT(this, "Entity::suspend: null this");

	Container_suspend(__SAFE_CAST(Container, this));
	Entity_releaseSprites(this);
}

// resume after pause
void Entity_resume(Entity this)
{
	ASSERT(this, "Entity::resume: null this");

	Container_resume(__SAFE_CAST(Container, this));

	// initialize sprites
	if(this->entityDefinition)
	{
		Entity_addSprites(this, this->entityDefinition->spritesDefinitions);
		Entity_translateSprites(this, true, true);
	}
	
	if(this->hidden)
	{
		Entity_hide(this);
	}
}

// defaults to true
bool Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration)
{
	return __XAXIS | __YAXIS | __ZAXIS;
}