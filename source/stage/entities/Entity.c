/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Camera.h>
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

static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation, u32 updateProjection);
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
	this->size = Size_getFromPixelSize(entityDefinition->pixelSize);

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

	Entity_releaseSprites(this);

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Clean up method
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_iAmDeletingMyself(Entity this)
{
	ASSERT(this, "Entity::iAmDeletingMyself: null this");

	__CALL_BASE_METHOD(Container, iAmDeletingMyself, this);

	// destroy collision shapes
	Entity_activateShapes(this, false);
//	Entity_destroyShapes(this);
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
		VirtualNode node = this->children->head;

		// look through all children
		for(; node ; node = node->next)
		{
			Entity child = __SAFE_CAST(Entity, node->data);

			if(Entity_getId(child) == id)
			{
				return this->removedChildren && VirtualList_find(this->removedChildren, child) ? NULL : child;
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
 * @param entityDefinition	EntityDefinition
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
 * @private
 *
 * @param this	Function scope
 */
static void Entity_destroyShapes(Entity this)
{
	ASSERT(this, "Entity::setDefinition: null this");

	if(this->shapes)
	{
		ASSERT(__IS_OBJECT_ALIVE(this->shapes), "Entity::setDefinition: dead shapes");

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

	Entity_releaseSprites(this);
}

/**
 * Delete all of the Entity's sprites
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 */
void Entity_releaseSprites(Entity this)
{
	ASSERT(this, "Entity::releaseSprites: null this");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		SpriteManager spriteManager = SpriteManager_getInstance();

		for(; node ; node = node->next)
		{
			SpriteManager_disposeSprite(spriteManager, __SAFE_CAST(Sprite, node->data));
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
 * @param pixelRightBox
 * @param environmentPosition
 */
static void Entity_calculateSizeFromChildren(Entity this, PixelRightBox* pixelRightBox, Vector3D environmentPosition)
{
	ASSERT(this, "Entity::calculateSizeFromChildren: null this");

	PixelVector pixelGlobalPosition = PixelVector_getFromVector3D(environmentPosition, 0);

	pixelGlobalPosition.x += __METERS_TO_PIXELS(this->transformation.localPosition.x);
	pixelGlobalPosition.y += __METERS_TO_PIXELS(this->transformation.localPosition.y);
	pixelGlobalPosition.z += __METERS_TO_PIXELS(this->transformation.localPosition.z);

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
			ASSERT(sprite, "Entity::calculateSizeFromChildren: null sprite");
//			 Sprite_resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			halfWidth = Sprite_getHalfWidth(sprite);
			halfHeight = Sprite_getHalfHeight(sprite);
			halfDepth = 16;

			PixelVector spriteDisplacement = Sprite_getDisplacement(sprite);

			if(left > -halfWidth + spriteDisplacement.x)
			{
				left = -halfWidth + spriteDisplacement.x;
			}

			if(right < halfWidth + spriteDisplacement.x)
			{
				right = halfWidth + spriteDisplacement.x;
			}

			if(top > -halfHeight + spriteDisplacement.y)
			{
				top = -halfHeight + spriteDisplacement.y;
			}

			if(bottom < halfHeight + spriteDisplacement.y)
			{
				bottom = halfHeight + spriteDisplacement.y;
			}

			if(front > -halfDepth + spriteDisplacement.z)
			{
				front = -halfDepth + spriteDisplacement.z;
			}

			if(back < halfDepth + spriteDisplacement.z)
			{
				back = halfDepth + spriteDisplacement.z;
			}
		}
	}
	else
	{
		right = __METERS_TO_PIXELS(this->size.x >> 1);
		left = -right;
		bottom = __METERS_TO_PIXELS(this->size.y >> 1);
		top = -bottom;
		back = __METERS_TO_PIXELS(this->size.z >> 1);
		front = -back;
	}

	if((0 == pixelRightBox->x0) | (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) | (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) | (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) | (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) | (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) | (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(this->children)
	{
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Entity_calculateSizeFromChildren(__SAFE_CAST(Entity, childNode->data), pixelRightBox, Vector3D_getFromPixelVector(pixelGlobalPosition));
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

	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity_calculateSizeFromChildren(this, &pixelRightBox, (Vector3D){0, 0, 0});

	Vector3D centerDisplacement =
	{
		(__PIXELS_TO_METERS(pixelRightBox.x1 + pixelRightBox.x0) >> 1) - this->transformation.localPosition.x,
		(__PIXELS_TO_METERS(pixelRightBox.y1 + pixelRightBox.y0) >> 1) - this->transformation.localPosition.y,
		(__PIXELS_TO_METERS(pixelRightBox.z1 + pixelRightBox.z0) >> 1) - this->transformation.localPosition.z
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

	this->size.x = __PIXELS_TO_METERS(pixelRightBox.x1 - pixelRightBox.x0);
	this->size.y = __PIXELS_TO_METERS(pixelRightBox.y1 - pixelRightBox.y0);
	this->size.z = __PIXELS_TO_METERS(pixelRightBox.z1 - pixelRightBox.z0);
}

/**
 * Get size from definition
 *
 * @memberof					Entity
 * @private
 *
 * @param positionedEntity		Function scope
 * @param environmentPosition
 * @param pixelRightBox
 */
static void Entity_getSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition, PixelRightBox* pixelRightBox)
{
	ASSERT(positionedEntity, "Entity::getSizeFromDefinition: null positionedEntity");
	ASSERT(positionedEntity->entityDefinition, "Entity::getSizeFromDefinition: null entityDefinition");

	PixelVector pixelGlobalPosition = *environmentPosition;

	pixelGlobalPosition.x += positionedEntity->onScreenPosition.x;
	pixelGlobalPosition.y += positionedEntity->onScreenPosition.y;
	pixelGlobalPosition.z += positionedEntity->onScreenPosition.z;

	s16 left = 0;
	s16 right = 0;
	s16 top = 0;
	s16 bottom = 0;
	s16 front = 0;
	s16 back = 0;
	s16 halfWidth = 0;
	s16 halfHeight = 0;
	s16 halfDepth = 5;

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
					if(halfWidth < (s16)(mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2))
					{
						halfWidth = mBgmapSpriteDefinition->textureDefinitions[j]->cols << 2;
					}

					if(halfHeight < (s16)(mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2))
					{
						halfHeight = (s16)(mBgmapSpriteDefinition->textureDefinitions[j]->rows << 2);
					}
				}

				if(left > -halfWidth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x)
				{
					left = -halfWidth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x;
				}

				if(right < halfWidth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x)
				{
					right = halfWidth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.x;
				}

				if(top > -halfHeight + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y)
				{
					top = -halfHeight + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y;
				}

				if(bottom < halfHeight + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y)
				{
					bottom = halfHeight + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.y;
				}

				if(front > mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z)
				{
					front = mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z;
				}

				if(back < halfDepth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z)
				{
					back = halfDepth + mBgmapSpriteDefinition->bgmapSpriteDefinition.spriteDefinition.displacement.z;
				}

			}
			else if(positionedEntity->entityDefinition->spriteDefinitions[i]->textureDefinition)
			{
				SpriteDefinition* spriteDefinition = (SpriteDefinition*)positionedEntity->entityDefinition->spriteDefinitions[i];
				halfWidth = spriteDefinition->textureDefinition->cols << 2;
				halfHeight = spriteDefinition->textureDefinition->rows << 2;
				halfDepth = 16;

				if(left > -halfWidth + spriteDefinition->displacement.x)
				{
					left = -halfWidth + spriteDefinition->displacement.x;
				}

				if(right < halfWidth + spriteDefinition->displacement.x)
				{
					right = halfWidth + spriteDefinition->displacement.x;
				}

				if(top > -halfHeight + spriteDefinition->displacement.y)
				{
					top = -halfHeight + spriteDefinition->displacement.y;
				}

				if(bottom < halfHeight + spriteDefinition->displacement.y)
				{
					bottom = halfHeight + spriteDefinition->displacement.y;
				}

				if(front > -halfDepth + spriteDefinition->displacement.z)
				{
					front = -halfDepth + spriteDefinition->displacement.z;
				}

				if(back < (halfDepth << 1) + spriteDefinition->displacement.z)
				{
					back = (halfDepth << 1) + spriteDefinition->displacement.z;
				}
			}
		}
	}
	else if(!positionedEntity->childrenDefinitions)
	{
		// TODO: there should be a class which handles special cases
		halfWidth = positionedEntity->entityDefinition->pixelSize.x >> 1;
		halfHeight = positionedEntity->entityDefinition->pixelSize.y >> 1;
		halfDepth = positionedEntity->entityDefinition->pixelSize.z >> 1;

		left = -halfWidth;
		right = halfWidth;
		top = -halfHeight;
		bottom = halfHeight;
		front = -halfDepth;
		back = halfDepth;
	}

	if((0 == pixelRightBox->x0) | (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) | (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) | (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) | (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) | (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) | (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(positionedEntity->childrenDefinitions)
	{
		int i = 0;
		for(; positionedEntity->childrenDefinitions[i].entityDefinition; i++)
		{
			Entity_getSizeFromDefinition(&positionedEntity->childrenDefinitions[i], &pixelGlobalPosition, pixelRightBox);
		}
	}
}

/**
 * Calculate total size from definition
 *
 * @memberof					Entity
 * @public
 *
 * @param positionedEntity		Function scope
 * @param environmentPosition
 *
 * @return						PixelRightBox
 */
PixelRightBox Entity_getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition)
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity_getSizeFromDefinition(positionedEntity, environmentPosition, &pixelRightBox);

	pixelRightBox.x0 = pixelRightBox.x0 - positionedEntity->onScreenPosition.x;
	pixelRightBox.x1 = pixelRightBox.x1 - positionedEntity->onScreenPosition.x;
	pixelRightBox.y0 = pixelRightBox.y0 - positionedEntity->onScreenPosition.y;
	pixelRightBox.y1 = pixelRightBox.y1 - positionedEntity->onScreenPosition.y;
	pixelRightBox.z0 = pixelRightBox.z0 - positionedEntity->onScreenPosition.z;
	pixelRightBox.z1 = pixelRightBox.z1 - positionedEntity->onScreenPosition.z;

	return pixelRightBox;
}

/**
 * Find child by name in given list
 *
 * @memberof					Entity
 * @public
 *
 * @param childrenDefinitions	Function scope
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
			position.x = environmentPosition.x + __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.x);
			position.y = environmentPosition.y + __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.y);
			position.z = environmentPosition.z + __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.z + childrenDefinitions[i].onScreenPosition.zDisplacement);
			return &position;
		}

		if(childrenDefinitions[i].childrenDefinitions)
		{
			Vector3D concatenatedEnvironmentPosition = environmentPosition;
			concatenatedEnvironmentPosition.x += __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.x);
			concatenatedEnvironmentPosition.y += __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.y);
			concatenatedEnvironmentPosition.z += __PIXELS_TO_METERS(childrenDefinitions[i].onScreenPosition.z + childrenDefinitions[i].onScreenPosition.zDisplacement);

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
		 Entity_setExtraInfo(entity, extraInfo);
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
 * @memberof				Entity
 * @public
 *
 * @param positionedEntity
 * @param internalId
 *
 * @return					Entity
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

	Vector3D position = Vector3D_getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	 Container_setLocalPosition(entity, &position);

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

	Vector3D position = Vector3D_getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	 Container_setLocalPosition(entity, &position);

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

	PixelVector pixelPosition = PixelVector_getFromVector3D(*position, 0);

	ScreenPixelVector screenPixelVector =
	{
		pixelPosition.x,
		pixelPosition.y,
		pixelPosition.z,
		0
	};

	PositionedEntity positionedEntity =
	{
		(EntityDefinition*)entityDefinition,
		screenPixelVector,
		this->internalId + Container_getChildCount(__SAFE_CAST(Container, this)),
		(char*)name,
		NULL,
		extraInfo,
		false
	};

	// load child entity
	Entity childEntity = Entity_loadEntity(&positionedEntity, 0 > internalId ? internalId : positionedEntity.id);
	ASSERT(childEntity, "Entity::addChildEntity: childEntity no created");

	// must add graphics
	 Container_setupGraphics(childEntity);
	 Entity_initialize(childEntity, true);

	// create the entity and add it to the world
	Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, childEntity));

	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));

	 // apply transformations
	 Container_initialTransform(childEntity, &environmentTransform, true);

	// make ready
	 Entity_ready(childEntity, true);

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
//		bool isAffectedByRelativity =  SpatialObject_isAffectedByRelativity(this);
		const Vector3D* myPosition =  SpatialObject_getPosition(this);
		const Rotation* myRotation =  SpatialObject_getRotation(this);
		const Scale* myScale =  SpatialObject_getScale(this);

		Direction currentDirection = Entity_getDirection(this);
		VirtualNode node = this->shapes->head;
		int i = 0;

		for(; node && shapeDefinitions[i].allocator; node = node->next, i++)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);
			u16 axesForShapeSyncWithDirection =  Entity_getAxesForShapeSyncWithDirection(this);

			Vector3D shapeDisplacement = Vector3D_getFromPixelVector(shapeDefinitions[i].displacement);

			Vector3D shapePosition =
			{
				myPosition->x + ((__X_AXIS & axesForShapeSyncWithDirection) && __LEFT == currentDirection.x ? -shapeDisplacement.x : shapeDisplacement.x),
				myPosition->y + ((__Y_AXIS & axesForShapeSyncWithDirection) && __UP == currentDirection.y ? -shapeDisplacement.y : shapeDisplacement.y),
				myPosition->z + ((__Z_AXIS & axesForShapeSyncWithDirection) && __NEAR == currentDirection.z ? -shapeDisplacement.z : shapeDisplacement.z),
			};

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

			Size size = Size_getFromPixelSize(shapeDefinitions[i].pixelSize);

			 Shape_position(shape, &shapePosition, &shapeRotation, &shapeScale, &size);
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
			 Entity_initialize(childNode->data, recursive);
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
			 Entity_ready(childNode->data, recursive);
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

	if(!this->sprites)
	{
		this->sprites = __NEW(VirtualList);
	}

	SpriteManager spriteManager = SpriteManager_getInstance();

	int i = 0;

	// go through n sprites in entity's definition
	for(; spriteDefinitions[i]; i++)
	{
		ASSERT(spriteDefinitions[i]->allocator, "Entity::addSprites: no sprite allocator");

		VirtualList_pushBack(this->sprites, SpriteManager_createSprite(spriteManager, (SpriteDefinition*)spriteDefinitions[i], __SAFE_CAST(Object, this)));
		ASSERT(__SAFE_CAST(Sprite, VirtualList_back(this->sprites)), "Entity::addSprite: sprite not created");
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

	if(!this->sprites)
	{
		this->sprites = __NEW(VirtualList);
	}

	// call the appropriate allocator to support inheritance
	VirtualList_pushBack(this->sprites, SpriteManager_createSprite(SpriteManager_getInstance(), (SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this)));

	return true;
}

static void Entity_updateSprites(Entity this, u32 updatePosition, u32 updateScale, u32 updateRotation, u32 updateProjection)
{
	ASSERT(this, "Entity::transform: null this");

	if(!this->sprites)
	{
		return;
	}

	updatePosition |= updateRotation;
	updatePosition |= updateProjection;

	VirtualNode node = this->sprites->head;

	if(updatePosition && updateRotation && updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			 Sprite_position(sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			 Sprite_rotate(sprite, &this->transformation.globalRotation);

			// calculate the scale
			 Sprite_resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			 Sprite_calculateParallax(sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition && updateRotation)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			 Sprite_position(sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			 Sprite_rotate(sprite, &this->transformation.globalRotation);
		}
	}
	else if(updatePosition && updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			 Sprite_position(sprite, &this->transformation.globalPosition);

			// calculate the scale
			 Sprite_resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			 Sprite_calculateParallax(sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// update sprite's 2D position
			 Sprite_position(sprite, &this->transformation.globalPosition);
		}
	}
	else if(updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			// calculate the scale
			 Sprite_resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			 Sprite_calculateParallax(sprite, this->transformation.globalPosition.z);
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

	Entity_updateSprites(this, this->invalidateSprites & __INVALIDATE_POSITION, this->invalidateSprites & __INVALIDATE_SCALE, this->invalidateSprites & __INVALIDATE_ROTATION, this->invalidateSprites & __INVALIDATE_PROJECTION);

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
fix10_6 Entity_getWidth(Entity this)
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
fix10_6 Entity_getHeight(Entity this)
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
fix10_6 Entity_getDepth(Entity this)
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

			PixelVector spritePosition = Sprite_getDisplacedPosition(sprite);

			PixelSize pixelSize = PixelSize_getFromSize(this->size);

			s16 halfWidth	 = pixelSize.x >> 1;
			s16 halfHeight	 = pixelSize.y >> 1;
			s16 halfDepth	 = pixelSize.z >> 1;

			x = spritePosition.x;
			y = spritePosition.y;
			z = spritePosition.z;

			// check x visibility
			if((x + halfWidth < _cameraFrustum->x0 - pad) || (x - halfWidth > _cameraFrustum->x1 + pad))
			{
				continue;
			}

			// check y visibility
			if((y + halfHeight < _cameraFrustum->y0 - pad) || (y - halfHeight > _cameraFrustum->y1 + pad))
			{
				continue;
			}

			// check z visibility
			if((z + halfDepth < _cameraFrustum->z0 - pad) || (z - halfDepth > _cameraFrustum->z1 + pad))
			{
				continue;
			}

			return true;
		}
	}
	else
	{
		Vector3D position3D = Vector3D_getRelativeToCamera(this->transformation.globalPosition);

		if(this->centerDisplacement)
		{
			position3D.x += this->centerDisplacement->x;
			position3D.y += this->centerDisplacement->y;
			position3D.z += this->centerDisplacement->z;
		}

		PixelVector position2D = Vector3D_projectToPixelVector(position3D, 0);

		s16 halfWidth = __METERS_TO_PIXELS(this->size.x >> 1);
		s16 halfHeight = __METERS_TO_PIXELS(this->size.y >> 1);
		s16 halfDepth = __METERS_TO_PIXELS(this->size.z >> 1);

		x = position2D.x;
		y = position2D.y;
		z = position2D.z;

		// check x visibility
		if((x + halfWidth < _cameraFrustum->x0 - pad) || (x - halfWidth > _cameraFrustum->x1 + pad))
		{
			return false;
		}

		// check y visibility
		if((y + halfHeight < _cameraFrustum->y0 - pad) || (y - halfHeight > _cameraFrustum->y1 + pad))
		{
			return false;
		}

		// check z visibility
		if((z + halfDepth < _cameraFrustum->z0 - pad) || (z - halfDepth > _cameraFrustum->z1 + pad))
		{
			return false;
		}

		return true;
	}

	if(recursive && this->children)
	{
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			if( Entity_isVisible(VirtualNode_getData(childNode), pad, true))
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
	 Container_transform(this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity_synchronizeGraphics(this);

	__CALL_BASE_METHOD(Container, show, this);

	// show all sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;
		for(; node ; node = node->next)
		{
			 Sprite_show(__SAFE_CAST(Sprite, node->data));
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
	 Container_transform(this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity_synchronizeGraphics(this);

	__CALL_BASE_METHOD(Container, hide, this);

	// hide all sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;
		for(; node ; node = node->next)
		{
			 Sprite_hide(__SAFE_CAST(Sprite, node->data));
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

	Entity_releaseSprites(this);
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
 * Get bounciness for physics
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Bounciness
 */
fix10_6 Entity_getBounciness(Entity this)
{
	ASSERT(this, "Entity::getBounciness: null this");

	return this->entityDefinition->physicalSpecification ? this->entityDefinition->physicalSpecification->bounciness : 0;
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
fix10_6 Entity_getFrictionCoefficient(Entity this)
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
			Shape shape = __SAFE_CAST(Shape, node->data);

			CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), shape);
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
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Direction
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

/**
 * Get Shape layers
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Shape layers
 */
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

/**
 * Set Shape layers
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 * @param u32	Shape layers
 */
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

/**
 * Get Shape layers to ignore
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Shape layers to ignore
 */
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

/**
 * Set Shape layers to ignore
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 * @param u32	Shape layers to ignore
 */
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

/**
 * Get axes for Shape sync with direction
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Axes
 */
u16 Entity_getAxesForShapeSyncWithDirection(Entity this __attribute__ ((unused)))
{
	ASSERT(this, "Entity::getAxesForShapeSyncWithDirection: null this");

	return __ALL_AXES;
}

/**
 * Whether to respawn this Entity after it has been streamed out
 *
 * @memberof	Entity
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether to respawn this Entity
 */
bool Entity_respawn(Entity this __attribute__ ((unused)))
{
	ASSERT(this, "Entity::respawn: null this");

	return true;
}
