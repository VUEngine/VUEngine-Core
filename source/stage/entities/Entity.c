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

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param entityDefinition
 * @param id
 * @param internalId
 * @param name
 */
void Entity::constructor(EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name)
{
	// construct base Container
	Base::constructor(name);

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
	this->size = Size::getFromPixelSize(entityDefinition->pixelSize);

	this->invalidateSprites = 0;
}

/**
 * Class destructor
 */
void Entity::destructor()
{
	Entity::destroyShapes(this);

	if(this->centerDisplacement)
	{
		delete this->centerDisplacement;
	}

	if(this->entityFactory)
	{
		delete this->entityFactory;
	}

	Entity::releaseSprites(this);

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Clean up method
 */
void Entity::iAmDeletingMyself()
{
	Base::iAmDeletingMyself(this);

	// destroy collision shapes
	Entity::activateShapes(this, false);
//	Entity::destroyShapes(this);
}

/**
 * Retrieve instance's in game id
 *
 * @return		Internal ID
 */
s16 Entity::getInternalId()
{
	return this->internalId;
}

/**
 * Retrieve instance's id as per its definition
 *
 * @return		ID
 */
s16 Entity::getId()
{
	return this->id;
}

/**
 * Get child by id
 *
 * @param id
 * @return		Child Entity
 */
Entity Entity::getChildById(s16 id)
{
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// look through all children
		for(; node ; node = node->next)
		{
			Entity child = Entity::safeCast(node->data);

			if(Entity::getId(child) == id)
			{
				return this->removedChildren && VirtualList::find(this->removedChildren, child) ? NULL : child;
			}
		}
	}

	return NULL;
}

/**
 * Set definition
 *
 * @param entityDefinition	EntityDefinition
 */
void Entity::setDefinition(void* entityDefinition)
{
	// save definition
	this->entityDefinition = entityDefinition;
}

/**
 * Destroy shapes
 *
 * @private
 */
void Entity::destroyShapes()
{
	if(this->shapes)
	{
		ASSERT(!isDeleted(this->shapes), "Entity::setDefinition: dead shapes");

		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			CollisionManager::destroyShape(Game::getCollisionManager(Game::getInstance()), Shape::safeCast(node->data));
		}

		delete this->shapes;
		this->shapes = NULL;
	}
}

/**
 * Add sprites
 */
void Entity::setupGraphics()
{
	Base::setupGraphics(this);

	Entity::addSprites(this, this->entityDefinition->spriteDefinitions);
}

/**
 * Release sprites
 */
void Entity::releaseGraphics()
{
	Base::releaseGraphics(this);

	Entity::releaseSprites(this);
}

/**
 * Delete all of the Entity's sprites
 */
void Entity::releaseSprites()
{
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		SpriteManager spriteManager = SpriteManager::getInstance();

		for(; node ; node = node->next)
		{
			SpriteManager::disposeSprite(spriteManager, Sprite::safeCast(node->data));
		}

		// delete the sprites
		delete this->sprites;
		this->sprites = NULL;
	}
}

/**
 * Calculate size from children
 *
 * @private
 * @param pixelRightBox
 * @param environmentPosition
 */
void Entity::calculateSizeFromChildren(PixelRightBox* pixelRightBox, Vector3D environmentPosition)
{
	PixelVector pixelGlobalPosition = PixelVector::getFromVector3D(environmentPosition, 0);

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
			Sprite sprite = Sprite::safeCast(spriteNode->data);
			ASSERT(sprite, "Entity::calculateSizeFromChildren: null sprite");
//			Sprite::resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			halfWidth = Sprite::getHalfWidth(sprite);
			halfHeight = Sprite::getHalfHeight(sprite);
			halfDepth = 16;

			PixelVector spriteDisplacement = Sprite::getDisplacement(sprite);

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
			Entity::calculateSizeFromChildren(childNode->data, pixelRightBox, Vector3D::getFromPixelVector(pixelGlobalPosition));
		}
	}
}

/**
 * Calculate my size based on me and my children
 *
 * @private
 */
