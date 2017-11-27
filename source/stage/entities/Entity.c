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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Entity.h>
#include <Screen.h>
#include <Game.h>
#include <Entity.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>
#include <SpriteManager.h>
#include <BgmapSprite.h>
#include <MBgmapSprite.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Entity
 * @extends	Container
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(Entity, Container);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

u32 EntityFactory_instantiateEntities(EntityFactory this);
u32 EntityFactory_initializeEntities(EntityFactory this);
u32 EntityFactory_transformEntities(EntityFactory this);
u32 EntityFactory_makeReadyEntities(EntityFactory this);
u32 EntityFactory_callLoadedEntities(EntityFactory this);

static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation);
static void Entity_addShapes(Entity this, const ShapeDefinition* shapeDefinitions, bool destroyPreviousShapes);
static void Entity_destroyShapes(Entity this);

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Entity, EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(Entity, entityDefinition, id, internalId, name);

/**
 * Class constructor
 *
 * @memberof				Entity
 * @public
 *
 * @param this				Function scope
 * @param entityDefinition
 * @param id
 * @param internalId
 * @param name
 */
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "Entity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Container, name);

	// set the ids
	this->id = id;
	this->internalId = internalId;

	// save definition
	this->entityDefinition = entityDefinition;

	// the sprite must be initialized in the derived class
	this->sprites = NULL;
	this->shapes = NULL;
	this->centerDisplacement = NULL;
	this->entityFactory = NULL;

	// initialize to 0 for the engine to know that size must be set
	this->size = entityDefinition->size;

	this->invalidateSprites = 0;
}

/**
 * Class destructor
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_destructor(Entity this)
{
	ASSERT(this, "Entity::destructor: null this");

	Entity_destroyShapes(this);

	if(this->centerDisplacement)
	{
		__DELETE_BASIC(this->centerDisplacement);
	}

	if(this->entityFactory)
	{
		__DELETE(this->entityFactory);
	}

	Entity_releaseSprites(this, true);

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Retrieve instance's in game id
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Internal ID
 */
s16 Entity_getInternalId(Entity this)
{
	ASSERT(this, "Entity::getInternalId: null this");

	return this->internalId;
}

/**
 * Retrieve instance's id as per its definition
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		ID
 */
s16 Entity_getId(Entity this)
{
	ASSERT(this, "Entity::getId: null this");

	return this->id;
}

/**
 * Get child by id
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 * @param id
 *
 * @return		Child Entity
 */
Entity Entity_getChildById(Entity this, s16 id)
{
	ASSERT(this, "Entity::getChildById: null this");

	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(__SAFE_CAST(Container, this));

		VirtualNode node = this->children->head;

		// look through all children
		for(; node ; node = node->next)
		{
			Entity child = __SAFE_CAST(Entity, node->data);

			if(Entity_getId(child) == id)
			{
				return child;
			}
		}
	}

	return NULL;
}

/**
 * Set definition
 *
 * @memberof				Entity
 * @public
 *
 * @param this				Function scope
 * @param entityDefinition
 */
void Entity_setDefinition(Entity this, void* entityDefinition)
{
	ASSERT(this, "Entity::setDefinition: null this");

	// save definition
	this->entityDefinition = entityDefinition;
}

/**
 * Destroy shapes
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
static void Entity_destroyShapes(Entity this)
{
	ASSERT(this, "Entity::setDefinition: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			CollisionManager_destroyShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(Shape, node->data));
		}

		__DELETE(this->shapes);
		this->shapes = NULL;
	}
}

/**
 * Add sprites
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_setupGraphics(Entity this)
{
	ASSERT(this, "Entity::setupGraphics: null this");

	__CALL_BASE_METHOD(Container, setupGraphics, this);

	Entity_addSprites(this, this->entityDefinition->spriteDefinitions);
}

/**
 * Release sprites
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_releaseGraphics(Entity this)
{
	ASSERT(this, "Entity::releaseGraphics: null this");

	__CALL_BASE_METHOD(Container, releaseGraphics, this);

	Entity_releaseSprites(this, false);
}

/**
 * Release sprites
 *
 * @memberof			Entity
 * @public
 *
 * @param this			Function scope
 * @param deleteThem	True deleted Sprites immediately, false defers sprite disposal to not impact performance.
 */
void Entity_releaseSprites(Entity this, bool deleteThem)
{
	ASSERT(this, "Entity::releaseSprites: null this");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		if(deleteThem)
		{
			for(; node ; node = node->next)
			{
				__DELETE(node->data);
			}
		}
		else
		{
			for(; node ; node = node->next)
			{
				SpriteManager_disposeSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, node->data));
			}
		}

		// delete the sprites
		__DELETE(this->sprites);

		this->sprites = NULL;
	}
}

/**
 * Calculate size from children
 *
 * @memberof					Entity
 * @private
 *
 * @param this					Function scope
 * @param rightBox
 * @param environmentPosition
 */
