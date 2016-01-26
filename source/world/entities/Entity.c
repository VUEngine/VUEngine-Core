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
#include <Game.h>
#include <InGameEntity.h>
#include <InanimatedInGameEntity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>
#include <BgmapSprite.h>
#include <MBgmapSprite.h>
#include <debugConfig.h>


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

VBVec3D centerDisplacement;
//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Entity, EntityDefinition* entityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(Entity, entityDefinition, id, name);


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
	this->centerDisplacement = NULL;

	// initialize to 0 for the engine to know that
	// size must be set
	this->size.x = 0;
	this->size.y = 0;
	this->size.z = 0;
	
	this->updateSprites = 0;
}

// class's destructor
void Entity_destructor(Entity this)
{
	ASSERT(this, "Entity::destructor: null this");

	// better to do it here than forget in other classes
	// unregister the shape for collision detection
	if(this->shape)
	{
		CollisionManager_unregisterShape(Game_getCollisionManager(Game_getInstance()), this->shape);
	
		this->shape = NULL;
	}
	
	if(this->centerDisplacement)
	{
		__DELETE_BASIC(this->centerDisplacement);
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

static void Entity_calculateSizeFromChildren(Entity this, const VBVec3D* environmentPosition, SmallRightCuboid* rightCuboid)
{
	ASSERT(this, "Entity::calculateSizeFromChildren: null this");

	VBVec3D globalPosition3D = this->transform.globalPosition;
	
	s16 left = 0;
	s16 right = 0;
	s16 top = 0;
	s16 bottom = 0;
	s16 front = 0;
	s16 back = 0;
	s16 halfWidth = 0;
	s16 halfHeight = 0;
	s16 halfDepth = 10;

	if((!this->size.x || !this->size.y || !this->size.z) && this->sprites)
	{
		VirtualNode spriteNode = this->sprites->head;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);

			Texture texture = Sprite_getTexture(sprite);

			if(texture)
			{
				halfWidth = Optics_calculateRealSize(((u16)Texture_getCols(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL_UNSAFE(Scale, Sprite, getScale, sprite).x)) >> 1;
				halfHeight = Optics_calculateRealSize(((u16)Texture_getRows(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL_UNSAFE(Scale, Sprite, getScale, sprite).y)) >> 1;
				halfDepth = this->size.z >> 1;
			}				
			
			VBVec3D spriteDisplacement = Sprite_getDisplacement(sprite);
			if(left > -halfWidth + FIX19_13TOI(spriteDisplacement.x))
			{
				left = -halfWidth + FIX19_13TOI(spriteDisplacement.x);
			}

			if(right < halfWidth + FIX19_13TOI(spriteDisplacement.x))
			{
				right = halfWidth + FIX19_13TOI(spriteDisplacement.x);
			}

			if(top > -halfHeight + FIX19_13TOI(spriteDisplacement.y))
			{
				top = -halfHeight + FIX19_13TOI(spriteDisplacement.y);
			}

			if(bottom < halfHeight + FIX19_13TOI(spriteDisplacement.y))
			{
				bottom = halfHeight + FIX19_13TOI(spriteDisplacement.y);
			}

			if(front > -halfDepth + FIX19_13TOI(spriteDisplacement.z))
			{
				front = -halfDepth + FIX19_13TOI(spriteDisplacement.z);
			}

			if(back < halfDepth + FIX19_13TOI(spriteDisplacement.z))
			{
				back = halfDepth + FIX19_13TOI(spriteDisplacement.z);
			}
		}
	}
	else
	{
		
		right = this->size.x >> 1;
		left = -right;
		bottom = this->size.y >> 1;
		top = -bottom;
		front = 0;
		back = this->size.z;
	}

	int x = FIX19_13TOI(globalPosition3D.x);
	int y = FIX19_13TOI(globalPosition3D.y);
	int z = FIX19_13TOI(globalPosition3D.z);
	
	if(0 == rightCuboid->x0 || x + left < rightCuboid->x0)
	{
		rightCuboid->x0 = x + left;
	}
	
	if(0 == rightCuboid->x1 || right + x > rightCuboid->x1)
	{
		rightCuboid->x1 = right + x;
	}

	if(0 == rightCuboid->y0 || y + top < rightCuboid->y0)
	{
		rightCuboid->y0 = y + top;
	}
	
	if(0 == rightCuboid->y1 || bottom + y > rightCuboid->y1)
	{
		rightCuboid->y1 = bottom + y;
	}

	if(0 == rightCuboid->z0 || z + front < rightCuboid->z0)
	{
		rightCuboid->z0 = z + front;
	}
	
	if(0 == rightCuboid->z1 || back + z > rightCuboid->z1)
	{
		rightCuboid->z1 = back + z;
	}

	if(this->children)
	{
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Entity_calculateSizeFromChildren(__SAFE_CAST(Entity, childNode->data), &globalPosition3D, rightCuboid);
		}
	}
}

// calculate my size based on me and my children
void Entity_calculateSize(Entity this)
{
	ASSERT(this, "Entity::calculateSize: null this");

	SmallRightCuboid rightCuboid = {0, 0, 0, 0, 0, 0};

	VBVec3D environmentPosition = {0, 0, 0};
	Entity_calculateSizeFromChildren(this, &environmentPosition, &rightCuboid);

	VBVec3D centerDisplacement =
	{
		(ITOFIX19_13(rightCuboid.x1 + rightCuboid.x0) >> 1) - this->transform.globalPosition.x,
		(ITOFIX19_13(rightCuboid.y1 + rightCuboid.y0) >> 1) - this->transform.globalPosition.y,
		(ITOFIX19_13(rightCuboid.z1 + rightCuboid.z0) >> 1) - this->transform.globalPosition.z
	};
	
	if(centerDisplacement.x || centerDisplacement.y || centerDisplacement.z)
	{
		this->centerDisplacement = __NEW_BASIC(VBVec3D);
		*this->centerDisplacement = centerDisplacement;
	}
	
	this->size.x = rightCuboid.x1 - rightCuboid.x0;
	this->size.y = rightCuboid.y1 - rightCuboid.y0;
	this->size.z = rightCuboid.z1 - rightCuboid.z0;
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
	
	s16 left = 0;
	s16 right = 0;
	s16 top = 0;
	s16 bottom = 0;
	s16 front = 0;
	s16 back = 0;
	s16 halfWidth = 0;
	s16 halfHeight = 0;
	s16 halfDepth = 5;

	if(positionedEntity->entityDefinition->spritesDefinitions && positionedEntity->entityDefinition->spritesDefinitions[0])
	{
		int i = 0;
		
		for(; positionedEntity->entityDefinition->spritesDefinitions[i]; i++)
		{
			if(__TYPE(MBgmapSprite) == positionedEntity->entityDefinition->spritesDefinitions[i]->allocator && ((MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i])->textureDefinitions[0])
			{
				MBgmapSpriteDefinition* mBgmapSpriteDefinition = (MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i]; 

				int j = 0;
				
				halfWidth = 0;
				halfHeight = 0;
				halfDepth = 0;
				
				for(; mBgmapSpriteDefinition->textureDefinitions[j]; j++)
				{	
					if(halfWidth < mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2)
					{
						halfWidth = mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2;
					}

					if(halfHeight < mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2)
					{
						halfHeight = mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2;
					}
				}
				
				if(left > -halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.x))
				{
					left = -halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.x);
				}
	
				if(right < halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.x))
				{
					right = halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.x);
				}
	
				if(top > -halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.y))
				{
					top = -halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.y);
				}
	
				if(bottom < halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.y))
				{
					bottom = halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.y);
				}
	
				if(front > FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.z))
				{
					front = FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.z);
				}
	
				if(back < halfDepth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.z))
				{
					back = halfDepth + FIX19_13TOI(mBgmapSpriteDefinition->bSpriteDefinition.displacement.z);
				}
				
			}
			else if(positionedEntity->entityDefinition->spritesDefinitions[i]->textureDefinition)
			{
				SpriteDefinition* bgmapSpriteDefinition = (SpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i];
				halfWidth = bgmapSpriteDefinition->textureDefinition->cols << 2;
				halfHeight = bgmapSpriteDefinition->textureDefinition->rows << 2;
				halfDepth = 10;
				
				if(left > -halfWidth + FIX19_13TOI(bgmapSpriteDefinition->displacement.x))
				{
					left = -halfWidth + FIX19_13TOI(bgmapSpriteDefinition->displacement.x);
				}
	
				if(right < halfWidth + FIX19_13TOI(bgmapSpriteDefinition->displacement.x))
				{
					right = halfWidth + FIX19_13TOI(bgmapSpriteDefinition->displacement.x);
				}
	
				if(top > -halfHeight + FIX19_13TOI(bgmapSpriteDefinition->displacement.y))
				{
					top = -halfHeight + FIX19_13TOI(bgmapSpriteDefinition->displacement.y);
				}
	
				if(bottom < halfHeight + FIX19_13TOI(bgmapSpriteDefinition->displacement.y))
				{
					bottom = halfHeight + FIX19_13TOI(bgmapSpriteDefinition->displacement.y);
				}
	
				if(front > -halfDepth + FIX19_13TOI(bgmapSpriteDefinition->displacement.z))
				{
					front = -halfDepth + FIX19_13TOI(bgmapSpriteDefinition->displacement.z);
				}
	
				if(back < (halfDepth << 1) + FIX19_13TOI(bgmapSpriteDefinition->displacement.z))
				{
					back = (halfDepth << 1) + FIX19_13TOI(bgmapSpriteDefinition->displacement.z);
				}
			}
		}
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
			
			left = -halfWidth;
			right = halfWidth;
			top = -halfHeight;
			bottom = halfHeight;
			front = -halfDepth;
			back = halfDepth;
		}
		else
		{
			left = -1;
			right = 1;
			top = -1;
			bottom = 1;
			front = -1;
			back = 1;
		}
	}

	int x = FIX19_13TOI(globalPosition3D.x);
	int y = FIX19_13TOI(globalPosition3D.y);
	int z = FIX19_13TOI(globalPosition3D.z);
	
	if(0 == rightCuboid->x0 || x + left < rightCuboid->x0)
	{
		rightCuboid->x0 = x + left;
	}
	
	if(0 == rightCuboid->x1 || right + x > rightCuboid->x1)
	{
		rightCuboid->x1 = right + x;
	}

	if(0 == rightCuboid->y0 || y + top < rightCuboid->y0)
	{
		rightCuboid->y0 = y + top;
	}
	
	if(0 == rightCuboid->y1 || bottom + y > rightCuboid->y1)
	{
		rightCuboid->y1 = bottom + y;
	}

	if(0 == rightCuboid->z0 || z + front < rightCuboid->z0)
	{
		rightCuboid->z0 = z + front;
	}
	
	if(0 == rightCuboid->z1 || back + z > rightCuboid->z1)
	{
		rightCuboid->z1 = back + z;
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

	rightCuboid.x0 = rightCuboid.x0 - FIX19_13TOI(positionedEntity->position.x);
	rightCuboid.x1 = rightCuboid.x1 - FIX19_13TOI(positionedEntity->position.x);
	rightCuboid.y0 = rightCuboid.y0 - FIX19_13TOI(positionedEntity->position.y);
	rightCuboid.y1 = rightCuboid.y1 - FIX19_13TOI(positionedEntity->position.y);
	rightCuboid.z0 = rightCuboid.z0 - FIX19_13TOI(positionedEntity->position.z);
	rightCuboid.z1 = rightCuboid.z1 - FIX19_13TOI(positionedEntity->position.z);

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
		// call the appropriate allocator to support inheritance
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
}

