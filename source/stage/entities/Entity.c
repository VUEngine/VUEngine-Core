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
extern const VBVec3D* _screenDisplacement;
extern const Optical* _optical;

u32 EntityFactory_instantiateEntities(EntityFactory this);
u32 EntityFactory_initializeEntities(EntityFactory this);
u32 EntityFactory_transformEntities(EntityFactory this);
u32 EntityFactory_makeReadyEntities(EntityFactory this);
u32 EntityFactory_callLoadedEntities(EntityFactory this);

static void Entity_addSprites(Entity this, const SpriteDefinition** spritesDefinitions);
static void Entity_releaseSprites(Entity this);
static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation);
static void Entity_setupShape(Entity this);

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
	__CONSTRUCT_BASE(Container, id, name);

	// save definition
	this->entityDefinition = entityDefinition;

	// the sprite must be initialized in the derived class
	this->sprites = NULL;
	this->shape = NULL;
	this->centerDisplacement = NULL;
	this->entityFactory = NULL;

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

	if(this->entityFactory)
	{
	    __DELETE(this->entityFactory);
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

static void Entity_calculateSizeFromChildren(Entity this, SmallRightCuboid* rightCuboid, VBVec3D environmentPosition)
{
	ASSERT(this, "Entity::calculateSizeFromChildren: null this");

	VBVec3D globalPosition3D = environmentPosition;

	globalPosition3D.x += this->transform.localPosition.x;
	globalPosition3D.y += this->transform.localPosition.y;
	globalPosition3D.z += this->transform.localPosition.z;

	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
	int front = 0;
	int back = 0;
	int halfWidth = 0;
	int halfHeight = 0;
	int halfDepth = 10;

	if((!this->size.x || !this->size.y || !this->size.z) && this->sprites)
	{
		VirtualNode spriteNode = this->sprites->head;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
        	ASSERT(sprite, "Entity::calculateSizeFromChildren: null sprite");
//            __VIRTUAL_CALL(Sprite, resize, sprite, this->transform.globalScale, this->transform.globalPosition.z);

//			halfWidth = Optics_calculateRealSize(((int)Texture_getCols(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL(Sprite, getScale, sprite).x)) >> 1;
//			halfHeight = Optics_calculateRealSize(((int)Texture_getRows(texture)) << 3, Sprite_getMode(sprite), abs(__VIRTUAL_CALL(Sprite, getScale, sprite).y)) >> 1;
            halfWidth = Sprite_getHalfWidth(sprite);
            halfHeight = Sprite_getHalfHeight(sprite);
            halfDepth = this->size.z >> 1;

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
			Entity_calculateSizeFromChildren(__SAFE_CAST(Entity, childNode->data), rightCuboid, globalPosition3D);
		}
	}
}