static void Entity_calculateSizeFromChildren(Entity this, SmallRightBox* rightBox, Vector3D environmentPosition)
{
	ASSERT(this, "Entity::calculateSizeFromChildren: null this");

	Vector3D globalPosition3D = environmentPosition;

	globalPosition3D.x += this->transformation.localPosition.x;
	globalPosition3D.y += this->transformation.localPosition.y;
	globalPosition3D.z += this->transformation.localPosition.z;

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
//			__VIRTUAL_CALL(Sprite, resize, sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			halfWidth = Sprite_getHalfWidth(sprite);
			halfHeight = Sprite_getHalfHeight(sprite);
			halfDepth = this->size.z >> 1;

			WorldVector spriteDisplacement = Sprite_getDisplacement(sprite);

			if(left > -halfWidth + __FIX19_13_TO_I(spriteDisplacement.x))
			{
				left = -halfWidth + __FIX19_13_TO_I(spriteDisplacement.x);
			}

			if(right < halfWidth + __FIX19_13_TO_I(spriteDisplacement.x))
			{
				right = halfWidth + __FIX19_13_TO_I(spriteDisplacement.x);
			}

			if(top > -halfHeight + __FIX19_13_TO_I(spriteDisplacement.y))
			{
				top = -halfHeight + __FIX19_13_TO_I(spriteDisplacement.y);
			}

			if(bottom < halfHeight + __FIX19_13_TO_I(spriteDisplacement.y))
			{
				bottom = halfHeight + __FIX19_13_TO_I(spriteDisplacement.y);
			}

			if(front > -halfDepth + __FIX19_13_TO_I(spriteDisplacement.z))
			{
				front = -halfDepth + __FIX19_13_TO_I(spriteDisplacement.z);
			}

			if(back < halfDepth + __FIX19_13_TO_I(spriteDisplacement.z))
			{
				back = halfDepth + __FIX19_13_TO_I(spriteDisplacement.z);
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

	int x = __FIX19_13_TO_I(globalPosition3D.x);
	int y = __FIX19_13_TO_I(globalPosition3D.y);
	int z = __FIX19_13_TO_I(globalPosition3D.z);

	if((0 == rightBox->x0) | (x + left < rightBox->x0))
	{
		rightBox->x0 = x + left;
	}

	if((0 == rightBox->x1) | (right + x > rightBox->x1))
	{
		rightBox->x1 = right + x;
	}

	if((0 == rightBox->y0) | (y + top < rightBox->y0))
	{
		rightBox->y0 = y + top;
	}

	if((0 == rightBox->y1) | (bottom + y > rightBox->y1))
	{
		rightBox->y1 = bottom + y;
	}

	if((0 == rightBox->z0) | (z + front < rightBox->z0))
	{
		rightBox->z0 = z + front;
	}

	if((0 == rightBox->z1) | (back + z > rightBox->z1))
	{
		rightBox->z1 = back + z;
	}

	if(this->children)
	{
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Entity_calculateSizeFromChildren(__SAFE_CAST(Entity, childNode->data), rightBox, globalPosition3D);
		}
	}
}

/**
 * Calculate my size based on me and my children
 *
 * @memberof	Entity
 * @private
 *
 * @param this	Function scope
 */
void Entity_calculateSize(Entity this)
{
	ASSERT(this, "Entity::calculateSize: null this");

	SmallRightBox rightBox = {0, 0, 0, 0, 0, 0};

	Entity_calculateSizeFromChildren(this, &rightBox, (Vector3D){0, 0, 0});

	Vector3D centerDisplacement =
	{
		(__I_TO_FIX19_13(rightBox.x1 + rightBox.x0) / 2) - this->transformation.localPosition.x,
		(__I_TO_FIX19_13(rightBox.y1 + rightBox.y0) / 2) - this->transformation.localPosition.y,
		(__I_TO_FIX19_13(rightBox.z1 + rightBox.z0) / 2) - this->transformation.localPosition.z
	};

	if(centerDisplacement.x | centerDisplacement.y | centerDisplacement.z)
	{
		if(this->centerDisplacement)
		{
			__DELETE_BASIC(this->centerDisplacement);
		}

		this->centerDisplacement = __NEW_BASIC(Vector3D);
		*this->centerDisplacement = centerDisplacement;
	}

	this->size.x = rightBox.x1 - rightBox.x0;
	this->size.y = rightBox.y1 - rightBox.y0;
	this->size.z = rightBox.z1 - rightBox.z0;
}

/**
 * Get size from definition
 *
 * @memberof					Entity
 * @private
 *
 * @param positionedEntity
 * @param environmentPosition
 * @param rightBox
 */
