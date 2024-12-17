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

#define ENTITY_MIN_SIZE							16
#define ENTITY_HALF_MIN_SIZE					(ENTITY_MIN_SIZE >> 1)


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
static RightBox Entity::getBoundingBoxFromSpec(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition)
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	Entity::getSizeFromSpec(positionedEntity, environmentPosition, &rightBox);

	return rightBox;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void Entity::getSizeFromSpec(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition, RightBox* rightBox)
{
	ASSERT(positionedEntity, "Entity::getSizeFromSpec: null positionedEntity");
	ASSERT(positionedEntity->entitySpec, "Entity::getSizeFromSpec: null entitySpec");

	fixed_t left = 0;
	fixed_t right = 0;
	fixed_t top = 0;
	fixed_t bottom = 0;
	fixed_t front = 0;
	fixed_t back = 0;
	fixed_t halfWidth = 0;
	fixed_t halfHeight = 0;
	fixed_t halfDepth = 0;

	if(0 != positionedEntity->entitySpec->pixelSize.x || 0 != positionedEntity->entitySpec->pixelSize.y || 0 != positionedEntity->entitySpec->pixelSize.z)
	{
		// TODO: there should be a class which handles special cases
		halfWidth = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.x) >> 1;
		halfHeight = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.y) >> 1;
		halfDepth = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.z) >> 1;

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

					halfWidth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
					halfHeight = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
					halfDepth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);

					for(; mBgmapSpriteSpec->textureSpecs[j]; j++)
					{
						if(halfWidth < __PIXELS_TO_METERS(mBgmapSpriteSpec->textureSpecs[j]->cols << 2))
						{
							halfWidth = __PIXELS_TO_METERS(mBgmapSpriteSpec->textureSpecs[j]->cols << 2);
						}

						if(halfHeight < __PIXELS_TO_METERS(mBgmapSpriteSpec->textureSpecs[j]->rows << 2))
						{
							halfHeight = __PIXELS_TO_METERS(mBgmapSpriteSpec->textureSpecs[j]->rows << 2);
						}
					}

					if(left > -halfWidth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x))
					{
						left = -halfWidth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x);
					}

					if(right < halfWidth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x))
					{
						right = halfWidth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x);
					}

					if(top > -halfHeight + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y))
					{
						top = -halfHeight + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y);
					}

					if(bottom < halfHeight + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y))
					{
						bottom = halfHeight + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y);
					}

					if(front > __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z))
					{
						front = __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z);
					}

					if(back < halfDepth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z))
					{
						back = halfDepth + __PIXELS_TO_METERS(mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z);
					}

				}
				else if(NULL != positionedEntity->entitySpec->spriteSpecs[i]->textureSpec)
				{
					SpriteSpec* spriteSpec = (SpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i];
					halfWidth = __PIXELS_TO_METERS(spriteSpec->textureSpec->cols << 2);
					halfHeight = __PIXELS_TO_METERS(spriteSpec->textureSpec->rows << 2);
					halfDepth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);

					if(left > -halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x))
					{
						left = -halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x);
					}

					if(right < halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x))
					{
						right = halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x);
					}

					if(top > -halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y))
					{
						top = -halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y);
					}

					if(bottom < halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y))
					{
						bottom = halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y);
					}

					if(front > -halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z))
					{
						front = -halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z);
					}

					if(back < halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z))
					{
						back = halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z);
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
					RightBox rightBox = Mesh::getRightBoxFromSpec((MeshSpec*)positionedEntity->entitySpec->wireframeSpecs[i]);

					if(left > rightBox.x0)
					{
						left = rightBox.x0;
					}

					if(right < rightBox.x1)
					{
						right = rightBox.x1;
					}

					if(top > rightBox.y0)
					{
						top = rightBox.y0;
					}

					if(bottom < rightBox.y1)
					{
						bottom = rightBox.y1;
					}

					if(front > rightBox.z0)
					{
						front = rightBox.z0;
					}

					if(back < rightBox.z1)
					{
						back = rightBox.z1;
					}
				}
			}
		}
	}	

	Vector3D globalPosition = *environmentPosition;

	if((0 == rightBox->x0) || (globalPosition.x + left < rightBox->x0))
	{
		rightBox->x0 = globalPosition.x + left;
	}

	if((0 == rightBox->x1) || (right + globalPosition.x > rightBox->x1))
	{
		rightBox->x1 = right + globalPosition.x;
	}

	if((0 == rightBox->y0) || (globalPosition.y + top < rightBox->y0))
	{
		rightBox->y0 = globalPosition.y + top;
	}

	if((0 == rightBox->y1) || (bottom + globalPosition.y > rightBox->y1))
	{
		rightBox->y1 = bottom + globalPosition.y;
	}

	if((0 == rightBox->z0) || (globalPosition.z + front < rightBox->z0))
	{
		rightBox->z0 = globalPosition.z + front;
	}

	if((0 == rightBox->z1) || (back + globalPosition.z > rightBox->z1))
	{
		rightBox->z1 = back + globalPosition.z;
	}

	globalPosition.x += __PIXELS_TO_METERS(positionedEntity->onScreenPosition.x);
	globalPosition.y += __PIXELS_TO_METERS(positionedEntity->onScreenPosition.y);
	globalPosition.z += __PIXELS_TO_METERS(positionedEntity->onScreenPosition.z);

	if(NULL != positionedEntity->childrenSpecs)
	{
		for(int32 i = 0; positionedEntity->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->childrenSpecs[i], &globalPosition, rightBox);
		}
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		for(int32 i = 0; positionedEntity->entitySpec->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->entitySpec->childrenSpecs[i], &globalPosition, rightBox);
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
	Base::constructor(name);

	// set the ids
	this->internalId = internalId;

	// save spec
	this->entitySpec = entitySpec;

	// the sprite must be initialized in the derived class
	this->colliders = NULL;
	this->centerDisplacement = NULL;
	this->entityFactory = NULL;
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

	Entity::calculateSize(this);
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

	VisualComponent::propagateCommand(cVisualComponentCommandShow, SpatialObject::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void Entity::hide()
{
	Base::hide(this);

	VisualComponent::propagateCommand(cVisualComponentCommandHide, SpatialObject::safeCast(this));
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

	VisualComponent::propagateCommand(cVisualComponentCommandSetTransparency, SpatialObject::safeCast(this), (uint32)transparency);
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

	return SpriteManager::createSprite(spriteManager, SpatialObject::safeCast(this), spriteSpec);
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
	SpriteManager::destroySprite(SpriteManager::getInstance(), sprite);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeSprites()
{
	SpriteManager::destroySprites(SpriteManager::getInstance(), SpatialObject::safeCast(this));
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

	return WireframeManager::createWireframe(wireframeManager, wireframeSpec, SpatialObject::safeCast(this));
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
	WireframeManager::destroyWireframe(WireframeManager::getInstance(), wireframe);
}
//---------------------------------------------------------------------------------------------------------
void Entity::removeWireframes()
{
	WireframeManager::destroyWireframes(WireframeManager::getInstance(), SpatialObject::safeCast(this));
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
void Entity::calculateSize()
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	Entity::calculateSizeFromChildren(this, &rightBox, Vector3D::zero());

	Vector3D centerDisplacement =
	{
		((rightBox.x1 + rightBox.x0) >> 1) - this->localTransformation.position.x,
		((rightBox.y1 + rightBox.y0) >> 1) - this->localTransformation.position.y,
		((rightBox.z1 + rightBox.z0) >> 1) - this->localTransformation.position.z
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

	this->size.x = rightBox.x1 - rightBox.x0;
	this->size.y = rightBox.y1 - rightBox.y0;
	this->size.z = rightBox.z1 - rightBox.z0;

	NM_ASSERT(0 < this->size.x, "Entity::calculateSize: 0 x size");
	NM_ASSERT(0 < this->size.y, "Entity::calculateSize: 0 y size");
	NM_ASSERT(0 < this->size.z, "Entity::calculateSize: 0 z size");
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getWidth()
{
	if(0 == this->size.x)
	{
		Entity::calculateSize(this);
	}

	return this->size.x;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getHeight()
{
	if(0 == this->size.y)
	{
		Entity::calculateSize(this);
	}

	return this->size.y;
}
//---------------------------------------------------------------------------------------------------------
fixed_t Entity::getDepth()
{
	if(0 == this->size.z)
	{
		Entity::calculateSize(this);
	}

	return this->size.z;
}
//---------------------------------------------------------------------------------------------------------
bool Entity::isInCameraRange(int16 padding, bool recursive)
{
	if(VisualComponent::isAnyVisible(SpatialObject::safeCast(this)))
	{
		return true;
	}

	Vector3D position3D = this->transformation.position;
	Vector3D centerDisplacement = Vector3D::zero();

	if(NULL != this->centerDisplacement)
	{
		centerDisplacement = *this->centerDisplacement;
	}

	fixed_t paddingHelper = __PIXELS_TO_METERS(padding);

	RightBox rightBox	=
	{
		// The center of displacement has to be added to the bounding box and not to the 
		// position because this has to be rotated
		-(this->size.x >> 1) - paddingHelper + centerDisplacement.x,
		-(this->size.y >> 1) - paddingHelper + centerDisplacement.y,
		-(this->size.z >> 1) - paddingHelper + centerDisplacement.z,

		(this->size.x >> 1) + paddingHelper + centerDisplacement.x,
		(this->size.y >> 1) + paddingHelper + centerDisplacement.y,
		(this->size.z >> 1) + paddingHelper + centerDisplacement.z
	};

	bool inCameraRange = Entity::isInsideFrustrum(position3D, rightBox);

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
void Entity::createSprites()
{
	// This method can be called multiple times so only add sprites if not already added
	if(0 == SpriteManager::getCount(SpriteManager::getInstance(), SpatialObject::safeCast(this)))
	{
		Entity::addSprites(this, this->entitySpec->spriteSpecs, true);
	}
}
//---------------------------------------------------------------------------------------------------------
void Entity::createWireframes()
{
	// This method can be called multiple times so only add wireframes if not already added
	if(0 == WireframeManager::getCount(WireframeManager::getInstance(), SpatialObject::safeCast(this)))
	{
		Entity::addWireframes(this, this->entitySpec->wireframeSpecs, true);
	}
}
//---------------------------------------------------------------------------------------------------------
bool Entity::createColliders()
{
	// This method can be called multiple times so only add colliders if not already added
	if(NULL == this->colliders)
	{
		Entity::addColliders(this, this->entitySpec->colliderSpecs, true);
	}

	return NULL != this->colliders;
}
//---------------------------------------------------------------------------------------------------------
void Entity::calculateSizeFromChildren(RightBox* rightBox, Vector3D environmentPosition)
{
	Vector3D globalPosition = Vector3D::sum(environmentPosition, this->localTransformation.position);

	RightBox myRightBox = {0, 0, 0, 0, 0, 0};

	if(0 == this->size.x || 0 == this->size.y || 0 == this->size.z)
	{
		bool rightBoxComputed = VisualComponent::calculateRightBox(SpatialObject::safeCast(this), &myRightBox);

		if(!rightBoxComputed)
		{
			myRightBox.x1 = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
			myRightBox.x0 = -__PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
			myRightBox.y1 = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
			myRightBox.y0 = -__PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
			myRightBox.z1 = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
			myRightBox.z0 = -__PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
		}
	}
	else
	{
		myRightBox.x1 = (this->size.x >> 1);
		myRightBox.x0 = -myRightBox.x1;
		myRightBox.y1 = (this->size.y >> 1);
		myRightBox.y0 = -myRightBox.y1;
		myRightBox.z1 = (this->size.z >> 1);
		myRightBox.z0 = -myRightBox.z1;
	}

	if((0 == rightBox->x0) || (globalPosition.x + myRightBox.x0 < rightBox->x0))
	{
		rightBox->x0 = globalPosition.x + myRightBox.x0;
	}

	if((0 == rightBox->x1) || (myRightBox.x1 + globalPosition.x > rightBox->x1))
	{
		rightBox->x1 = myRightBox.x1 + globalPosition.x;
	}

	if((0 == rightBox->y0) || (globalPosition.y + myRightBox.y0 < rightBox->y0))
	{
		rightBox->y0 = globalPosition.y + myRightBox.y0;
	}

	if((0 == rightBox->y1) || (myRightBox.y1 + globalPosition.y > rightBox->y1))
	{
		rightBox->y1 = myRightBox.y1 + globalPosition.y;
	}

	if((0 == rightBox->z0) || (globalPosition.z + myRightBox.z0 < rightBox->z0))
	{
		rightBox->z0 = globalPosition.z + myRightBox.z0;
	}

	if((0 == rightBox->z1) || (myRightBox.z1 + globalPosition.z > rightBox->z1))
	{
		rightBox->z1 = myRightBox.z1 + globalPosition.z;
	}

	if(!isDeleted(this->children))
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Entity::calculateSizeFromChildren(childNode->data, rightBox, globalPosition);
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