void Entity::calculateSize()
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::calculateSizeFromChildren(this, &pixelRightBox, (Vector3D){0, 0, 0});

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
			delete this->centerDisplacement;
		}

		this->centerDisplacement = new Vector3D;
		*this->centerDisplacement = centerDisplacement;
	}

	this->size.x = __PIXELS_TO_METERS(pixelRightBox.x1 - pixelRightBox.x0);
	this->size.y = __PIXELS_TO_METERS(pixelRightBox.y1 - pixelRightBox.y0);
	this->size.z = __PIXELS_TO_METERS(pixelRightBox.z1 - pixelRightBox.z0);
}

/**
 * Get size from definition
 *
 * @private
 * @param positionedEntity		Function scope
 * @param environmentPosition
 * @param pixelRightBox
 */
static void Entity::getSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition, PixelRightBox* pixelRightBox)
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
			Entity::getSizeFromDefinition(&positionedEntity->childrenDefinitions[i], &pixelGlobalPosition, pixelRightBox);
		}
	}
}

/**
 * Calculate total size from definition
 *
 * @param positionedEntity		Function scope
 * @param environmentPosition
 * @return						PixelRightBox
 */
static PixelRightBox Entity::getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition)
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::getSizeFromDefinition(positionedEntity, environmentPosition, &pixelRightBox);

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
 * @param childrenDefinitions	Function scope
 * @param environmentPosition
 * @param childName
 * @return						Entity's global position
 */
static Vector3D* Entity::calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, Vector3D environmentPosition, const char* childName)
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

			Vector3D* position = Entity::calculateGlobalPositionFromDefinitionByName(childrenDefinitions[i].childrenDefinitions, concatenatedEnvironmentPosition, childName);

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
 * @param entityDefinition
 * @param id
 * @param internalId
 * @param name
 * @param extraInfo
 * @return					Entity instance
 */
static Entity Entity::instantiate(const EntityDefinition* const entityDefinition, s16 id, s16 internalId, const char* const name, void* extraInfo)
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
		Entity::setExtraInfo(entity, extraInfo);
	}

	return entity;
}

/**
 * Add children to the instance from the definitions array
 *
 * @param childrenDefinitions
 */
void Entity::addChildEntities(const PositionedEntity* childrenDefinitions)
{
	if(!childrenDefinitions)
	{
		return;
	}

	int i = 0;

	// go through n sprites in entity's definition
	for(; childrenDefinitions[i].entityDefinition; i++)
	{
		Entity child = Entity::loadEntity(&childrenDefinitions[i], this->internalId + Container::getChildCount(this));
		ASSERT(child, "Entity::loadChildren: entity not loaded");

		// create the entity and add it to the world
		Container::addChild(this, Container::safeCast(child));
	}
}

/**
 * Load an entity and instantiate all its children
 *
 * @param positionedEntity
 * @param internalId
 * @return					Entity
 */
static Entity Entity::loadEntity(const PositionedEntity* const positionedEntity, s16 internalId)
{
	ASSERT(positionedEntity, "Entity::loadFromDefinition: null positionedEntity");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity->entityDefinition, positionedEntity->id, internalId, positionedEntity->name, positionedEntity->extraInfo);
	ASSERT(entity, "Entity::loadFromDefinition: entity not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	Container::setLocalPosition(entity, &position);

	// add children if defined
	if(positionedEntity->childrenDefinitions)
	{
		Entity::addChildEntities(entity, positionedEntity->childrenDefinitions);
	}

	return entity;
}

/**
 * Add children to instance from the definitions array, but deferred
 *
 * @param childrenDefinitions
 */
void Entity::addChildEntitiesDeferred(const PositionedEntity* childrenDefinitions)
{
	ASSERT(childrenDefinitions, "Entity::addChildEntitiesDeferred: null childrenDefinitions");

	if(!childrenDefinitions)
	{
		return;
	}

	if(!this->entityFactory)
	{
		this->entityFactory = new EntityFactory();
	}

	int i = 0;

	// go through n sprites in entity's definition
	for(; childrenDefinitions[i].entityDefinition; i++)
	{
		EntityFactory::spawnEntity(this->entityFactory, &childrenDefinitions[i], Container::safeCast(this), NULL, this->internalId + Container::getChildCount(this));
	}
}