static void Entity_getSizeFromDefinition(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition, SmallRightBox* rightBox)
{
	ASSERT(positionedEntity, "Entity::getSizeFromDefinition: null positionedEntity");
	ASSERT(positionedEntity->entityDefinition, "Entity::getSizeFromDefinition: null entityDefinition");

	Vector3D globalPosition3D =
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

	if(positionedEntity->entityDefinition->spriteDefinitions && positionedEntity->entityDefinition->spriteDefinitions[0])
	{
		int i = 0;

		for(; positionedEntity->entityDefinition->spriteDefinitions[i]; i++)
		{
			if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(positionedEntity->entityDefinition->spriteDefinitions[i]->allocator && ((MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spriteDefinitions[i])->textureDefinitions[0]))
			{
				MBgmapSpriteDefinition* mBgmapSpriteDefinition = (MBgmapSpriteDefinition*)positionedEntity->entityDefinition->spriteDefinitions[i];

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

				if(left > -halfWidth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x))
				{
					left = -halfWidth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x);
				}

				if(right < halfWidth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x))
				{
					right = halfWidth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x);
				}

				if(top > -halfHeight + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y))
				{
					top = -halfHeight + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y);
				}

				if(bottom < halfHeight + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y))
				{
					bottom = halfHeight + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y);
				}

				if(front > __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z))
				{
					front = __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z);
				}

				if(back < halfDepth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z))
				{
					back = halfDepth + __FIX19_13_TO_I(mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z);
				}

			}
			else if(positionedEntity->entityDefinition->spriteDefinitions[i]->textureDefinition)
			{
				SpriteDefinition* spriteDefinition = (SpriteDefinition*)positionedEntity->entityDefinition->spriteDefinitions[i];
				halfWidth = spriteDefinition->textureDefinition->cols << 2;
				halfHeight = spriteDefinition->textureDefinition->rows << 2;
				halfDepth = 10;

				if(left > -halfWidth + __FIX19_13_TO_I(spriteDefinition->displacement.x))
				{
					left = -halfWidth + __FIX19_13_TO_I(spriteDefinition->displacement.x);
				}

				if(right < halfWidth + __FIX19_13_TO_I(spriteDefinition->displacement.x))
				{
					right = halfWidth + __FIX19_13_TO_I(spriteDefinition->displacement.x);
				}

				if(top > -halfHeight + __FIX19_13_TO_I(spriteDefinition->displacement.y))
				{
					top = -halfHeight + __FIX19_13_TO_I(spriteDefinition->displacement.y);
				}

				if(bottom < halfHeight + __FIX19_13_TO_I(spriteDefinition->displacement.y))
				{
					bottom = halfHeight + __FIX19_13_TO_I(spriteDefinition->displacement.y);
				}

				if(front > -halfDepth + __FIX19_13_TO_I(spriteDefinition->displacement.z))
				{
					front = -halfDepth + __FIX19_13_TO_I(spriteDefinition->displacement.z);
				}

				if(back < (halfDepth << 1) + __FIX19_13_TO_I(spriteDefinition->displacement.z))
				{
					back = (halfDepth << 1) + __FIX19_13_TO_I(spriteDefinition->displacement.z);
				}
			}
		}
	}
	else if(!positionedEntity->childrenDefinitions)
	{
		// TODO: there should be a class which handles these special cases
		if(__TYPE(Entity) == __ALLOCATOR_TYPE(positionedEntity->entityDefinition->allocator)
		)
		{
			halfWidth = positionedEntity->entityDefinition->size.x >> 1;
			halfHeight = positionedEntity->entityDefinition->size.y >> 1;
			halfDepth = positionedEntity->entityDefinition->size.z >> 1;

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

	int x = __FIX19_13_TO_I(globalPosition3D.x);
	int y = __FIX19_13_TO_I(globalPosition3D.y);
	int z = __FIX19_13_TO_I(globalPosition3D.z);

	if((0 == rightBox->x0) | (x + left < rightBox->x0))
	{
		rightBox->x0 = x + left;
	}

	if((0 == rightBox->x1) | (right + x > rightBox->x1))
	{
		rightBox->x1 = right + x;
	}

	if((0 == rightBox->y0) | (y + top < rightBox->y0))
	{
		rightBox->y0 = y + top;
	}

	if((0 == rightBox->y1) | (bottom + y > rightBox->y1))
	{
		rightBox->y1 = bottom + y;
	}

	if((0 == rightBox->z0) | (z + front < rightBox->z0))
	{
		rightBox->z0 = z + front;
	}

	if((0 == rightBox->z1) | (back + z > rightBox->z1))
	{
		rightBox->z1 = back + z;
	}

	if(positionedEntity->childrenDefinitions)
	{
		int i = 0;
		for(; positionedEntity->childrenDefinitions[i].entityDefinition; i++)
		{
			Entity_getSizeFromDefinition(&positionedEntity->childrenDefinitions[i], &globalPosition3D, rightBox);
		}
	}
}

/**
 * Calculate total size from definition
 *
 * @memberof					Entity
 * @public
 *
 * @param positionedEntity
 * @param environmentPosition
 *
 * @return						SmallRightBox
 */
SmallRightBox Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition)
{
	SmallRightBox rightBox = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromDefinition(positionedEntity, (Vector3D*)environmentPosition, &rightBox);

	rightBox.x0 = rightBox.x0 - __FIX19_13_TO_I(positionedEntity->position.x);
	rightBox.x1 = rightBox.x1 - __FIX19_13_TO_I(positionedEntity->position.x);
	rightBox.y0 = rightBox.y0 - __FIX19_13_TO_I(positionedEntity->position.y);
	rightBox.y1 = rightBox.y1 - __FIX19_13_TO_I(positionedEntity->position.y);
	rightBox.z0 = rightBox.z0 - __FIX19_13_TO_I(positionedEntity->position.z);
	rightBox.z1 = rightBox.z1 - __FIX19_13_TO_I(positionedEntity->position.z);

	return rightBox;
}

/**
 * Find child by name in given list
 *
 * @memberof					Entity
 * @public
 *
 * @param childrenDefinitions
 * @param environmentPosition
 * @param childName
 *
 * @return						Entity's global position
 */
Vector3D* Entity_calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, Vector3D environmentPosition, const char* childName)
{
	ASSERT(childrenDefinitions, "Entity::calculateGlobalPositionFromDefinitionByName: null positionedEntity");

	if(!childrenDefinitions)
	{
		return NULL;
	}

	static Vector3D position;

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
			Vector3D concatenatedEnvironmentPosition = environmentPosition;
			concatenatedEnvironmentPosition.x += childrenDefinitions[i].position.x;
			concatenatedEnvironmentPosition.y += childrenDefinitions[i].position.y;
			concatenatedEnvironmentPosition.z += childrenDefinitions[i].position.z;

			Vector3D* position = Entity_calculateGlobalPositionFromDefinitionByName(childrenDefinitions[i].childrenDefinitions, concatenatedEnvironmentPosition, childName);

			if(position)
			{
				return position;
			}
		}
	}

	return NULL;
}

/**
 * Instantiate an Entity using the provided allocator
 *
 * @memberof				Entity
 * @public
 *
 * @param entityDefinition
 * @param id
 * @param internalId
 * @param name
 * @param extraInfo
 *
 * @return					Entity instance
 */
Entity Entity_instantiate(const EntityDefinition* const entityDefinition, s16 id, s16 internalId, const char* const name, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::load: null definition");
	ASSERT(entityDefinition->allocator, "Entity::load: no allocator defined");

	if(!entityDefinition || !entityDefinition->allocator)
	{
		return NULL;
	}

	// call the appropriate allocator to support inheritance
	Entity entity = ((Entity (*)(EntityDefinition*, s16, s16, const char* const)) entityDefinition->allocator)((EntityDefinition*)entityDefinition, id, internalId, name);

	// process extra info
	if(extraInfo)
	{
		__VIRTUAL_CALL(Entity, setExtraInfo, entity, extraInfo);
	}

	return entity;
}