// calculate my size based on me and my children
void Entity_calculateSize(Entity this)
{
	ASSERT(this, "Entity::calculateSize: null this");

	SmallRightCuboid rightCuboid = {0, 0, 0, 0, 0, 0};

	Entity_calculateSizeFromChildren(this, &rightCuboid, (VBVec3D){0, 0, 0});

	VBVec3D centerDisplacement =
	{
		(ITOFIX19_13(rightCuboid.x1 + rightCuboid.x0) >> 1) - this->transform.localPosition.x,
		(ITOFIX19_13(rightCuboid.y1 + rightCuboid.y0) >> 1) - this->transform.localPosition.y,
		(ITOFIX19_13(rightCuboid.z1 + rightCuboid.z0) >> 1) - this->transform.localPosition.z
	};

	if(centerDisplacement.x || centerDisplacement.y || centerDisplacement.z)
	{
		if(this->centerDisplacement)
		{
			__DELETE_BASIC(this->centerDisplacement);
		}

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

	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
	int front = 0;
	int back = 0;
	int halfWidth = 0;
	int halfHeight = 0;
	int halfDepth = 5;

	if(positionedEntity->entityDefinition->spritesDefinitions && positionedEntity->entityDefinition->spritesDefinitions[0])
	{
		int i = 0;

		for(; positionedEntity->entityDefinition->spritesDefinitions[i]; i++)
		{
			if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(positionedEntity->entityDefinition->spritesDefinitions[i]->allocator && ((MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i])->textureDefinitions[0]))
			{
				MBgmapSpriteDefinition* mBgmapSpriteDefinition = (MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i];

				int j = 0;

				halfWidth = 0;
				halfHeight = 0;
				halfDepth = 0;

				for(; mBgmapSpriteDefinition->textureDefinitions[j]; j++)
				{
					if(halfWidth < (int)mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2)
					{
						halfWidth = mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2;
					}

					if(halfHeight < (int)mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2)
					{
						halfHeight = mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2;
					}
				}

				if(left > -halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x))
				{
					left = -halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x);
				}

				if(right < halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x))
				{
					right = halfWidth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x);
				}

				if(top > -halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y))
				{
					top = -halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y);
				}

				if(bottom < halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y))
				{
					bottom = halfHeight + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y);
				}

				if(front > FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z))
				{
					front = FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z);
				}

				if(back < halfDepth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z))
				{
					back = halfDepth + FIX19_13TOI(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z);
				}

			}
			else if(positionedEntity->entityDefinition->spritesDefinitions[i]->textureDefinition)
			{
				SpriteDefinition* spriteDefinition = (SpriteDefinition*)positionedEntity->entityDefinition->spritesDefinitions[i];
				halfWidth = spriteDefinition->textureDefinition->cols << 2;
				halfHeight = spriteDefinition->textureDefinition->rows << 2;
				halfDepth = 10;

				if(left > -halfWidth + FIX19_13TOI(spriteDefinition->displacement.x))
				{
					left = -halfWidth + FIX19_13TOI(spriteDefinition->displacement.x);
				}

				if(right < halfWidth + FIX19_13TOI(spriteDefinition->displacement.x))
				{
					right = halfWidth + FIX19_13TOI(spriteDefinition->displacement.x);
				}

				if(top > -halfHeight + FIX19_13TOI(spriteDefinition->displacement.y))
				{
					top = -halfHeight + FIX19_13TOI(spriteDefinition->displacement.y);
				}

				if(bottom < halfHeight + FIX19_13TOI(spriteDefinition->displacement.y))
				{
					bottom = halfHeight + FIX19_13TOI(spriteDefinition->displacement.y);
				}

				if(front > -halfDepth + FIX19_13TOI(spriteDefinition->displacement.z))
				{
					front = -halfDepth + FIX19_13TOI(spriteDefinition->displacement.z);
				}

				if(back < (halfDepth << 1) + FIX19_13TOI(spriteDefinition->displacement.z))
				{
					back = (halfDepth << 1) + FIX19_13TOI(spriteDefinition->displacement.z);
				}
			}
		}
	}
	else if(!positionedEntity->childrenDefinitions)
	{
		// TODO: there should be a class which handles these special cases
		if(__TYPE(InGameEntity) == __ALLOCATOR_TYPE(positionedEntity->entityDefinition->allocator) ||
		    __TYPE(InanimatedInGameEntity) == __ALLOCATOR_TYPE(positionedEntity->entityDefinition->allocator)
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

	if(!childrenDefinitions)
	{
	    return NULL;
	}

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

// instantiate and entity using the provided allocator
Entity Entity_instantiate(const EntityDefinition* const entityDefinition, int id, const char* const name, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::load: null definition");
	ASSERT(entityDefinition->allocator, "Entity::load: no allocator defined");

	if(!entityDefinition || !entityDefinition->allocator)
	{
	    return NULL;
	}

    // call the appropriate allocator to support inheritance
    Entity entity = ((Entity (*)(EntityDefinition*, s16, const char* const)) entityDefinition->allocator)((EntityDefinition*)entityDefinition, id, name);

    // process extra info
    if(extraInfo)
    {
        __VIRTUAL_CALL(Entity, setExtraInfo, entity, extraInfo);
    }

    return entity;
}

// add children to the instance from the definitions array
void Entity_addChildEntities(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if(!childrenDefinitions)
	{
	    return;
	}

    int i = 0;

    //go through n sprites in entity's definition
    for(; childrenDefinitions[i].entityDefinition; i++)
    {
        Entity child = Entity_loadEntity(&childrenDefinitions[i], this->id + Container_getChildCount(__SAFE_CAST(Container, this)));
    	ASSERT(child, "Entity::loadChildren: entity not loaded");

        // create the entity and add it to the world
        Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, child));
	}
}