/**
 * Load an entity and instantiate all its children, deferred
 *
 * @param positionedEntity
 * @param internalId
 * @return					Entity
 */
static Entity Entity::loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 internalId)
{
	ASSERT(positionedEntity, "Entity::loadEntityDeferred: null positionedEntity");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity->entityDefinition, positionedEntity->id, internalId, positionedEntity->name, positionedEntity->extraInfo);
	ASSERT(entity, "Entity::loadEntityDeferred: entity not loaded");

	if(positionedEntity->name)
	{
		Container::setName(entity, positionedEntity->name);
	}

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	Container::setLocalPosition(entity, &position);

	// add children if defined
	if(positionedEntity->childrenDefinitions)
	{
		Entity::addChildEntitiesDeferred(entity, positionedEntity->childrenDefinitions);
	}

	return entity;
}

/**
 * Add child entity from definition
 *
 * @param entityDefinition
 * @param internalId
 * @param name
 * @param position
 * @param extraInfo
 * @return					Entity
 */
Entity Entity::addChildEntity(const EntityDefinition* entityDefinition, int internalId, const char* name, const Vector3D* position, void* extraInfo)
{
	ASSERT(entityDefinition, "Entity::addChildEntity: null entityDefinition");

	if(!entityDefinition)
	{
		return NULL;
	}

	PixelVector pixelPosition = PixelVector::getFromVector3D(*position, 0);

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
		this->internalId + Container::getChildCount(this),
		(char*)name,
		NULL,
		extraInfo,
		false
	};

	// load child entity
	Entity childEntity = Entity::loadEntity(&positionedEntity, 0 > internalId ? internalId : positionedEntity.id);
	ASSERT(childEntity, "Entity::addChildEntity: childEntity no created");

	// must add graphics
	Container::setupGraphics(childEntity);
	Entity::initialize(childEntity, true);

	// create the entity and add it to the world
	Container::addChild(this, Container::safeCast(childEntity));

	// apply transformations
	Transformation environmentTransform = Container::getEnvironmentTransform(this);
	Container::initialTransform(childEntity, &environmentTransform, true);

	// make ready
	Entity::ready(childEntity, true);

	return childEntity;
}

/**
 * Are all children instantiated?
 *
 * @return		Boolean whether all children are instantiated
 */