/**
 * Add children to the instance from the definitions array
 *
 * @memberof					Entity
 * @public
 *
 * @param this					Function scope
 * @param childrenDefinitions
 */
void Entity_addChildEntities(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::loadChildren: null this");

	if(!childrenDefinitions)
	{
		return;
	}

	int i = 0;

	// go through n sprites in entity's definition
	for(; childrenDefinitions[i].entityDefinition; i++)
	{
		Entity child = Entity_loadEntity(&childrenDefinitions[i], this->internalId + Container_getChildCount(__SAFE_CAST(Container, this)));
		ASSERT(child, "Entity::loadChildren: entity not loaded");

		// create the entity and add it to the world
		Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, child));
	}
}

/**
 * Load an entity and instantiate all its children
 *
 * @memberof					Entity
 * @public
 *
 * @param positionedEntity
 * @param internalId
 *
 * @return						Entity
 */
Entity Entity_loadEntity(const PositionedEntity* const positionedEntity, s16 internalId)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity_instantiate(positionedEntity->entityDefinition, positionedEntity->id, internalId, positionedEntity->name, positionedEntity->extraInfo);
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

/**
 * Add children to instance from the definitions array, but deferred
 *
 * @memberof					Entity
 * @public
 *
 * @param this					Function scope
 * @param childrenDefinitions
 */
void Entity_addChildEntitiesDeferred(Entity this, const PositionedEntity* childrenDefinitions)
{
	ASSERT(this, "Entity::addChildEntitiesDeferred: null this");
	ASSERT(childrenDefinitions, "Entity::addChildEntitiesDeferred: null childrenDefinitions");

	if(!childrenDefinitions)
	{
		return;
	}

	if(!this->entityFactory)
	{
		this->entityFactory = __NEW(EntityFactory);
	}

	int i = 0;

	// go through n sprites in entity's definition
	for(; childrenDefinitions[i].entityDefinition; i++)
	{
		EntityFactory_spawnEntity(this->entityFactory, &childrenDefinitions[i], __SAFE_CAST(Container, this), NULL, this->internalId + Container_getChildCount(__SAFE_CAST(Container, this)));
	}
}

/**
 * Load an entity and instantiate all its children, deferred
 *
 * @memberof				Entity
 * @public
 *
 * @param positionedEntity
 * @param internalId
 *
 * @return					Entity
 */
Entity Entity_loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 internalId)
{
	ASSERT(positionedEntity, "Entity::loadEntityDeferred: null positionedEntity");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity_instantiate(positionedEntity->entityDefinition, positionedEntity->id, internalId, positionedEntity->name, positionedEntity->extraInfo);
	ASSERT(entity, "Entity::loadEntityDeferred: entity not loaded");

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

/**
 * Add child entity from definition
 *
 * @memberof				Entity
 * @public
 *
 * @param this				Function scope
 * @param entityDefinition
 * @param internalId
 * @param name
 * @param position
 * @param extraInfo
 *
 * @return					Entity
 */
Entity Entity_addChildEntity(Entity this, const EntityDefinition* entityDefinition, int internalId, const char* name, const Vector3D* position, void* extraInfo)
{
	ASSERT(this, "Entity::addChildEntity: null this");
	ASSERT(entityDefinition, "Entity::addChildEntity: null entityDefinition");

	if(!entityDefinition)
	{
		return NULL;
	}

	PositionedEntity positionedEntity =
	{
		(EntityDefinition*)entityDefinition,
		{position->x, position->y, position->z},
		this->internalId + Container_getChildCount(__SAFE_CAST(Container, this)),
		(char*)name,
		NULL,
		extraInfo,
		false
	};

	// create the hint entity and add it to the hero as a child entity
	Entity childEntity = Entity_loadEntity(&positionedEntity, 0 > internalId? internalId: positionedEntity.id);
	ASSERT(childEntity, "Entity::addChildEntity: childEntity no created");

	// must add graphics
	__VIRTUAL_CALL(Container, setupGraphics, childEntity);
	__VIRTUAL_CALL(Entity, initialize, childEntity, true);

	// create the entity and add it to the world
	Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, childEntity));

	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));

	 // apply transformations
	__VIRTUAL_CALL(Container, initialTransform, childEntity, &environmentTransform, true);

	// make ready
	__VIRTUAL_CALL(Entity, ready, childEntity, true);

	return childEntity;
}

/**
 * Are all children instantiated?
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether all children are instantiated
 */