// entity is initialized
void Entity_ready(Entity this)
{
	ASSERT(this, "Entity::initialize: null this");
	
	if(this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			__VIRTUAL_CALL(void, Entity, ready, __SAFE_CAST(Entity, childNode->data));
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
		// call the appropriate allocator to support inheritance
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

// update sprites
void Entity_updateSprites(Entity this, bool updateSpriteTransformations, bool updateSpritePosition)
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

	// force invalid position in all children
	Container_invalidateGlobalPosition(__SAFE_CAST(Container, this), __XAXIS | __YAXIS | __ZAXIS);

	// call base class's transform method
	Container_initialTransform(__SAFE_CAST(Container, this), environmentTransform);

	// now can calculate the size
	if(!this->size.x || !this->size.y || !this->size.z)
	{
		// must force size calculation now
		Entity_calculateSize(this);
	}
	
	this->updateSprites = __UPDATE_SPRITE_POSITION | __UPDATE_SPRITE_TRANSFORMATIONS;
	Container_invalidateGlobalPosition(this, __XAXIS | __YAXIS | __ZAXIS);

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
	
	if(this->invalidateGlobalPosition.x ||
		this->invalidateGlobalPosition.y ||
		this->invalidateGlobalPosition.z ||
		this->children)
	{
		this->updateSprites |= __UPDATE_SPRITE_POSITION;
		this->updateSprites |= this->invalidateGlobalPosition.z? __UPDATE_SPRITE_TRANSFORMATIONS : 0;

		// call base class's transform method
		Container_transform(__SAFE_CAST(Container, this), environmentTransform);
	}
	else
	{
		this->updateSprites |= __VIRTUAL_CALL(bool, Entity, updateSpritePosition, this)? __UPDATE_SPRITE_POSITION : 0;
		this->updateSprites |= __VIRTUAL_CALL(bool, Entity, updateSpriteTransformations, this)? __UPDATE_SPRITE_TRANSFORMATIONS : 0;
	}
}

void Entity_updateVisualRepresentation(Entity this)
{
	ASSERT(this, "Entity::updateVisualRepresentation: null this");

	Container_updateVisualRepresentation(__SAFE_CAST(Container, this));
	
	Entity_updateSprites(this, this->updateSprites & __UPDATE_SPRITE_TRANSFORMATIONS, this->updateSprites & __UPDATE_SPRITE_POSITION);

	this->updateSprites = 0;

	/*
	if(this->shape)
	{
		__VIRTUAL_CALL(void, Shape, draw, this->shape);
	}
	*/
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
bool Entity_isVisible(Entity this, int pad, bool recursive)
{
	ASSERT(this, "Entity::isVisible: null this");

	int x = 0;
	int y = 0;
	int z = 0;
	
	if(this->sprites)
	{
		VirtualNode spriteNode = this->sprites->head;
		
		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
			ASSERT(sprite, "Entity:isVisible: null sprite");

			VBVec2D spritePosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
			
			x = FIX19_13TOI(spritePosition.x);
			y = FIX19_13TOI(spritePosition.y);
			z = FIX19_13TOI(spritePosition.z);

			// check x visibility
			if(x + this->size.x >= -pad && x <= __SCREEN_WIDTH + pad)
			{
				// check y visibility
				if(y + this->size.y >= -pad && y <= __SCREEN_HEIGHT + pad)
				{
					// check z visibility
					if(z + this->size.z >= -pad && z <= __SCREEN_DEPTH + pad)
					{
						return true;
					}
				}
			}
		}
	}
	else 
	{
		VBVec3D position3D = this->transform.globalPosition;

		if(this->centerDisplacement)
		{
			position3D.x += this->centerDisplacement->x;
			position3D.y += this->centerDisplacement->y;
			position3D.z += this->centerDisplacement->z;
		}
		
		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		VBVec2D position2D;
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		s16 halfWidth = this->size.x >> 1;
		s16 halfHeight = this->size.y >> 1;
		s16 halfDepth = this->size.z >> 1;

		x = FIX19_13TOI(position2D.x);
		y = FIX19_13TOI(position2D.y);
		z = FIX19_13TOI(position2D.z);
		
		if(x + halfWidth > -pad && x - halfWidth < __SCREEN_WIDTH + pad)
		{
			// check y visibility
			if(y + halfHeight > -pad && y - halfHeight < __SCREEN_HEIGHT + pad)
			{
				// check z visibility
				if(z + halfDepth > -pad && z - halfDepth < __SCREEN_DEPTH + pad)
				{
					return true;
				}
			}
		}
	}

	if(recursive && this->children)
	{
		VirtualNode childNode = this->children->head;
		
		for(; childNode; childNode = childNode->next)
		{
			if(__VIRTUAL_CALL(bool, Entity, isVisible, VirtualNode_getData(childNode), pad, true))
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
	
	// force invalid position to update sprites and children's sprites
	Container_invalidateGlobalPosition(__SAFE_CAST(Container, this), __XAXIS | __YAXIS | __ZAXIS);
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
	}
	
	if(this->hidden)
	{
		Entity_hide(this);
	}

	// force update sprites on next game's cycle
	this->updateSprites = __UPDATE_SPRITE_POSITION | __UPDATE_SPRITE_TRANSFORMATIONS;
}

// defaults to true
bool Entity_canMoveOverAxis(Entity this, const Acceleration* acceleration)
{
	return __XAXIS | __YAXIS | __ZAXIS;
}