u32 Entity::areAllChildrenInstantiated()
{
	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory::instantiateEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children initialized?
 *
 * @return		Boolean whether all children are initialized
 */
u32 Entity::areAllChildrenInitialized()
{
	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory::initializeEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children transformed?
 *
 * @return		Boolean whether all children are transformed
 */
u32 Entity::areAllChildrenTransformed()
{
	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory::transformEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children ready?
 *
 * @return		Boolean whether all children are ready
 */
u32 Entity::areAllChildrenReady()
{
	if(this->entityFactory)
	{
		u32 returnValue = __LIST_EMPTY == EntityFactory::makeReadyEntities(this->entityFactory);

		if(!EntityFactory::hasEntitiesPending(this->entityFactory))
		{
			delete this->entityFactory;
			this->entityFactory = NULL;

			// must force size calculation now that all children are loaded
			Entity::calculateSize(this);
		}

		return returnValue;
	}

	Entity::calculateSize(this);

	return true;
}

/**
 * Set shape's position
 *
 * @private
 */
void Entity::transformShapes()
{
	if(this->shapes)
	{
		// setup shape
		//	bool isAffectedByRelativity =  SpatialObject::isAffectedByRelativity(this);
		const Vector3D* myPosition =  SpatialObject::getPosition(this);
		const Rotation* myRotation =  SpatialObject::getRotation(this);
		const Scale* myScale =  SpatialObject::getScale(this);

		Direction currentDirection = Entity::getDirection(this);
		VirtualNode node = this->shapes->head;
		if(this->entityDefinition->shapeDefinitions)
    	{
			const ShapeDefinition* shapeDefinitions = this->entityDefinition->shapeDefinitions;
			VirtualNode node = this->shapes->head;
			int i = 0;

			for(; node && shapeDefinitions[i].allocator; node = node->next, i++)
			{
				Shape shape = Shape::safeCast(node->data);
				u16 axesForShapeSyncWithDirection =  Entity::getAxesForShapeSyncWithDirection(this);

				Vector3D shapeDisplacement = Vector3D::getFromPixelVector(shapeDefinitions[i].displacement);

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

				Size size = Size::getFromPixelSize(shapeDefinitions[i].pixelSize);

				Shape::position(shape, &shapePosition, &shapeRotation, &shapeScale, &size);
			}
		}
		else
		{
			for(; node; node = node->next)
			{
				Shape shape = Shape::safeCast(node->data);
				Vector3D shapePosition = *myPosition;
				Rotation shapeRotation = *myRotation;
				Scale shapeScale = *myScale;
				Size size = this->size;

				Shape::position(shape, &shapePosition, &shapeRotation, &shapeScale, &size);
			}
		}
	}
}

/**
 * Setup shape
 *
 * @private
 * @param shapeDefinitions		List of shapes
 */
void Entity::addShapes(const ShapeDefinition* shapeDefinitions, bool destroyPreviousShapes)
{
	if(!shapeDefinitions)
	{
		return;
	}

	if(destroyPreviousShapes)
	{
		Entity::destroyShapes(this);
	}

	int i = 0;

	if(!this->shapes)
	{
		this->shapes = new VirtualList();
	}

	// go through n sprites in entity's definition
	for(; shapeDefinitions[i].allocator; i++)
	{
		Shape shape = CollisionManager::createShape(Game::getCollisionManager(Game::getInstance()), SpatialObject::safeCast(this), &shapeDefinitions[i]);
		ASSERT(shape, "Entity::addSprite: sprite not created");
		VirtualList::pushBack(this->shapes, shape);
	}

	Entity::transformShapes(this);
}

/**
 * Entity is initialized
 *
 * @param recursive
 */
void Entity::initialize(bool recursive)
{
	if(recursive && this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Entity::initialize(childNode->data, recursive);
		}
	}
}

/**
 * Entity is ready
 *
 * @param recursive
 */
void Entity::ready(bool recursive)
{
	if(recursive && this->children)
	{
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Entity::ready(childNode->data, recursive);
		}
	}
}

/**
 * Process extra info in initialization
 *
 * @param extraInfo
 */
void Entity::setExtraInfo(void* extraInfo __attribute__ ((unused)))
{
}

/**
 * Add sprites
 *
 * @param spriteDefinitions
 */
void Entity::addSprites(const SpriteDefinition** spriteDefinitions)
{
	if(!spriteDefinitions)
	{
		return;
	}

	if(!this->sprites)
	{
		this->sprites = new VirtualList();
	}

	SpriteManager spriteManager = SpriteManager::getInstance();

	int i = 0;

	// go through n sprites in entity's definition
	for(; spriteDefinitions[i]; i++)
	{
		ASSERT(spriteDefinitions[i]->allocator, "Entity::addSprites: no sprite allocator");

		VirtualList::pushBack(this->sprites, SpriteManager::createSprite(spriteManager, (SpriteDefinition*)spriteDefinitions[i], Object::safeCast(this)));
		ASSERT(Sprite::safeCast(VirtualList::back(this->sprites)), "Entity::addSprite: sprite not created");
	}

	// make sure that the new sprites are properly initialized
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
}

/**
 * Add sprite
 *
 * @param spriteDefinitionIndex		Index in sprite definitions array
 * @return							True if a sprite was created
 */
bool Entity::addSpriteFromDefinitionAtIndex(int spriteDefinitionIndex)
{
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
		this->sprites = new VirtualList();
	}

	// call the appropriate allocator to support inheritance
	VirtualList::pushBack(this->sprites, SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteDefinition*)spriteDefinition, Object::safeCast(this)));

	return true;
}