// load an entity and instantiate all its children
Entity Entity_loadEntity(const PositionedEntity* const positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");

	if(!positionedEntity)
	{
	    return NULL;
	}

    Entity entity = Entity_instantiate(positionedEntity->entityDefinition, id, positionedEntity->name, positionedEntity->extraInfo);
	ASSERT(entity, "Entity::loadFromDefinition: entity not loaded");

    // set spatial position
    __VIRTUAL_CALL(Container, setLocalPosition, entity, &positionedEntity->position);

    // add children if defined
    if(positionedEntity->childrenDefinitions)
    {
        Entity_addChildEntities(entity, positionedEntity->childrenDefinitions);
    }

    return entity;
}

// add children to instance from the definitions array, but deferredly
void Entity_addChildEntitiesDeferred(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::addChildrenWithoutInitilization: null this");
	ASSERT(childrenDefinitions, "Entity::addChildrenWithoutInitilization: null childrenDefinitions");

	if(!childrenDefinitions)
	{
	    return;
	}

	if(!this->entityFactory)
	{
	    this->entityFactory = __NEW(EntityFactory);
	}

    int i = 0;

    //go through n sprites in entity's definition
    for(; childrenDefinitions[i].entityDefinition; i++)
    {
        EntityFactory_spawnEntity(this->entityFactory, &childrenDefinitions[i], __SAFE_CAST(Container, this), NULL, this->id + Container_getChildCount(__SAFE_CAST(Container, this)));
    }
}

// load an entity and instantiate all its children deferredly
Entity Entity_loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 id)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinitionWithoutInitilization: null positionedEntity");

    if(!positionedEntity)
    {
        return NULL;
    }

    Entity entity = Entity_instantiate(positionedEntity->entityDefinition, id, positionedEntity->name, positionedEntity->extraInfo);
	ASSERT(entity, "Entity::loadFromDefinitionWithoutInitilization: entity not loaded");

    if(positionedEntity->name)
    {
        Container_setName(__SAFE_CAST(Container, entity), positionedEntity->name);
    }

    // set spatial position
    __VIRTUAL_CALL(Container, setLocalPosition, entity, &positionedEntity->position);

    // add children if defined
    if(positionedEntity->childrenDefinitions)
    {
        Entity_addChildEntitiesDeferred(entity, positionedEntity->childrenDefinitions);
    }

    return entity;
}

// add child entity from definition
Entity Entity_addChildEntity(Entity this, const EntityDefinition* entityDefinition, int id, const char* name, const VBVec3D* position, void* extraInfo)
{
	ASSERT(this, "Entity::addChildFromDefinition: null this");
	ASSERT(entityDefinition, "Entity::addChildFromDefinition: null entityDefinition");

	if(!entityDefinition)
	{
	    return NULL;
	}

	PositionedEntity positionedEntity =
	{
		(EntityDefinition*)entityDefinition,
		{position->x, position->y, position->z},
		(char*)name,
		NULL,
		extraInfo,
		false
	};

    // create the hint entity and add it to the hero as a child entity
	Entity childEntity = Entity_loadEntity(&positionedEntity, 0 > id? id: this->id + Container_getChildCount(__SAFE_CAST(Container, this)));
	ASSERT(childEntity, "Entity::addChildFromDefinition: childEntity no created");

    // must initialize after adding the children
    __VIRTUAL_CALL(Entity, initialize, childEntity, true);

    // if already initialized
    if(this->size.x && this->size.y && this->size.z)
    {
        Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));

         // apply transformations
        __VIRTUAL_CALL(Container, initialTransform, childEntity, &environmentTransform, true);
    }

    // create the entity and add it to the world
    Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, childEntity));

    __VIRTUAL_CALL(Entity, ready, childEntity, true);

	return childEntity;
}