u32 Entity_areAllChildrenInstantiated(Entity this)
{
	ASSERT(this, "Entity::areAllChildrenInstantiated: null this");

	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory_instantiateEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children initialized?
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether all children are initialized
 */
u32 Entity_areAllChildrenInitialized(Entity this)
{
	ASSERT(this, "Entity::areAllChildrenInitialized: null this");

	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory_initializeEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children transformed?
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether all children are transformed
 */
u32 Entity_areAllChildrenTransformed(Entity this)
{
	ASSERT(this, "Entity::areAllChildrenTransformed: null this");

	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory_transformEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children ready?
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether all children are ready
 */
u32 Entity_areAllChildrenReady(Entity this)
{
	ASSERT(this, "Entity::areAllChildrenReady: null this");

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

/**
 * Set shape's position
 *
 * @memberof					Entity
 * @private
 *
 * @param this					Function scope
 */
void Entity_transformShapes(Entity this)
{
	ASSERT(this, "Entity::transformShapes: null this");

	if(this->shapes && this->entityDefinition->shapeDefinitions)
	{
		const ShapeDefinition* shapeDefinitions = this->entityDefinition->shapeDefinitions;

		// setup shape
//		bool isAffectedByRelativity = __VIRTUAL_CALL(SpatialObject, isAffectedByRelativity, this);
		const Vector3D* myPosition = __VIRTUAL_CALL(SpatialObject, getPosition, this);
		const Rotation* myRotation = __VIRTUAL_CALL(SpatialObject, getRotation, this);
		const Scale* myScale = __VIRTUAL_CALL(SpatialObject, getScale, this);

		VirtualNode node = this->shapes->head;
//		Direction currentDirection = Entity_getDirection(this);
		int i = 0;

		for(; node && shapeDefinitions[i].allocator; node = node->next, i++)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			Vector3D shapePosition =
			{
				myPosition->x + shapeDefinitions[i].displacement.x,
				myPosition->y + shapeDefinitions[i].displacement.y,
				myPosition->z + shapeDefinitions[i].displacement.z,
/*				myPosition->x + (__RIGHT == currentDirection.x ? shapeDefinitions[i].displacement.x : -shapeDefinitions[i].displacement.x),
				myPosition->y + (__DOWN == currentDirection.y ? shapeDefinitions[i].displacement.y : -shapeDefinitions[i].displacement.y),
				myPosition->z + (__FAR == currentDirection.z ? shapeDefinitions[i].displacement.z : -shapeDefinitions[i].displacement.z),
*/			};

			Rotation shapeRotation =
			{
				myRotation->x + shapeDefinitions[i].rotation.x,
				myRotation->y + shapeDefinitions[i].rotation.y,
				myRotation->z + shapeDefinitions[i].rotation.z,
			};

			Scale shapeScale =
			{
				__FIX7_9_MULT(myScale->x, shapeDefinitions[i].scale.x),
				__FIX7_9_MULT(myScale->y, shapeDefinitions[i].scale.y),
				__FIX7_9_MULT(myScale->z, shapeDefinitions[i].scale.z),
			};

			__VIRTUAL_CALL(Shape, setup, shape, &shapePosition, &shapeRotation, &shapeScale, &shapeDefinitions[i].size, shapeDefinitions[i].layers, shapeDefinitions[i].layersToIgnore);
		}
	}
}

/**
 * Setup shape
 *
 * @memberof					Entity
 * @private
 *
 * @param this					Function scope
 * @param shapeDefinitions		List of shapes
 */
static void Entity_addShapes(Entity this, const ShapeDefinition* shapeDefinitions, bool destroyPreviousShapes)
{
	ASSERT(this, "Entity::addShapes: null this");

	if(!shapeDefinitions)
	{
		return;
	}

	if(destroyPreviousShapes)
	{
		Entity_destroyShapes(this);
	}

	int i = 0;

	if(!this->shapes)
	{
		this->shapes = __NEW(VirtualList);
	}

	// go through n sprites in entity's definition
	for(; shapeDefinitions[i].allocator; i++)
	{
		Shape shape = CollisionManager_createShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), &shapeDefinitions[i]);
		ASSERT(shape, "Entity::addSprite: sprite not created");

		Shape_setActive(shape, true);

		Shape_setCheckForCollisions(shape, shapeDefinitions[i].checkForCollisions);

		VirtualList_pushBack(this->shapes, shape);
	}

	Entity_transformShapes(this);
}

/**
 * Entity is initialized
 *
 * @memberof		Entity
 * @public
 *
 * @param this		Function scope
 * @param recursive
 */
void Entity_initialize(Entity this, bool recursive)
{
	ASSERT(this, "Entity::initialize: null this");

	if(recursive && this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			__VIRTUAL_CALL(Entity, initialize, childNode->data, recursive);
		}
	}
}

/**
 * Entity is ready
 *
 * @memberof		Entity
 * @public
 *
 * @param this		Function scope
 * @param recursive
 */
void Entity_ready(Entity this, bool recursive)
{
	ASSERT(this, "Entity::ready: null this");

	if(recursive && this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			__VIRTUAL_CALL(Entity, ready, childNode->data, recursive);
		}
	}
}

/**
 * Process extra info in initialization
 *
 * @memberof		Entity
 * @public
 *
 * @param this		Function scope
 * @param extraInfo
 */
void Entity_setExtraInfo(Entity this __attribute__ ((unused)), void* extraInfo __attribute__ ((unused)))
{
	ASSERT(this, "Entity::setExtraInfo: null this");
}

/**
 * Add sprites
 *
 * @memberof					Entity
 * @public
 *
 * @param this					Function scope
 * @param spriteDefinitions
 */
void Entity_addSprites(Entity this, const SpriteDefinition** spriteDefinitions)
{
	ASSERT(this, "Entity::addSprites: null this");

	if(!spriteDefinitions)
	{
		return;
	}

	int i = 0;

	if(!this->sprites)
	{
		this->sprites = __NEW(VirtualList);
	}

	// go through n sprites in entity's definition
	for(; spriteDefinitions[i]; i++)
	{
		ASSERT(spriteDefinitions[i]->allocator, "Entity::addSprites: no sprite allocator");

		Sprite sprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinitions[i]->allocator)((SpriteDefinition*)spriteDefinitions[i], __SAFE_CAST(Object, this));
		ASSERT(sprite, "Entity::addSprite: sprite not created");

		VirtualList_pushBack(this->sprites, (void*)sprite);
	}
}

/**
 * Add sprite
 *
 * @memberof						Entity
 * @public
 *
 * @param this						Function scope
 * @param spriteDefinitionIndex		Index in sprite definitions array
 *
 * @return							True if a sprite was created
 */