void Entity::updateSprites(u32 updatePosition, u32 updateScale, u32 updateRotation, u32 updateProjection)
{
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
			Sprite sprite = Sprite::safeCast(node->data);

			// update sprite's 2D position
			Sprite::position(sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			Sprite::rotate(sprite, &this->transformation.globalRotation);

			// calculate the scale
			Sprite::resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			Sprite::calculateParallax(sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition && updateRotation)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = Sprite::safeCast(node->data);

			// update sprite's 2D position
			Sprite::position(sprite, &this->transformation.globalPosition);

			// update sprite's 2D rotation
			Sprite::rotate(sprite, &this->transformation.globalRotation);
		}
	}
	else if(updatePosition && updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = Sprite::safeCast(node->data);

			// update sprite's 2D position
			Sprite::position(sprite, &this->transformation.globalPosition);

			// calculate the scale
			Sprite::resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			Sprite::calculateParallax(sprite, this->transformation.globalPosition.z);
		}
	}
	else if(updatePosition)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = Sprite::safeCast(node->data);

			// update sprite's 2D position
			Sprite::position(sprite, &this->transformation.globalPosition);
		}
	}
	else if(updateScale)
	{
		for(; node ; node = node->next)
		{
			Sprite sprite = Sprite::safeCast(node->data);

			// calculate the scale
			Sprite::resize(sprite, this->transformation.globalScale, this->transformation.globalPosition.z);

			// calculate sprite's parallax
			Sprite::calculateParallax(sprite, this->transformation.globalPosition.z);
		}
	}
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param recursive
 */
void Entity::initialTransform(const Transformation* environmentTransform, u32 recursive)
{
	// call base class's transformation method
	Base::initialTransform(this, environmentTransform, recursive);

	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;

	if(this->hidden)
	{
		Entity::hide(this);
	}

	// now can calculate the size
	if(recursive && (!this->size.x || !this->size.y || !this->size.z))
	{
		// must force size calculation now
		Entity::calculateSize(this);
	}

	// this method can be called multiple times so only add shapes
	// if not already done
	if(!this->shapes)
	{
		Entity::addShapes(this, this->entityDefinition->shapeDefinitions, false);
	}
}

/**
 * Transform class
 *
 * @param environmentTransform
 */
void Entity::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	if(this->sprites)
	{
		this->invalidateSprites |= invalidateTransformationFlag | Entity::updateSpritePosition(this) | Entity::updateSpriteRotation(this) | Entity::updateSpriteScale(this);
	}

	if(this->invalidateGlobalTransformation)
	{
		Entity::transformShapes(this);

		// call base class's transformation method
		Base::transform(this, environmentTransform, invalidateTransformationFlag);
	}
	else if((u32)this->children)
	{
		Base::transform(this, environmentTransform, invalidateTransformationFlag);
	}
}

/**
 * Set local position
 */
void Entity::setLocalPosition(const Vector3D* position)
{
	Base::setLocalPosition(this, position);

	Entity::transformShapes(this);
}

/**
 * Set local rotation
 */
void Entity::setLocalRotation(const Rotation* rotation)
{
	Base::setLocalRotation(this, rotation);

	Entity::transformShapes(this);
}

/**
 * Update visual representation
 */
void Entity::synchronizeGraphics()
{
	if(this->children)
	{
		Base::synchronizeGraphics(this);
	}

	Entity::updateSprites(this, this->invalidateSprites & __INVALIDATE_POSITION, this->invalidateSprites & __INVALIDATE_SCALE, this->invalidateSprites & __INVALIDATE_ROTATION, this->invalidateSprites & __INVALIDATE_PROJECTION);

	this->invalidateSprites = false;
}

/**
 * Retrieve EntityDefinition
 *
 * @return		EntityDefinition
 */
EntityDefinition* Entity::getEntityDefinition()
{
	return this->entityDefinition;
}

/**
 * Retrieve position
 *
 * @return		Global position
 */
const Vector3D* Entity::getPosition()
{
	return &this->transformation.globalPosition;
}

/**
 * Retrieve rotation
 *
 * @return		Global rotation
 */
const Rotation* Entity::getRotation()
{
	return &this->transformation.globalRotation;
}

/**
 * Retrieve scale
 *
 * @return		Global position
 */
