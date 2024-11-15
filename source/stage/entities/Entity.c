/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <string.h>

#include <Behavior.h>
#include <BgmapSprite.h>
#include <CollisionManager.h>
#include <DebugConfig.h>
#include <EntityFactory.h>
#include <MBgmapSprite.h>
#include <Mesh.h>
#include <Optics.h>
#include <Printing.h>
#include <Collider.h>
#include <SpriteManager.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Entity.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define ENTITY_SPRITE_DEPTH						16
#define ENTITY_SPRITE_HALF_DEPTH				(ENTITY_SPRITE_DEPTH >> 1)


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static Entity Entity::createEntity(const PositionedEntity* const positionedEntity, int16 internalId)
{
	NM_ASSERT(NULL != positionedEntity, "Entity::createEntity: null positionedEntity");
	NM_ASSERT(NULL != positionedEntity->entitySpec, "Entity::createEntity: null spec");
	NM_ASSERT(NULL != positionedEntity->entitySpec->allocator, "Entity::createEntity: no allocator defined");

	if(NULL == positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity, internalId, positionedEntity->name);
	ASSERT(entity, "Entity::loadFromSpec: entity not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);
	Rotation rotation = Rotation::getFromScreenPixelRotation(positionedEntity->onScreenRotation);
	Scale scale = Scale::getFromScreenPixelScale(positionedEntity->onScreenScale);

	Entity::setLocalPosition(entity, &position);
	Entity::setLocalRotation(entity, &rotation);
	Entity::setLocalScale(entity, &scale);

	// add children if defined
	if(NULL != positionedEntity->childrenSpecs)
	{
		Entity::addChildEntities(entity, positionedEntity->childrenSpecs);
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		Entity::addChildEntities(entity, positionedEntity->entitySpec->childrenSpecs);
	}

	return entity;
}
//---------------------------------------------------------------------------------------------------------
static Entity Entity::createEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId)
{
	NM_ASSERT(NULL != positionedEntity, "Entity::createEntityDeferred: null positionedEntity");
	NM_ASSERT(NULL != positionedEntity->entitySpec, "Entity::createEntityDeferred: null spec");
	NM_ASSERT(NULL != positionedEntity->entitySpec->allocator, "Entity::createEntityDeferred: no allocator defined");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity, internalId, positionedEntity->name);

	NM_ASSERT(!isDeleted(entity), "Entity::createEntityDeferred: entity not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);
	Rotation rotation = Rotation::getFromScreenPixelRotation(positionedEntity->onScreenRotation);
	Scale scale = Scale::getFromScreenPixelScale(positionedEntity->onScreenScale);

	Entity::setLocalPosition(entity, &position);
	Entity::setLocalRotation(entity, &rotation);
	Entity::setLocalScale(entity, &scale);

	// add children if defined
	if(positionedEntity->childrenSpecs)
	{
		Entity::addChildEntitiesDeferred(entity, positionedEntity->childrenSpecs);
	}

	if(positionedEntity->entitySpec->childrenSpecs)
	{
		Entity::addChildEntitiesDeferred(entity, positionedEntity->entitySpec->childrenSpecs);
	}

	return entity;
}
//---------------------------------------------------------------------------------------------------------
static PixelRightBox Entity::getBoundingBoxFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition)
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::getSizeFromSpec(positionedEntity, environmentPosition, &pixelRightBox);

	pixelRightBox.x0 = pixelRightBox.x0 - positionedEntity->onScreenPosition.x;
	pixelRightBox.x1 = pixelRightBox.x1 - positionedEntity->onScreenPosition.x;
	pixelRightBox.y0 = pixelRightBox.y0 - positionedEntity->onScreenPosition.y;
	pixelRightBox.y1 = pixelRightBox.y1 - positionedEntity->onScreenPosition.y;
	pixelRightBox.z0 = pixelRightBox.z0 - positionedEntity->onScreenPosition.z;
	pixelRightBox.z1 = pixelRightBox.z1 - positionedEntity->onScreenPosition.z;

	return pixelRightBox;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void Entity::getSizeFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition, PixelRightBox* pixelRightBox)
{
	ASSERT(positionedEntity, "Entity::getSizeFromSpec: null positionedEntity");
	ASSERT(positionedEntity->entitySpec, "Entity::getSizeFromSpec: null entitySpec");

	PixelVector pixelGlobalPosition = *environmentPosition;

	pixelGlobalPosition.x += positionedEntity->onScreenPosition.x;
	pixelGlobalPosition.y += positionedEntity->onScreenPosition.y;
	pixelGlobalPosition.z += positionedEntity->onScreenPosition.z;

	int16 left = 0;
	int16 right = 0;
	int16 top = 0;
	int16 bottom = 0;
	int16 front = 0;
	int16 back = 0;
	int16 halfWidth = 0;
	int16 halfHeight = 0;
	int16 halfDepth = ENTITY_SPRITE_HALF_DEPTH;

	if(0 != positionedEntity->entitySpec->pixelSize.x || 0 != positionedEntity->entitySpec->pixelSize.y || 0 != positionedEntity->entitySpec->pixelSize.z)
	{
		// TODO: there should be a class which handles special cases
		halfWidth = positionedEntity->entitySpec->pixelSize.x >> 1;
		halfHeight = positionedEntity->entitySpec->pixelSize.y >> 1;
		halfDepth = positionedEntity->entitySpec->pixelSize.z >> 1;

		left = -halfWidth;
		right = halfWidth;
		top = -halfHeight;
		bottom = halfHeight;
		front = -halfDepth;
		back = halfDepth;
	}
	else 
	{
		if(NULL != positionedEntity->entitySpec->spriteSpecs && NULL != positionedEntity->entitySpec->spriteSpecs[0])
		{
			int32 i = 0;

			for(; NULL != positionedEntity->entitySpec->spriteSpecs[i]; i++)
			{
				if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(positionedEntity->entitySpec->spriteSpecs[i]->allocator && ((MBgmapSpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i])->textureSpecs[0]))
				{
					MBgmapSpriteSpec* mBgmapSpriteSpec = (MBgmapSpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i];

					int32 j = 0;

					halfWidth = 0;
					halfHeight = 0;
					halfDepth = ENTITY_SPRITE_HALF_DEPTH;

					for(; mBgmapSpriteSpec->textureSpecs[j]; j++)
					{
						if(halfWidth < (int16)(mBgmapSpriteSpec->textureSpecs[j]->cols << 2))
						{
							halfWidth = mBgmapSpriteSpec->textureSpecs[j]->cols << 2;
						}

						if(halfHeight < (int16)(mBgmapSpriteSpec->textureSpecs[j]->rows << 2))
						{
							halfHeight = (int16)(mBgmapSpriteSpec->textureSpecs[j]->rows << 2);
						}
					}

					if(left > -halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x)
					{
						left = -halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x;
					}

					if(right < halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x)
					{
						right = halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x;
					}

					if(top > -halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y)
					{
						top = -halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y;
					}

					if(bottom < halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y)
					{
						bottom = halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y;
					}

					if(front > mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z)
					{
						front = mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z;
					}

					if(back < halfDepth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z)
					{
						back = halfDepth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z;
					}

				}
				else if(NULL != positionedEntity->entitySpec->spriteSpecs[i]->textureSpec)
				{
					SpriteSpec* spriteSpec = (SpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i];
					halfWidth = spriteSpec->textureSpec->cols << 2;
					halfHeight = spriteSpec->textureSpec->rows << 2;
					halfDepth = ENTITY_SPRITE_HALF_DEPTH;

					if(left > -halfWidth + spriteSpec->displacement.x)
					{
						left = -halfWidth + spriteSpec->displacement.x;
					}

					if(right < halfWidth + spriteSpec->displacement.x)
					{
						right = halfWidth + spriteSpec->displacement.x;
					}

					if(top > -halfHeight + spriteSpec->displacement.y)
					{
						top = -halfHeight + spriteSpec->displacement.y;
					}

					if(bottom < halfHeight + spriteSpec->displacement.y)
					{
						bottom = halfHeight + spriteSpec->displacement.y;
					}

					if(front > -halfDepth + spriteSpec->displacement.z)
					{
						front = -halfDepth + spriteSpec->displacement.z;
					}

					if(back < halfDepth + spriteSpec->displacement.z)
					{
						back = halfDepth + spriteSpec->displacement.z;
					}
				}
			}
		}

		if(NULL != positionedEntity->entitySpec->wireframeSpecs && NULL != positionedEntity->entitySpec->wireframeSpecs[0])
		{
			int32 i = 0;

			for(; NULL != positionedEntity->entitySpec->wireframeSpecs[i]; i++)
			{
				if(__TYPE(Mesh) == __ALLOCATOR_TYPE(positionedEntity->entitySpec->wireframeSpecs[i]->allocator && ((MeshSpec*)positionedEntity->entitySpec->wireframeSpecs[i])->segments[0]))
				{
					PixelRightBox pixelRightBox = Mesh::getPixelRightBoxFromSpec((MeshSpec*)positionedEntity->entitySpec->wireframeSpecs[i]);

					if(left > pixelRightBox.x0)
					{
						left = pixelRightBox.x0;
					}

					if(right < pixelRightBox.x1)
					{
						right = pixelRightBox.x1;
					}

					if(top > pixelRightBox.y0)
					{
						top = pixelRightBox.y0;
					}

					if(bottom < pixelRightBox.y1)
					{
						bottom = pixelRightBox.y1;
					}

					if(front > pixelRightBox.z0)
					{
						front = pixelRightBox.z0;
					}

					if(back < pixelRightBox.z1)
					{
						back = pixelRightBox.z1;
					}
				}
			}
		}
	}	

	if((0 == pixelRightBox->x0) || (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) || (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) || (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) || (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) || (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) || (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(NULL != positionedEntity->childrenSpecs)
	{
		int32 i = 0;
		for(; positionedEntity->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->childrenSpecs[i], &pixelGlobalPosition, pixelRightBox);
		}
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		int32 i = 0;
		for(; positionedEntity->entitySpec->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->entitySpec->childrenSpecs[i], &pixelGlobalPosition, pixelRightBox);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
static Entity Entity::instantiate(const PositionedEntity* const positionedEntity, int16 internalId, const char* const name)
{
	NM_ASSERT(NULL != positionedEntity, "Entity::instantiate: null positionedEntity");
	NM_ASSERT(NULL != positionedEntity->entitySpec, "Entity::instantiate: null spec");
	NM_ASSERT(NULL != positionedEntity->entitySpec->allocator, "Entity::instantiate: no allocator defined");

	if(NULL == positionedEntity || NULL == positionedEntity->entitySpec || NULL == positionedEntity->entitySpec->allocator)
	{
		return NULL;
	}

	// Call the appropriate allocator to support inheritance
	Entity entity = ((Entity (*)(EntitySpec*, int16, const char* const)) positionedEntity->entitySpec->allocator)((EntitySpec*)positionedEntity->entitySpec, internalId, name);

	// process extra info
	if(NULL != positionedEntity->extraInfo)
	{
		Entity::setExtraInfo(entity, positionedEntity->extraInfo);
	}

	if(NULL != positionedEntity->entitySpec->extraInfo)
	{
		Entity::setExtraInfo(entity, positionedEntity->entitySpec->extraInfo);
	}

	return entity;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Entity::constructor(EntitySpec* entitySpec, int16 internalId, const char* const name)
{
	// construct base Container
	Base::constructor(name);

	// set the ids
	this->internalId = internalId;

	// save spec
	this->entitySpec = entitySpec;

	// the sprite must be initialized in the derived class
	this->sprites = NULL;
	this->colliders = NULL;
	this->centerDisplacement = NULL;
	this->entityFactory = NULL;
	this->wireframes = NULL;
	this->behaviors = NULL;

	// initialize to 0 for the engine to know that size must be set
	this->size = Size::getFromPixelSize(entitySpec->pixelSize);
	this->collisionsEnabled = true;
	this->checkingCollisions = true;
}
//---------------------------------------------------------------------------------------------------------
void Entity::destructor()
{
	Entity::removeComponents(this);
	Entity::destroyEntityFactory(this);

	if(NULL != this->centerDisplacement)
	{
		delete this->centerDisplacement;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
EntitySpec* Entity::getSpec()
{
	return this->entitySpec;
}
//---------------------------------------------------------------------------------------------------------
int16 Entity::getInternalId()
{
	return this->internalId;
}
//---------------------------------------------------------------------------------------------------------
EntityFactory Entity::getEntityFactory()
{
	return this->entityFactory;
}
//---------------------------------------------------------------------------------------------------------
void Entity::setNormalizedDirection(NormalizedDirection normalizedDirection)
{
	NormalizedDirection currentNormalizedDirection = Entity::getNormalizedDirection(this);

	// if directions XOR is 0, they are equal
	if(
		!(
			(currentNormalizedDirection.x ^ normalizedDirection.x) |
			(currentNormalizedDirection.y ^ normalizedDirection.y) |
			(currentNormalizedDirection.z ^ normalizedDirection.z)
		)
	)
	{
		return;
	}

	Rotation rotation =
	{
		__UP == normalizedDirection.y ? __HALF_ROTATION_DEGREES : __DOWN == normalizedDirection.y ? 0 : this->localTransformation.rotation.x,
		__LEFT == normalizedDirection.x ? __HALF_ROTATION_DEGREES : __RIGHT == normalizedDirection.x ? 0 : this->localTransformation.rotation.y,
		//__NEAR == direction.z ? __HALF_ROTATION_DEGREES : __FAR == direction.z ? 0 : this->localTransformation.rotation.z,
		this->localTransformation.rotation.z,
	};

	Entity::setLocalRotation(this, &rotation);
}
//---------------------------------------------------------------------------------------------------------
NormalizedDirection Entity::getNormalizedDirection()
{
	NormalizedDirection normalizedDirection =
	{
		__RIGHT, __DOWN, __FAR
	};

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.y))
	{
		normalizedDirection.x = __LEFT;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.x))
	{
		normalizedDirection.y = __UP;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.z))
	{
		normalizedDirection.z = __NEAR;
	}

	return normalizedDirection;
}
//---------------------------------------------------------------------------------------------------------
Entity Entity::spawnChildEntity(const PositionedEntity* const positionedEntity)
{
	if(NULL != positionedEntity)
	{
		Entity entity = Entity::createEntity(positionedEntity, this->internalId + Entity::getChildrenCount(this));
		NM_ASSERT(!isDeleted(entity), "Stage::doAddChildEntity: entity not loaded");

		if(!isDeleted(entity))
		{
			// create the entity and add it to the world
			Entity::addChild(this, Container::safeCast(entity));
		}

		return entity;
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addChildEntities(const PositionedEntity* childrenSpecs)
{
	if(NULL == childrenSpecs)
	{
		return;
	}

	int16 internalId = this->internalId + (!isDeleted(this->children) ? VirtualList::getCount(this->children) : 1);

	for(int32 i = 0; NULL != childrenSpecs[i].entitySpec; i++)
	{
		Entity entity = Entity::createEntity(&childrenSpecs[i], internalId++);

		if(!isDeleted(entity))
		{
			Entity::addChild(this, Container::safeCast(entity));
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::addChildEntitiesDeferred(const PositionedEntity* childrenSpecs)
{
	ASSERT(NULL != childrenSpecs, "Entity::addChildEntitiesDeferred: null childrenSpecs");

	if(NULL == childrenSpecs)
	{
		return;
	}

	if(isDeleted(this->entityFactory))
	{
		this->entityFactory = new EntityFactory();

		Entity::addEventListener(this, ListenerObject::safeCast(this), (EventListener)Entity::onEntityLoadedDeferred, kEventEntityLoaded);
	}

	for(int32 i = 0; NULL != childrenSpecs[i].entitySpec; i++)
	{
		EntityFactory::spawnEntity(this->entityFactory, &childrenSpecs[i], Container::safeCast(this), NULL, this->internalId + Entity::getChildrenCount(this));
	}
}
//---------------------------------------------------------------------------------------------------------
Entity Entity::getChildById(int16 id)
{
	if(this->children)
	{
		// look through all children
		for(VirtualNode node = this->children->head; NULL != node ; node = node->next)
		{
			Entity child = Entity::safeCast(node->data);

			if(child->internalId == id)
			{
				return !isDeleted(child) && !child->deleteMe ? child : NULL;
			}
		}
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
Behavior Entity::addBehavior(BehaviorSpec* behaviorSpec)
{
	if(NULL == behaviorSpec)
	{
		return NULL;
	}

	if(NULL == this->behaviors)
	{
		this->behaviors = new VirtualList();
	}

	Behavior behavior = Behavior::create(SpatialObject::safeCast(this), behaviorSpec);

	NM_ASSERT(!isDeleted(behavior), "Entity::addBehavior: behavior not created");

	if(!isDeleted(behavior))
	{
		VirtualList::pushBack(this->behaviors, behavior);
	}

	return behavior;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addBehaviors(BehaviorSpec** behaviorSpecs, bool destroyOldBehaviors)
{
	if(NULL == behaviorSpecs)
	{
		return;
	}

	if(destroyOldBehaviors)
	{
		Entity::removeBehaviors(this);
	}

	if(NULL == this->behaviors)
	{
		this->behaviors = new VirtualList();
	}

	// go through n behaviors in entity's spec
	for(int32 i = 0; NULL != behaviorSpecs[i]; i++)
	{
		Entity::addBehavior(this, behaviorSpecs[i]);
		ASSERT(Behavior::safeCast(VirtualList::back(this->behaviors)), "Entity::addBehaviors: behavior not created");
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeBehaviors()
{
	if(!isDeleted(this->behaviors))
	{
		ASSERT(!isDeleted(this->behaviors), "Entity::removeBehaviors: dead behaviors");

		VirtualList::deleteData(this->behaviors);
		delete this->behaviors;
		this->behaviors = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
bool Entity::getBehaviors(ClassPointer classPointer, VirtualList behaviors)
{
	if(!isDeleted(this->behaviors) && !isDeleted(behaviors))
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(!classPointer || Object::getCast(behavior, classPointer, NULL))
			{
				VirtualList::pushBack(behaviors, behavior);
			}
		}

		if(NULL != behaviors->head)
		{
			return true;
		}
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
Sprite Entity::addSprite(SpriteSpec* spriteSpec, SpriteManager spriteManager)
{
	if(NULL == spriteSpec)
	{
		return NULL;
	}

	if(NULL == spriteManager)
	{
		spriteManager = SpriteManager::getInstance();
	}

	if(NULL == this->sprites)
	{
		this->sprites = new VirtualList();
	}

	Sprite sprite = SpriteManager::createSprite(spriteManager, SpatialObject::safeCast(this), spriteSpec);

	NM_ASSERT(!isDeleted(sprite), "Entity::addSprite: sprite not created");

	if(!isDeleted(sprite))
	{
		VirtualList::pushBack(this->sprites, sprite);
	}

	return sprite;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addSprites(SpriteSpec** spriteSpecs, bool destroyOldSprites)
{
	if(NULL == spriteSpecs || NULL == spriteSpecs[0])
	{
		return;
	}

	if(destroyOldSprites)
	{
		Entity::removeSprites(this);
	}

	SpriteManager spriteManager = SpriteManager::getInstance();

	for(int32 i = 0; NULL != spriteSpecs[i] && NULL != spriteSpecs[i]->allocator; i++)
	{
		Entity::addSprite(this, (SpriteSpec*)spriteSpecs[i], spriteManager);
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeSprite(Sprite sprite)
{
	if(isDeleted(this->sprites) || !VirtualList::removeData(this->sprites, sprite))
	{
		return;
	}

	SpriteManager::destroySprite(SpriteManager::getInstance(), sprite);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeSprites()
{
#ifndef __SHIPPING
#ifndef __RELEASE
	int32 lp = HardwareManager::getLinkPointer();
#endif
#endif

	if(!isDeleted(this->sprites))
	{
		SpriteManager spriteManager = SpriteManager::getInstance();
		
		// Must use a temporal list to prevent any race condition
		VirtualList sprites = this->sprites;
		this->sprites = NULL;

		for(VirtualNode node = sprites->head; NULL != node ; node = node->next)
		{

#ifndef __SHIPPING
			if(isDeleted(Sprite::safeCast(node->data)))
			{
				Printing::setDebugMode(Printing::getInstance());
				Printing::clear(Printing::getInstance());
				Printing::text(Printing::getInstance(), "Onwer type: ", 1, 26, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 12, 26, NULL);
#ifndef __RELEASE
				Printing::text(Printing::getInstance(), "Caller address: ", 1, 27, NULL);
				Printing::hex(Printing::getInstance(), lp, 1, 12, 8, NULL);
#endif
				Error::triggerException("Entity::removeSprites: trying to dispose dead sprite", NULL);		
			}
#endif
			NM_ASSERT(!isDeleted(Sprite::safeCast(node->data)), "Entity::removeSprites: trying to dispose dead sprite");

			SpriteManager::destroySprite(spriteManager, Sprite::safeCast(node->data));
		}

		// delete the sprites
		delete sprites;
	}

	this->sprites = NULL;
}
//---------------------------------------------------------------------------------------------------------
VirtualList Entity::getSprites()
{
	return this->sprites;
}
//---------------------------------------------------------------------------------------------------------
Wireframe Entity::addWireframe(WireframeSpec* wireframeSpec, WireframeManager wireframeManager)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	if(NULL == wireframeManager)
	{
		wireframeManager = WireframeManager::getInstance();
	}

	if(NULL == this->wireframes)
	{
		this->wireframes = new VirtualList();
	}

	Wireframe wireframe = WireframeManager::createWireframe(wireframeManager, wireframeSpec, SpatialObject::safeCast(this));

	NM_ASSERT(!isDeleted(wireframe), "Entity::addWireframe: wireframe not created");

	if(!isDeleted(wireframe))
	{
		VirtualList::pushBack(this->wireframes, wireframe);
	}

	return wireframe;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addWireframes(WireframeSpec** wireframeSpecs, bool destroyOldWireframes)
{
	if(NULL == wireframeSpecs || NULL == wireframeSpecs[0])
	{
		return;
	}

	if(destroyOldWireframes)
	{
		Entity::removeWireframes(this);
	}

	WireframeManager wireframeManager = WireframeManager::getInstance();

	for(int32 i = 0; NULL != wireframeSpecs[i] && NULL != wireframeSpecs[i]->allocator; i++)
	{
		Entity::addWireframe(this, wireframeSpecs[i], wireframeManager);
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeWireframe(Wireframe wireframe)
{
	if(isDeleted(this->wireframes) || !VirtualList::removeData(this->wireframes, wireframe))
	{
		return;
	}

	WireframeManager::destroyWireframe(WireframeManager::getInstance(), wireframe);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeWireframes()
{
	if(!isDeleted(this->wireframes))
	{
		ASSERT(!isDeleted(this->wireframes), "Entity::wireframes: dead colliders");

		WireframeManager wireframeManager = WireframeManager::getInstance();

		for(VirtualNode node = this->wireframes->head; NULL != node; node = node->next)
		{
			WireframeManager::destroyWireframe(wireframeManager, Wireframe::safeCast(node->data));
		}

		delete this->wireframes;
		this->wireframes = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
VirtualList Entity::getWireframes()
{
	return this->wireframes;
}
//---------------------------------------------------------------------------------------------------------
Collider Entity::addCollider(ColliderSpec* colliderSpec, CollisionManager collisionManager)
{
	if(NULL == colliderSpec)
	{
		return NULL;
	}

	if(NULL == collisionManager)
	{
		collisionManager = VUEngine::getCollisionManager(_vuEngine);
	}

	if(NULL == this->colliders)
	{
		this->colliders = new VirtualList();
	}

	Collider collider = CollisionManager::createCollider(collisionManager, SpatialObject::safeCast(this), colliderSpec);

	NM_ASSERT(!isDeleted(collider), "Entity::addCollider: collider not created");

	if(!isDeleted(collider))
	{
		VirtualList::pushBack(this->colliders, collider);
	}

	return collider;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addColliders(ColliderSpec* colliderSpecs, bool destroyOldColliders)
{
	if(NULL == colliderSpecs)
	{
		return;
	}

	if(destroyOldColliders)
	{
		Entity::removeColliders(this);
	}

	CollisionManager collisionManager = VUEngine::getCollisionManager(_vuEngine);

	// go through n sprites in entity's spec
	for(int32 i = 0; NULL != colliderSpecs[i].allocator; i++)
	{
		Entity::addCollider(this, &colliderSpecs[i], collisionManager);
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeCollider(Collider collider)
{
	if(isDeleted(this->colliders) || !VirtualList::removeData(this->colliders, collider))
	{
		return;
	}

	CollisionManager::destroyCollider(VUEngine::getCollisionManager(_vuEngine), collider);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeColliders()
{
	if(NULL != this->colliders)
	{
		ASSERT(!isDeleted(this->colliders), "Entity::setSpec: dead colliders");

		CollisionManager collisionManager = VUEngine::getCollisionManager(_vuEngine);

		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			CollisionManager::destroyCollider(collisionManager, Collider::safeCast(node->data));
		}

		delete this->colliders;
		this->colliders = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
VirtualList Entity::getColliders()
{
	return this->colliders;
}
//---------------------------------------------------------------------------------------------------------
void Entity::enableCollisions()
{
	this->collisionsEnabled = true;

	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::enable(collider);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::disableCollisions()
{
	this->collisionsEnabled = false;

	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::disable(collider);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::checkCollisions(bool active)
{
	this->checkingCollisions = active;

	if(this->checkingCollisions && !this->collisionsEnabled)
	{
		this->collisionsEnabled = this->checkingCollisions;
	}

	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::checkCollisions(collider, active);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::registerCollisions(bool value)
{
	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::registerCollisions(collider, value);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::setCollidersLayers(uint32 layers)
{
	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::setLayers(collider, layers);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
uint32 Entity::getCollidersLayers()
{
	uint32 collidersLayers = 0;

	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			collidersLayers |= Collider::getLayers(collider);
		}
	}

	return collidersLayers;
}
//---------------------------------------------------------------------------------------------------------
void Entity::setCollidersLayersToIgnore(uint32 layersToIgnore)
{
	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			Collider::setLayersToIgnore(collider, layersToIgnore);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
uint32 Entity::getCollidersLayersToIgnore()
{
	uint32 collidersLayersToIgnore = 0;

	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			collidersLayersToIgnore |= Collider::getLayersToIgnore(collider);
		}
	}

	return collidersLayersToIgnore;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::hasColliders()
{
	return NULL != this->colliders && 0 < VirtualList::getCount(this->colliders);
}
//---------------------------------------------------------------------------------------------------------
void Entity::showColliders()
{
	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider::show(node->data);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::hideColliders()
{
	if(NULL != this->colliders)
	{
		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider::hide(node->data);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::calculateSize(bool force)
{
	if(!force)
	{
		if(0 != this->size.x || 0 != this->size.y || 0 != this->size.z)
		{
			return;
		}
	}

	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::calculateSizeFromChildren(this, &pixelRightBox, Vector3D::zero());

	Vector3D centerDisplacement =
	{
		__PIXELS_TO_METERS((pixelRightBox.x1 + pixelRightBox.x0) >> 1) - this->localTransformation.position.x,
		__PIXELS_TO_METERS((pixelRightBox.y1 + pixelRightBox.y0) >> 1) - this->localTransformation.position.y,
		__PIXELS_TO_METERS((pixelRightBox.z1 + pixelRightBox.z0) >> 1) - this->localTransformation.position.z
	};

	if(0 != (centerDisplacement.x | centerDisplacement.y | centerDisplacement.z))
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
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getWidth()
{
	if(0 == this->size.x)
	{
		Entity::calculateSize(this, false);
	}

	return this->size.x;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getHeight()
{
	if(0 == this->size.y)
	{
		Entity::calculateSize(this, false);
	}

	return this->size.y;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getDepth()
{
	if(0 == this->size.z)
	{
		Entity::calculateSize(this, false);
	}

	return this->size.z;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::isInCameraRange(int16 padding, bool recursive)
{
	if(!this->hidden && NULL != this->sprites && NULL != this->sprites->head)
	{
		for(VirtualNode spriteNode = this->sprites->head; NULL != spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = Sprite::safeCast(spriteNode->data);

			if(Sprite::isVisible(sprite))
			{
				return true;
			}
		}
	}

	// when DirectDraw draws at least a pixel
	if(!this->hidden && NULL != this->wireframes && NULL != this->wireframes->head)
	{
		for(VirtualNode wireframeNode = this->wireframes->head; NULL != wireframeNode; wireframeNode = wireframeNode->next)
		{
			Wireframe wireframe = Wireframe::safeCast(wireframeNode->data);

			if(Wireframe::isVisible(wireframe))
			{
				return true;
			}
		}
	}

	Vector3D position3D = this->transformation.position;

	if(NULL != this->centerDisplacement)
	{
		position3D = Vector3D::sum(position3D, *this->centerDisplacement);
	}

	bool inCameraRange = true;

	PixelRightBox pixelRightBox	=
	{
		- __METERS_TO_PIXELS(this->size.x >> 1) - padding,
		- __METERS_TO_PIXELS(this->size.y >> 1) - padding,
		- __METERS_TO_PIXELS(this->size.z >> 1) - padding,

		__METERS_TO_PIXELS(this->size.x >> 1) + padding,
		__METERS_TO_PIXELS(this->size.y >> 1) + padding,
		__METERS_TO_PIXELS(this->size.z >> 1) + padding,
	};

	if(!PixelVector::isVector3DVisible(position3D, pixelRightBox, 0))
	{
		inCameraRange = false;
	}

	if(!inCameraRange && recursive && NULL != this->children)
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Entity child = Entity::safeCast(VirtualNode::getData(childNode));

			if(child->hidden)
			{
				continue;
			}

			if(Entity::isInCameraRange(child, padding, true))
			{
				return true;
			}
		}
	}

	return inCameraRange;
}
//---------------------------------------------------------------------------------------------------------
void Entity::setSpec(void* entitySpec)
{
	// save spec
	this->entitySpec = entitySpec;
}
//---------------------------------------------------------------------------------------------------------
void Entity::setExtraInfo(void* extraInfo __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
bool Entity::alwaysStreamIn()
{
	return true;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getRadius()
{
	fixed_t width = Entity::getWidth(this);
	fixed_t height = Entity::getHeight(this);
	fixed_t depth = Entity::getDepth(this);

	if(width > height)
	{
		if(width > depth)
		{
			return width >> 1;
		}
		else
		{
			return depth >> 1;
		}

	}
	else if(height > depth)
	{
		return height >> 1;
	}
	else
	{
		return depth >> 1;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getBounciness()
{
	return this->entitySpec->physicalProperties ? this->entitySpec->physicalProperties->bounciness : 0;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getFrictionCoefficient()
{
	return this->entitySpec->physicalProperties ? this->entitySpec->physicalProperties->frictionCoefficient : 0;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return true;
}
//---------------------------------------------------------------------------------------------------------
uint32 Entity::getInGameType()
{
	return this->entitySpec->inGameType;
}
//---------------------------------------------------------------------------------------------------------
void Entity::addComponents()
{
	if(!isDeleted(this->children))
	{
		Base::addComponents(this);
	}

	Entity::createSprites(this);
	Entity::createWireframes(this);
	Entity::createColliders(this);
	Entity::createBehaviors(this);

	Entity::calculateSize(this, false);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeComponents()
{
	Entity::removeWireframes(this);
	Entity::removeSprites(this);
	Entity::removeColliders(this);
	Entity::removeBehaviors(this);
}
//---------------------------------------------------------------------------------------------------------
void Entity::show()
{
	Base::show(this);

	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::show(node->data);
		}
	}

	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::show(node->data);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::hide()
{
	Base::hide(this);

	// hide all sprites
	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::hide(node->data);
		}
	}

	// hide all wireframes
	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::hide(node->data);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::suspend()
{
	Base::suspend(this);

	Entity::removeSprites(this);
	Entity::removeWireframes(this);
}
//---------------------------------------------------------------------------------------------------------
void Entity::resume()
{
	Base::resume(this);

	// initialize sprites
	if(NULL != this->entitySpec)
	{
		Entity::createSprites(this);
		Entity::createWireframes(this);
	}

	if(this->hidden)
	{
		// Force syncronization even if hidden
		this->hidden = false;
		Entity::hide(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::setTransparency(uint8 transparency)
{
	Base::setTransparency(this, transparency);

	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::setTransparency(node->data, transparency);
		}
	}

	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::setTransparency(node->data, transparency);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
bool Entity::handlePropagatedMessage(int32 message)
{
	switch(message)
	{
		case kMessageReleaseVisualComponents:

			Entity::removeSprites(this);
			Entity::removeWireframes(this);
			break;

		case kMessageReloadVisualComponents:

			if(NULL != this->entitySpec)
			{
				Entity::createSprites(this);
				Entity::createWireframes(this);
			}
			break;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
bool Entity::createBehaviors()
{
	if(NULL == this->behaviors)
	{
		Entity::addBehaviors(this, this->entitySpec->behaviorSpecs, true);
	}

	return NULL != this->behaviors;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::createSprites()
{
	// this method can be called multiple times so only add colliders
	// if not already done
	if(NULL == this->sprites)
	{
		Entity::addSprites(this, this->entitySpec->spriteSpecs, true);
	}

	return NULL != this->sprites;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::createWireframes()
{
	// this method can be called multiple times so only add colliders
	// if not already done
	if(NULL == this->wireframes)
	{
		Entity::addWireframes(this, this->entitySpec->wireframeSpecs, true);
	}

	return NULL != this->wireframes;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::createColliders()
{
	// this method can be called multiple times so only add colliders
	// if not already done
	if(NULL == this->colliders)
	{
		Entity::addColliders(this, this->entitySpec->colliderSpecs, true);
	}

	return NULL != this->colliders;
}
//---------------------------------------------------------------------------------------------------------
void Entity::calculateSizeFromChildren(PixelRightBox* pixelRightBox, Vector3D environmentPosition)
{
	PixelVector pixelGlobalPosition = PixelVector::getFromVector3D(environmentPosition, 0);

	pixelGlobalPosition.x += __METERS_TO_PIXELS(this->localTransformation.position.x);
	pixelGlobalPosition.y += __METERS_TO_PIXELS(this->localTransformation.position.y);
	pixelGlobalPosition.z += __METERS_TO_PIXELS(this->localTransformation.position.z);

	int16 left = 0;
	int16 right = 0;
	int16 top = 0;
	int16 bottom = 0;
	int16 front = 0;
	int16 back = 0;
	int16 halfWidth = 0;
	int16 halfHeight = 0;
	int16 halfDepth = ENTITY_SPRITE_HALF_DEPTH;

	if((0 == this->size.x || 0 == this->size.y || 0 == this->size.z) && (NULL != this->sprites || NULL != this->wireframes))
	{
		if(NULL != this->sprites)
		{
			for(VirtualNode spriteNode = this->sprites->head; spriteNode; spriteNode = spriteNode->next)
			{
				Sprite sprite = Sprite::safeCast(spriteNode->data);
				ASSERT(sprite, "Entity::calculateSizeFromChildren: null sprite");

				halfWidth = Sprite::getHalfWidth(sprite);
				halfHeight = Sprite::getHalfHeight(sprite);
				halfDepth = ENTITY_SPRITE_HALF_DEPTH;

				PixelVector spriteDisplacement = *Sprite::getDisplacement(sprite);

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

		if(NULL != this->wireframes)
		{
			for(VirtualNode wireframeNode = this->wireframes->head; wireframeNode; wireframeNode = wireframeNode->next)
			{
				Wireframe wireframe = Wireframe::safeCast(wireframeNode->data);
				ASSERT(wireframe, "Entity::calculateSizeFromChildren: null wireframe");

				PixelRightBox pixelRightBox = Wireframe::getPixelRightBox(wireframe);

				if(left > pixelRightBox.x0)
				{
					left = pixelRightBox.x0;
				}

				if(right < pixelRightBox.x1)
				{
					right = pixelRightBox.x1;
				}

				if(top > pixelRightBox.y0)
				{
					top = pixelRightBox.y0;
				}

				if(bottom < pixelRightBox.y1)
				{
					bottom = pixelRightBox.y1;
				}

				if(front > pixelRightBox.z0)
				{
					front = pixelRightBox.z0;
				}

				if(back < pixelRightBox.z1)
				{
					back = pixelRightBox.z1;
				}
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

	if((0 == pixelRightBox->x0) || (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) || (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) || (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) || (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) || (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) || (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(!isDeleted(this->children))
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Entity::calculateSizeFromChildren(childNode->data, pixelRightBox, Vector3D::getFromPixelVector(pixelGlobalPosition));
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::destroyEntityFactory()
{
	if(!isDeleted(this->entityFactory))
	{
		delete this->entityFactory;
		this->entityFactory = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
bool Entity::onEntityLoadedDeferred(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(ListenerObject::safeCast(this) != eventFirer)
	{
		return false;
	} 

	if(isDeleted(this->entityFactory))
	{
		return false;
	}

	if(!EntityFactory::hasEntitiesPending(this->entityFactory))
	{
		Entity::destroyEntityFactory(this);
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