u32 Entity_areAllChildrenInstantiated(Entity this)
{
	ASSERT(this, "Entity::allChildrenSpawned: null this");

    if(this->entityFactory)
    {
        return __LIST_EMPTY == EntityFactory_instantiateEntities(this->entityFactory);
    }

    return true;
}

u32 Entity_areAllChildrenInitialized(Entity this)
{
	ASSERT(this, "Entity::allChildrenInitialized: null this");

    if(this->entityFactory)
    {
        return __LIST_EMPTY == EntityFactory_initializeEntities(this->entityFactory);
    }

    return true;
}

u32 Entity_areAllChildrenTransformed(Entity this)
{
	ASSERT(this, "Entity::allChildrenInitialized: null this");

    if(this->entityFactory)
    {
        return __LIST_EMPTY == EntityFactory_transformEntities(this->entityFactory);
    }

    return true;
}

u32 Entity_areAllChildrenReady(Entity this)
{
	ASSERT(this, "Entity::allChildrenReady: null this");

    if(this->entityFactory)
    {
        u32 returnValue = __LIST_EMPTY == EntityFactory_makeReadyEntities(this->entityFactory);

        if(!EntityFactory_hasEntitiesPending(this->entityFactory))
        {
            __DELETE(this->entityFactory);
            this->entityFactory = NULL;

            // must force size calculation now that all children are loaded
            Entity_calculateSize(this);
        }

        return returnValue;
    }

    Entity_calculateSize(this);

    return true;
}

static void Entity_setupShape(Entity this)
{
	ASSERT(this, "Entity::setupShape: null this");

	if(this->shape)
	{
		// setup shape
		__VIRTUAL_CALL(Shape, setup, this->shape);

		if(__VIRTUAL_CALL(Entity, moves, this))
	    {
			__VIRTUAL_CALL(Shape, position, this->shape);
		}

		Shape_setActive(this->shape, true);
	}
}

// initialize from definition
void Entity_initialize(Entity this, u32 recursive)
{
	ASSERT(this, "Entity::initialize: null this");

	if(!this->sprites)
	{
		Entity_addSprites(this, this->entityDefinition->spritesDefinitions);
	}

	if(recursive && this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Entity, initialize, __SAFE_CAST(Entity, node->data), recursive);
		}
	}
}

// entity is initialized
void Entity_ready(Entity this, u32 recursive)
{
	ASSERT(this, "Entity::initialize: null this");

	if(recursive && this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			__VIRTUAL_CALL(Entity, ready, __SAFE_CAST(Entity, childNode->data), recursive);
		}
	}
}

// process extra info in intialization
void Entity_setExtraInfo(Entity this __attribute__ ((unused)), void* extraInfo __attribute__ ((unused)))
{
	ASSERT(this, "Entity::setExtraInfo: null this");
}

// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition** spritesDefinitions)
{
	ASSERT(this, "Entity::addSprites: null this");

    if(!spritesDefinitions)
    {
        return;
    }

    int i = 0;

    if(!this->sprites)
    {
        this->sprites = __NEW(VirtualList);
    }

    //go through n sprites in entity's definition
    for(; spritesDefinitions[i]; i++)
    {
    	ASSERT(spritesDefinitions[i]->allocator, "Entity::addSprites: no sprite allocator");

        Sprite sprite = ((Sprite (*)(SpriteDefinition*, Object)) spritesDefinitions[i]->allocator)((SpriteDefinition*)spritesDefinitions[i], __SAFE_CAST(Object, this));
        ASSERT(sprite, "Entity::addSprite: sprite not created");

        VirtualList_pushBack(this->sprites, (void*)sprite);
    }
}