const Scale* Entity::getScale()
{
	return &this->transformation.globalScale;
}

/**
 * Retrieve sprites
 *
 * @return		VirtualList of Entity's sprites
 */
VirtualList Entity::getSprites()
{
	return this->sprites;
}

/**
 * Handles incoming messages
 *
 * @param telegram
 * @return			True if successfully processed, false otherwise
 */
bool Entity::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

/**
 * Get width
 *
 * @return		Entity's width
 */
fix10_6 Entity::getWidth()
{
	if(!this->size.x)
	{
		Entity::calculateSize(this);
	}

	// must calculate based on the scale because not affine container must be enlarged
	return this->size.x;
}

/**
 * Get height
 *
 * @return		Entity's height
 */
fix10_6 Entity::getHeight()
{
	if(!this->size.y)
	{
		Entity::calculateSize(this);
	}

	return this->size.y;
}

/**
 * Get depth
 *
 * @return		Entity's depth
 */
fix10_6 Entity::getDepth()
{
	if(!this->size.z)
	{
		Entity::calculateSize(this);
	}

	// must calculate based on the scale because not affine object must be enlarged
	return this->size.z;
}

/**
 * Whether it is visible
 *
 * @param pad
 * @param recursive
 * @return			Boolean if visible
 */
bool Entity::isVisible(int pad, bool recursive)
{
	int x = 0;
	int y = 0;
	int z = 0;

	if(this->sprites && this->sprites->head)
	{
		VirtualNode spriteNode = this->sprites->head;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = Sprite::safeCast(spriteNode->data);
			ASSERT(sprite, "Entity:isVisible: null sprite");

			PixelVector spritePosition = Sprite::getDisplacedPosition(sprite);

			PixelSize pixelSize = PixelSize::getFromSize(this->size);

			s16 halfWidth	= pixelSize.x >> 1;
			s16 halfHeight	= pixelSize.y >> 1;
			s16 halfDepth	= pixelSize.z >> 1;

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
		Vector3D position3D = Vector3D::getRelativeToCamera(this->transformation.globalPosition);

		if(this->centerDisplacement)
		{
			position3D.x += this->centerDisplacement->x;
			position3D.y += this->centerDisplacement->y;
			position3D.z += this->centerDisplacement->z;
		}

		PixelVector position2D = Vector3D::projectToPixelVector(position3D, 0);

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
			if( Entity::isVisible(VirtualNode::getData(childNode), pad, true))
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
 * @return		Boolean if necessary
 */
bool Entity::updateSpritePosition()
{
	return __INVALIDATE_POSITION & this->invalidateGlobalTransformation;
}

/**
 * Check if necessary to update sprite's rotation
 *
 * @return		Boolean if necessary
 */
bool Entity::updateSpriteRotation()
{
	return __INVALIDATE_ROTATION & this->invalidateGlobalTransformation;
}

/**
 * Check if necessary to update sprite's scale
 *
 * @return		Boolean if necessary
 */
bool Entity::updateSpriteScale()
{
	return __INVALIDATE_SCALE & this->invalidateGlobalTransformation;
}

/**
 * Retrieve shapes list
 *
 * @return		Entity's Shape list
 */
VirtualList Entity::getShapes()
{
	return this->shapes;
}

/**
 * Make it visible
 */
void Entity::show()
{
	// update transformation before hiding
	Transformation environmentTransform = Container::getEnvironmentTransform(this);
	Container::transform(this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity::synchronizeGraphics(this);

	Base::show(this);

	// show all sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;
		for(; node ; node = node->next)
		{
			Sprite::show(node->data);
		}
	}

	// force invalid position to update sprites and children's sprites
	Container::invalidateGlobalTransformation(this);
}

/**
 * Make it invisible
 */
void Entity::hide()
{
	// update transformation before hiding
	Transformation environmentTransform = Container::getEnvironmentTransform(this);
	Container::transform(this, &environmentTransform, __INVALIDATE_TRANSFORMATION);

	// update the visual representation
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
	Entity::synchronizeGraphics(this);

	Base::hide(this);

	// hide all sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;
		for(; node ; node = node->next)
		{
			Sprite::hide(node->data);
		}
	}
}

/**
 * Suspend for pause
 */
void Entity::suspend()
{
	Base::suspend(this);

	Entity::releaseSprites(this);
}

/**
 * Resume after pause
 */
void Entity::resume()
{
	Base::resume(this);

	// initialize sprites
	if(this->entityDefinition)
	{
		Entity::addSprites(this, this->entityDefinition->spriteDefinitions);
	}

	if(this->hidden)
	{
		Entity::hide(this);
	}

	// force update sprites on next game's cycle
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;
}

/**
 * Defaults to true
 *
 * @param acceleration
 * @return				Defaults to true
 */
bool Entity::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return true;
}