bool Entity_addSpriteFromDefinitionAtIndex(Entity this, int spriteDefinitionIndex)
{
	ASSERT(this, "Entity::addSprite: null this");

	if(!this->entityDefinition->spriteDefinitions)
	{
		return false;
	}

	const SpriteDefinition* spriteDefinition = this->entityDefinition->spriteDefinitions[spriteDefinitionIndex];

	if(!spriteDefinition || !spriteDefinition->allocator)
	{
		return false;
	}

	// call the appropriate allocator to support inheritance
	Sprite sprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
	ASSERT(sprite, "Entity::addSprite: sprite not created");

	if(!this->sprites)
	{
		this->sprites = __NEW(VirtualList);
	}

	VirtualList_pushBack(this->sprites, sprite);

	return true;
}

/**
 * Update sprites
 *
 * @memberof				Entity
 * @private
 *
 * @param this				Function scope
 * @param updatePosition
 * @param updateScale
 * @param updateRotation
 */
 /*
static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation)
{
	ASSERT(this, "Entity::transform: null this");

	if(!this->sprites)
	{
		return;
	}

	updatePosition |= updateRotation;

	VirtualNode node = this->sprites->head;

	for(; node ; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		if(updatePosition)
		{
			// update sprite's 2D position
			__VIRTUAL_CALL(Sprite, position, sprite, &this->transformation.globalPosition);
		}

		if(updateRotation)
		{
			__VIRTUAL_CALL(Sprite, rotate, sprite, &this->transformation.globalRotation);
		}

		if(updateScale)
		{
			// calculate the scale
			__VIRTUAL_CALL(Sprite, resize, sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			__VIRTUAL_CALL(Sprite, calculateParallax, sprite, this->transformation.globalPosition.z);
		}
	}
}
*/
static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation)
{
	ASSERT(this, "Entity::transform: null this");

	if(!this->sprites)
	{
		return;
	}

	updatePosition |= updateRotation;

	VirtualNode node = this->sprites->head;

	if(updatePosition && updateRotation && updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			__VIRTUAL_CALL(Sprite, position, sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			__VIRTUAL_CALL(Sprite, rotate, sprite, &this->transformation.globalRotation);

			// calculate the scale
			__VIRTUAL_CALL(Sprite, resize, sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			__VIRTUAL_CALL(Sprite, calculateParallax, sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition && updateRotation)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			__VIRTUAL_CALL(Sprite, position, sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			__VIRTUAL_CALL(Sprite, rotate, sprite, &this->transformation.globalRotation);
		}
	}
	else if(updatePosition && updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			__VIRTUAL_CALL(Sprite, position, sprite, &this->transformation.globalPosition);

			// calculate the scale
			__VIRTUAL_CALL(Sprite, resize, sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			__VIRTUAL_CALL(Sprite, calculateParallax, sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			__VIRTUAL_CALL(Sprite, position, sprite, &this->transformation.globalPosition);
		}
	}
	else if(updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// calculate the scale
			__VIRTUAL_CALL(Sprite, resize, sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			__VIRTUAL_CALL(Sprite, calculateParallax, sprite, this->transformation.globalPosition.z);
		}
	}
}

/**
 * Initial transformation
 *
 * @memberof					Entity
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 * @param recursive
 */
void Entity_initialTransform(Entity this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "Entity::initialTransform: null this");

	// call base class's transformation method
	__CALL_BASE_METHOD(Container, initialTransform, this, environmentTransform, recursive);

	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;

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

	// this method can be called multiple times so only add shapes
	// if not already done
	if(!this->shapes)
	{
		Entity_addShapes(this, this->entityDefinition->shapeDefinitions, false);
	}
}

/**
 * Transform class
 *
 * @memberof					Entity
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void Entity_transform(Entity this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "Entity::transform: null this");

	this->invalidateSprites = 0;

	if(this->sprites)
	{
		this->invalidateSprites = invalidateTransformationFlag | Entity_updateSpritePosition(this) | Entity_updateSpriteRotation(this) | Entity_updateSpriteScale(this);
	}

	if(this->invalidateGlobalTransformation)
	{
		Entity_transformShapes(this);

		// call base class's transformation method
		__CALL_BASE_METHOD(Container, transform, this, environmentTransform, invalidateTransformationFlag);
	}
	else if((u32)this->children)
	{
		__CALL_BASE_METHOD(Container, transform, this, environmentTransform, invalidateTransformationFlag);
	}
}

/**
 * Set local position
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_setLocalPosition(Entity this, const Vector3D* position)
{
	ASSERT(this, "Entity::setLocalPosition: null this");

	__CALL_BASE_METHOD(Container, setLocalPosition, this, position);

	Entity_transformShapes(this);
}

/**
 * Set local rotation
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_setLocalRotation(Entity this, const Rotation* rotation)
{
	ASSERT(this, "Entity::setLocalRotation: null this");

	__CALL_BASE_METHOD(Container, setLocalRotation, this, rotation);

	Entity_transformShapes(this);
}

/**
 * Update visual representation
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_synchronizeGraphics(Entity this)
{
	ASSERT(this, "Entity::synchronizeGraphics: null this");

	if(this->children)
	{
		__CALL_BASE_METHOD(Container, synchronizeGraphics, this);
	}

	Entity_updateSprites(this, this->invalidateSprites & __INVALIDATE_POSITION, this->invalidateSprites & __INVALIDATE_SCALE, this->invalidateSprites & __INVALIDATE_ROTATION);

	this->invalidateSprites = false;
}

/**
 * Retrieve EntityDefinition
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		EntityDefinition
 */
EntityDefinition* Entity_getEntityDefinition(Entity this)
{
	ASSERT(this, "Entity::getEntityDefinition: null this");

	return this->entityDefinition;
}

/**
 * Retrieve position
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Global position
 */
const Vector3D* Entity_getPosition(Entity this)
{
	ASSERT(this, "Entity::getPosition: null this");

	return &this->transformation.globalPosition;
}

/**
 * Retrieve rotation
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Global rotation
 */
const Rotation* Entity_getRotation(Entity this)
{
	ASSERT(this, "Entity::getRotation: null this");

	return &this->transformation.globalRotation;
}

/**
 * Retrieve scale
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Global position
 */
const Scale* Entity_getScale(Entity this)
{
	ASSERT(this, "Entity::getScale: null this");

	return &this->transformation.globalScale;
}

/**
 * Retrieve sprites
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		VirtualList of Entity's sprites
 */
VirtualList Entity_getSprites(Entity this)
{
	ASSERT(this, "Entity::getSprites: null this");

	return this->sprites;
}

/**
 * Handles incoming messages
 *
 * @memberof		Entity
 * @public
 *
 * @param this		Function scope
 * @param telegram
 *
 * @return			True if successfully processed, false otherwise
 */
bool Entity_handleMessage(Entity this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	ASSERT(this, "Entity::handleMessage: null this");

	return false;
}

/**
 * Get width
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Entity's width
 */
u16 Entity_getWidth(Entity this)
{
	ASSERT(this, "Entity::getWidth: null this");

	if(!this->size.x)
	{
		Entity_calculateSize(this);
	}

	// must calculate based on the scale because not affine container must be enlarged
	return this->size.x;
}

/**
 * Get height
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Entity's height
 */
u16 Entity_getHeight(Entity this)
{
	ASSERT(this, "Entity::getHeight: null this");

	if(!this->size.y)
	{
		Entity_calculateSize(this);
	}

	return this->size.y;
}

/**
 * Get depth
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Entity's depth
 */
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

/**
 * Whether it is visible
 *
 * @memberof		Entity
 * @public
 *
 * @param this		Function scope
 * @param pad
 * @param recursive
 *
 * @return			Boolean if visible
 */
bool Entity_isVisible(Entity this, int pad, bool recursive)
{
	ASSERT(this, "Entity::isVisible: null this");

	int x = 0;
	int y = 0;
	int z = 0;

	if(this->sprites && this->sprites->head)
	{
		VirtualNode spriteNode = this->sprites->head;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
			ASSERT(sprite, "Entity:isVisible: null sprite");

			Vector2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, sprite);

			x = __FIX19_13_TO_I(spritePosition.x);
			y = __FIX19_13_TO_I(spritePosition.y);
			z = __FIX19_13_TO_I(spritePosition.z);

			// check x visibility
			if((x + (int)this->size.x < -pad) | (x > __SCREEN_WIDTH + pad))
			{
				continue;
			}

			// check y visibility
			if((y + (int)this->size.y < -pad) | (y > __SCREEN_HEIGHT + pad))
			{
				continue;
			}

			// check z visibility
			if((z + (int)this->size.z < -pad) | (z > __SCREEN_DEPTH + pad))
			{
				continue;
			}

			return true;

			/*
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
			*/
		}
	}
	else
	{
		Vector3D position3D = Vector3D_toScreen(this->transformation.globalPosition);

		if(this->centerDisplacement)
		{
			position3D.x += this->centerDisplacement->x;
			position3D.y += this->centerDisplacement->y;
			position3D.z += this->centerDisplacement->z;
		}

		Vector2D position2D = Vector3D_projectToVector2D(position3D);

		int halfWidth = (int)(this->size.x >> 1);
		int halfHeight = (int)(this->size.y >> 1);
		int halfDepth = (int)(this->size.z >> 1);

		x = __FIX19_13_TO_I(position2D.x);
		y = __FIX19_13_TO_I(position2D.y);
		z = __FIX19_13_TO_I(position2D.z);

		// check x visibility
		if((x + halfWidth <= -pad) | (x - halfWidth >= __SCREEN_WIDTH + pad))
		{
			return false;
		}

		// check y visibility
		if((y + halfHeight <= -pad) | (y - halfHeight >= __SCREEN_HEIGHT + pad))
		{
			return false;
		}

		// check z visibility
		if((z + halfDepth <= -pad) | (z - halfDepth >= __SCREEN_DEPTH + pad))
		{
			return false;
		}

		return true;
/*
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
		*/
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

/**
 * Check if necessary to update sprite's position
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean if necessary
 */
bool Entity_updateSpritePosition(Entity this)
{
	ASSERT(this, "Entity::updateSpritePosition: null this");

	return __INVALIDATE_POSITION & this->invalidateGlobalTransformation;
}

/**
 * Check if necessary to update sprite's rotation
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean if necessary
 */
bool Entity_updateSpriteRotation(Entity this)
{
	ASSERT(this, "Entity::updateSpriteRotation: null this");

	return __INVALIDATE_ROTATION & this->invalidateGlobalTransformation;
}

/**
 * Check if necessary to update sprite's scale
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean if necessary
 */
bool Entity_updateSpriteScale(Entity this)
{
	ASSERT(this, "Entity::updateSpriteScale: null this");

	return __INVALIDATE_SCALE & this->invalidateGlobalTransformation;
}

/**
 * Retrieve shapes list
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Entity's Shape list
 */
VirtualList Entity_getShapes(Entity this)
{
	ASSERT(this, "Entity::getShapes: null this");

	return this->shapes;
}

/**
 * Make it visible
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_show(Entity this)
{
	ASSERT(this, "Entity::show: null this");

	// update transformation before hiding
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(Container, transform, this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// and update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity_synchronizeGraphics(this);

	__CALL_BASE_METHOD(Container, show, this);

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

/**
 * Make it invisible
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_hide(Entity this)
{
	ASSERT(this, "Entity::hide: null this");

	// update transformation before hiding
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(Container, transform, this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// and update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity_synchronizeGraphics(this);

	__CALL_BASE_METHOD(Container, hide, this);

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

/**
 * Suspend for pause
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_suspend(Entity this)
{
	ASSERT(this, "Entity::suspend: null this");

	__CALL_BASE_METHOD(Container, suspend, this);

	Entity_releaseSprites(this, true);
}

/**
 * Resume after pause
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_resume(Entity this)
{
	ASSERT(this, "Entity::resume: null this");

	__CALL_BASE_METHOD(Container, resume, this);

	// initialize sprites
	if(this->entityDefinition)
	{
		Entity_addSprites(this, this->entityDefinition->spriteDefinitions);
	}

	if(this->hidden)
	{
		Entity_hide(this);
	}

	// force update sprites on next game's cycle
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
}

/**
 * Defaults to true
 *
 * @memberof			Entity
 * @public
 *
 * @param this			Function scope
 * @param acceleration
 *
 * @return				Defaults to true
 */
bool Entity_isSubjectToGravity(Entity this __attribute__ ((unused)), Acceleration gravity __attribute__ ((unused)))
{
	ASSERT(this, "Entity::isSubjectToGravity: null this");

	return true;
}

/**
 * Get axis for flipping
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Defaults to true
 */
u16 Entity_getAxisForFlipping(Entity this __attribute__ ((unused)))
{
	ASSERT(this, "Entity::getAxisForFlipping: null this");

	return __X_AXIS | __Y_AXIS;
}


/**
 * Get in game type
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Type of entity within the game's logic
 */
u32 Entity_getInGameType(Entity this)
{
	ASSERT(this, "Entity::getInGameType: null this");

	return this->entityDefinition->inGameType;
}

/**
 * Get elasticity for physics
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Elasticity
 */
fix19_13 Entity_getElasticity(Entity this)
{
	ASSERT(this, "Entity::getElasticity: null this");

	return this->entityDefinition->physicalSpecification ? this->entityDefinition->physicalSpecification->elasticity : 0;
}

/**
 * Get friction
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Friction
 */
fix19_13 Entity_getFrictionCoefficient(Entity this)
{
	ASSERT(this, "Entity::getFrictionCoefficient: null this");

	return this->entityDefinition->physicalSpecification ? this->entityDefinition->physicalSpecification->frictionCoefficient : 0;
}

/**
 * Propagate that movement started to the shapes
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_informShapesThatStartedMoving(Entity this)
{
	ASSERT(this, "Entity::informShapesThatStartedMoving: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape_setActive(__SAFE_CAST(Shape, node->data), true);
			CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(Shape, node->data));
		}
	}
}

/**
 * Propagate that movement stopped to the shapes
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_informShapesThatStoppedMoving(Entity this)
{
	ASSERT(this, "Entity::informShapesThatStoppedMoving: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape_setActive(__SAFE_CAST(Shape, node->data), true);
			CollisionManager_shapeStoppedMoving(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(Shape, node->data));
		}
	}
}

/**
 * Propagate active status to the shapes
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_activateShapes(Entity this, bool value)
{
	ASSERT(this, "Entity::activateShapes: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape_setActive(__SAFE_CAST(Shape, node->data), value);
		}
	}
}

/**
 * Set direction
 *
 * @memberof			Entity
 * @public
 *
 * @param this			Function scope
 * @param direction		Direction
 */
void Entity_setDirection(Entity this, Direction direction)
{
	ASSERT(this, "Entity::setDirection: null this");

	Direction currentDirection = Entity_getDirection(this);

	// if directions XOR is 0, they are equal
	if(
		!(
			(currentDirection.x ^ direction.x) |
			(currentDirection.y ^ direction.y) |
			(currentDirection.z ^ direction.z)
		)
	)
	{
		return;
	}

	Rotation rotation =
	{
		__UP == direction.y ? __HALF_ROTATION_DEGREES : __DOWN == direction.y ? 0 : this->transformation.localRotation.x,
		__LEFT == direction.x ? __HALF_ROTATION_DEGREES : __RIGHT == direction.x ? 0 : this->transformation.localRotation.y,
		//__NEAR == direction.z ? __HALF_ROTATION_DEGREES : __FAR == direction.z ? 0 : this->transformation.localRotation.z,
		this->transformation.localRotation.z,
	};


	Container_setLocalRotation(__SAFE_CAST(Container, this), &rotation);
}

/**
 * Get direction
 *
 * @memberof			Entity
 * @public
 *
 * @param this			Function scope
 *
 * @return				Direction
 */
Direction Entity_getDirection(Entity this)
{
	ASSERT(this, "Entity::getDirection: null this");

	Direction direction =
	{
		__RIGHT, __DOWN, __FAR
	};

	if((__QUARTER_ROTATION_DEGREES) < __ABS(this->transformation.globalRotation.y))
	{
		direction.x = __LEFT;
	}

	if((__QUARTER_ROTATION_DEGREES) < __ABS(this->transformation.globalRotation.x))
	{
		direction.y = __UP;
	}

	if((__QUARTER_ROTATION_DEGREES) < __ABS(this->transformation.globalRotation.z))
	{
		direction.z = __NEAR;
	}

	return direction;
}

u32 Entity_getShapesLayers(Entity this)
{
	ASSERT(this, "Entity::getShapesLayers: null this");

	u32 shapesLayers = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			shapesLayers |= Shape_getLayers(shape);
		}
	}

	return shapesLayers;
}

void Entity_setShapesLayers(Entity this, u32 layers)
{
	ASSERT(this, "Entity::setShapesLayers: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			Shape_setLayers(shape, layers);
		}
	}
}

u32 Entity_getShapesLayersToIgnore(Entity this)
{
	ASSERT(this, "Entity::getShapesLayersToIgnore: null this");

	u32 shapesLayersToIgnore = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			shapesLayersToIgnore |= Shape_getLayersToIgnore(shape);
		}
	}

	return shapesLayersToIgnore;
}

void Entity_setShapesLayersToIgnore(Entity this, u32 layersToIgnore)
{
	ASSERT(this, "Entity::setShapesLayersToIgnore: null this");

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			Shape_setLayersToIgnore(shape, layersToIgnore);
		}
	}
}