// add sprite
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition)
{
	ASSERT(this, "Entity::addSprite: null this");

	ASSERT(spriteDefinition, "Entity::addSprite: null spriteDefinition");
	ASSERT(spriteDefinition->allocator, "Entity::addSprite: no sprite allocator");

	if(!spriteDefinition || !spriteDefinition->allocator)
	{
	    return;
    }

	Sprite sprite = NULL;

    // call the appropriate allocator to support inheritance
    sprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
    ASSERT(sprite, "Entity::addSprite: sprite not created");

    if(!this->sprites)
    {
        this->sprites = __NEW(VirtualList);
    }

    VirtualList_pushBack(this->sprites, (void*)sprite);
}

// update sprites
static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation)
{
	ASSERT(this, "Entity::transform: null this");

	if(!this->sprites)
	{
	    return;
    }

    updatePosition |= updateRotation;

    VirtualNode node = this->sprites->head;

    // move each child to a temporary list
    for(; node ; node = node->next)
    {
        Sprite sprite = __SAFE_CAST(Sprite, node->data);

        if(updatePosition)
        {
            // update sprite's 2D position
            __VIRTUAL_CALL(Sprite, position, sprite, &this->transform.globalPosition);
        }

        if(updateRotation)
        {
            __VIRTUAL_CALL(Sprite, rotate, sprite, &this->transform.globalRotation);
        }

        if(updateScale)
        {
            // calculate the scale
            __VIRTUAL_CALL(Sprite, resize, sprite, this->transform.globalScale, this->transform.globalPosition.z);

            // calculate sprite's parallax
            __VIRTUAL_CALL(Sprite, calculateParallax, sprite, this->transform.globalPosition.z);
        }
    }
}

// initial transformation
void Entity_initialTransform(Entity this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "Entity::initialTransform: null this");

	// call base class's transform method
	Container_initialTransform(__SAFE_CAST(Container, this), environmentTransform, recursive);

	this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;

	if(this->hidden)
	{
		Entity_hide(this);
	}

    // now can calculate the size
    if(recursive && (!this->size.x || !this->size.y || !this->size.z))
    {
        // must force size calculation now
        Entity_calculateSize(this);
    }

    Entity_setupShape(this);
}

// transform class
void Entity_transform(Entity this, const Transformation* environmentTransform)
{
	ASSERT(this, "Entity::transform: null this");

    this->updateSprites = 0;
    this->updateSprites |= __VIRTUAL_CALL(Entity, updateSpritePosition, this)? __UPDATE_SPRITE_POSITION : 0;
    this->updateSprites |= __VIRTUAL_CALL(Entity, updateSpriteRotation, this)? __UPDATE_SPRITE_ROTATION : 0;
    this->updateSprites |= __VIRTUAL_CALL(Entity, updateSpriteScale, this)? __UPDATE_SPRITE_SCALE : 0;

    // call base class's transform method
    Container_transform(__SAFE_CAST(Container, this), environmentTransform);
}