/**
 * Get in game type
 *
 * @return		Type of entity within the game's logic
 */
u32 Entity::getInGameType()
{
	return this->entityDefinition->inGameType;
}

/**
 * Get bounciness for physics
 *
 * @return		Bounciness
 */
fix10_6 Entity::getBounciness()
{
	return this->entityDefinition->physicalSpecification ? this->entityDefinition->physicalSpecification->bounciness : 0;
}

/**
 * Get friction
 *
 * @return		Friction
 */
fix10_6 Entity::getFrictionCoefficient()
{
	return this->entityDefinition->physicalSpecification ? this->entityDefinition->physicalSpecification->frictionCoefficient : 0;
}

/**
 * Propagate that movement started to the shapes
 */
void Entity::informShapesThatStartedMoving()
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			CollisionManager::shapeStartedMoving(Game::getCollisionManager(Game::getInstance()), shape);
		}
	}
}

/**
 * Propagate that movement stopped to the shapes
 */
void Entity::informShapesThatStoppedMoving()
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			CollisionManager::shapeStoppedMoving(Game::getCollisionManager(Game::getInstance()), Shape::safeCast(node->data));
		}
	}
}

/**
 * Propagate active status to the shapes
 */
void Entity::activateShapes(bool value)
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape::setActive(node->data, value);
		}
	}
}

/**
 * Returns whether I have collision shapes or not
 */
bool Entity::hasShapes()
{
	return NULL != this->shapes && 0 < VirtualList::getSize(this->shapes);
}

void Entity::showShapes()
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape::show(node->data);
		}
	}
}

void Entity::hideShapes()
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape::hide(node->data);
		}
	}
}


/**
 * Set direction
 *
 * @param direction		Direction
 */
void Entity::setDirection(Direction direction)
{
	Direction currentDirection = Entity::getDirection(this);

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

	Container::setLocalRotation(this, &rotation);
}

/**
 * Get direction
 *
 * @return		Direction
 */
Direction Entity::getDirection()
{
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
 * @return		Shape layers
 */
u32 Entity::getShapesLayers()
{
	u32 shapesLayers = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			shapesLayers |= Shape::getLayers(shape);
		}
	}

	return shapesLayers;
}

/**
 * Set Shape layers
 *
 * @param u32	Shape layers
 */
void Entity::setShapesLayers(u32 layers)
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::setLayers(shape, layers);
		}
	}
}

/**
 * Get Shape layers to ignore
 *
 * @return		Shape layers to ignore
 */
u32 Entity::getShapesLayersToIgnore()
{
	u32 shapesLayersToIgnore = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			shapesLayersToIgnore |= Shape::getLayersToIgnore(shape);
		}
	}

	return shapesLayersToIgnore;
}

/**
 * Set Shape layers to ignore
 *
 * @param u32	Shape layers to ignore
 */
void Entity::setShapesLayersToIgnore(u32 layersToIgnore)
{
	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::setLayersToIgnore(shape, layersToIgnore);
		}
	}
}

/**
 * Get axes for Shape sync with direction
 *
 * @return		Axes
 */
u16 Entity::getAxesForShapeSyncWithDirection()
{
	return __ALL_AXES;
}

/**
 * Whether to respawn this Entity after it has been streamed out
 *
 * @return		Boolean whether to respawn this Entity
 */
bool Entity::respawn()
{
	return true;
}