void Entity_updateVisualRepresentation(Entity this)
{
	ASSERT(this, "Entity::updateVisualRepresentation: null this");

	Container_updateVisualRepresentation(__SAFE_CAST(Container, this));

	Entity_updateSprites(this, this->updateSprites & __UPDATE_SPRITE_POSITION, this->updateSprites & __UPDATE_SPRITE_SCALE, this->updateSprites & __UPDATE_SPRITE_ROTATION);

	this->updateSprites = 0;

/*
	if(this->shape)
	{
		__VIRTUAL_CALL(Shape, draw, this->shape);
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
bool Entity_handleMessage(Entity this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	ASSERT(this, "Entity::handleMessage: null this");

	return false;
}

// get width
int Entity_getWidth(Entity this)
{
	ASSERT(this, "Entity::getWidth: null this");

	if(!this->size.x)
	{
		Entity_calculateSize(this);
	}

	// must calculate based on the scale because not affine Container must be enlarged
	return (int)this->size.x;
}

// get height
int Entity_getHeight(Entity this)
{
	ASSERT(this, "Entity::getHeight: null this");

	if(!this->size.y)
	{
		Entity_calculateSize(this);
	}

	return (int)this->size.y;
}

// get depth
int Entity_getDepth(Entity this)
{
	ASSERT(this, "Entity::getDepth: null this");

	if(!this->size.z)
	{
		Entity_calculateSize(this);
	}

	// must calculate based on the scale because not affine object must be enlarged
	return (int)this->size.z;
}

// retrieve gap
Gap Entity_getGap(Entity this __attribute__ ((unused)))
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

			VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, sprite);

			x = FIX19_13TOI(spritePosition.x);
			y = FIX19_13TOI(spritePosition.y);
			z = FIX19_13TOI(spritePosition.z);

			// check x visibility
			if(x + (int)this->size.x >= -pad && x <= __SCREEN_WIDTH + pad)
			{
				// check y visibility
				if(y + (int)this->size.y >= -pad && y <= __SCREEN_HEIGHT + pad)
				{
					// check z visibility
					if(z + (int)this->size.z >= -pad && z <= __SCREEN_DEPTH + pad)
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

		int halfWidth = (int)this->size.x >> 1;
		int halfHeight = (int)this->size.y >> 1;
		int halfDepth = (int)this->size.z >> 1;

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
			if(__VIRTUAL_CALL(Entity, isVisible, VirtualNode_getData(childNode), pad, true))
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

	return (_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z) || (__INVALIDATE_POSITION & this->invalidateGlobalTransformation);
}

// check if necessary to update sprite's rotation
bool Entity_updateSpriteRotation(Entity this)
{
	ASSERT(this, "Entity::updateSpriteRotation: null this");

	return this->invalidateGlobalTransformation & __INVALIDATE_ROTATION;
}

// check if necessary to update sprite's scale
bool Entity_updateSpriteScale(Entity this)
{
	ASSERT(this, "Entity::updateSpriteScale: null this");

	return (_screenDisplacement->z || (this->invalidateGlobalTransformation & (__ZAXIS | __INVALIDATE_SCALE)));
}

// set the direction
void Entity_setSpritesDirection(Entity this, int axis, int direction)
{
	ASSERT(this, "Entity::setSpritesDirection: null this");

	u32 axisForFlipping = __VIRTUAL_CALL(Entity, getAxisForFlipping, this);

	if(this->sprites && (axis & axisForFlipping))
	{
		VirtualNode node = this->sprites->head;

		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(Sprite, setDirection, __SAFE_CAST(Sprite, node->data), axis, direction);
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

    // update transformation before hiding
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
    __VIRTUAL_CALL(Container, transform, this, &environmentTransform);

    // and update the visual representation
    this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;
    Entity_updateVisualRepresentation(this);

	Container_show(__SAFE_CAST(Container, this));

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// move each child to a temporary list
		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(Sprite, show, __SAFE_CAST(Sprite, node->data));
		}
	}

	// force invalid position to update sprites and children's sprites
	Container_invalidateGlobalTransformation(__SAFE_CAST(Container, this));
}

// make it invisible
void Entity_hide(Entity this)
{
	ASSERT(this, "Entity::hide: null this");

    // update transformation before hiding
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
    __VIRTUAL_CALL(Container, transform, this, &environmentTransform);

    // and update the visual representation
    this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;
    Entity_updateVisualRepresentation(this);

	Container_hide(__SAFE_CAST(Container, this));

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// move each child to a temporary list
		for(; node ; node = node->next)
	    {
			__VIRTUAL_CALL(Sprite, hide, __SAFE_CAST(Sprite, node->data));
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
	this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;
}

// defaults to true
int Entity_canMoveOverAxis(Entity this __attribute__ ((unused)), const Acceleration* acceleration __attribute__ ((unused)))
{
	return __XAXIS | __YAXIS | __ZAXIS;
}

u32 Entity_getAxisForFlipping(Entity this __attribute__ ((unused)))
{
	return __XAXIS | __YAXIS;
}
