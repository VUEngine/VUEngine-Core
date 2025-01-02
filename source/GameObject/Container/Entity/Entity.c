/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <DebugConfig.h>
#include <EntityFactory.h>
#include <Optics.h>
#include <Mesh.h>
#include <Printing.h>
#include <Sprite.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Entity.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#define ENTITY_MIN_SIZE							16
#define ENTITY_HALF_MIN_SIZE					(ENTITY_MIN_SIZE >> 1)


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

static RightBox Entity::getRightBoxFromSpec(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition)
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	Entity::getRightBoxFromChildrenSpec(positionedEntity, environmentPosition, &rightBox);

	Vector3D globalPosition = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);

	rightBox.x0 -= globalPosition.x;
	rightBox.x1 -= globalPosition.x;
	rightBox.y0 -= globalPosition.y;
	rightBox.y1 -= globalPosition.y;
	rightBox.z0 -= globalPosition.z;
	rightBox.z1 -= globalPosition.z;

	return rightBox;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Entity::getRightBoxFromChildrenSpec(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition, RightBox* rightBox)
{
	ASSERT(positionedEntity, "Entity::getRightBoxFromChildrenSpec: null positionedEntity");
	ASSERT(positionedEntity->entitySpec, "Entity::getRightBoxFromChildrenSpec: null entitySpec");

	RightBox myRightBox = {0, 0, 0, 0, 0, 0};

	if(0 != positionedEntity->entitySpec->pixelSize.x || 0 != positionedEntity->entitySpec->pixelSize.y || 0 != positionedEntity->entitySpec->pixelSize.z)
	{
		fixed_t halfWidth = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.x) >> 1;
		fixed_t halfHeight = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.y) >> 1;
		fixed_t halfDepth = __PIXELS_TO_METERS(positionedEntity->entitySpec->pixelSize.z) >> 1;

		myRightBox.x0 = -halfWidth;
		myRightBox.x1 = halfWidth;
		myRightBox.y0 = -halfHeight;
		myRightBox.y1 = halfHeight;
		myRightBox.z0 = -halfDepth;
		myRightBox.z1 = halfDepth;
	}
	else 
	{
		if(NULL != positionedEntity->entitySpec->componentSpecs && NULL != positionedEntity->entitySpec->componentSpecs[0])
		{
			for(int16 i = 0; NULL != positionedEntity->entitySpec->componentSpecs[i]; i++)
			{
				RightBox helperRightBox = {0, 0, 0, 0, 0, 0};

				switch (positionedEntity->entitySpec->componentSpecs[i]->componentType)
				{
					case kSpriteComponent:
					{
						SpriteSpec* spriteSpec = (SpriteSpec*)positionedEntity->entitySpec->componentSpecs[i];

						fixed_t halfWidth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
						fixed_t halfHeight = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
						fixed_t halfDepth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);

						if(NULL != spriteSpec->textureSpec)
						{
							halfWidth = __PIXELS_TO_METERS(spriteSpec->textureSpec->cols << 2);
							halfHeight = __PIXELS_TO_METERS(spriteSpec->textureSpec->rows << 2);
							halfDepth = __PIXELS_TO_METERS(ENTITY_HALF_MIN_SIZE);
						}
						
						helperRightBox = (RightBox)
						{
							-halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x),
							-halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y),
							-halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z),
							halfWidth + __PIXELS_TO_METERS(spriteSpec->displacement.x),
							halfHeight + __PIXELS_TO_METERS(spriteSpec->displacement.y),
							halfDepth + __PIXELS_TO_METERS(spriteSpec->displacement.z),
						};

						break;
					}

					case kWireframeComponent:
					{
						helperRightBox = Mesh::getRightBoxFromSpec((MeshSpec*)positionedEntity->entitySpec->componentSpecs[i]);
						break;
					}

					default:
					{
						continue;
					}
				}

				if(myRightBox.x0 > helperRightBox.x0)
				{
					myRightBox.x0 = helperRightBox.x0;
				}

				if(myRightBox.x0 > helperRightBox.x0)
				{
					myRightBox.x0 = helperRightBox.x0;
				}

				if(myRightBox.x1 < helperRightBox.x1)
				{
					myRightBox.x1 = helperRightBox.x1;
				}

				if(myRightBox.y0 > helperRightBox.y0)
				{
					myRightBox.y0 = helperRightBox.y0;
				}

				if(myRightBox.y1 < helperRightBox.y1)
				{
					myRightBox.y1 = helperRightBox.y1;
				}

				if(myRightBox.z0 > helperRightBox.z0)
				{
					myRightBox.z0 = helperRightBox.z0;
				}

				if(myRightBox.z1 < helperRightBox.z1)
				{
					myRightBox.z1 = helperRightBox.z1;
				}
			}
		}
	}	

	Vector3D globalPosition = Vector3D::sum(*environmentPosition, Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition));

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

	if(NULL != positionedEntity->childrenSpecs)
	{
		for(int32 i = 0; positionedEntity->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getRightBoxFromChildrenSpec(&positionedEntity->childrenSpecs[i], &globalPosition, rightBox);
		}
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		for(int32 i = 0; positionedEntity->entitySpec->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getRightBoxFromChildrenSpec(&positionedEntity->entitySpec->childrenSpecs[i], &globalPosition, rightBox);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::constructor(EntitySpec* entitySpec, int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor(internalId, name);

	this->entitySpec = entitySpec;

	this->centerDisplacement = NULL;
	this->entityFactory = NULL;

	this->size = Size::getFromPixelSize(entitySpec->pixelSize);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destructor()
{
	Entity::destroyComponents(this);
	Entity::destroyEntityFactory(this);

	if(NULL != this->centerDisplacement)
	{
		delete this->centerDisplacement;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::createComponents(ComponentSpec** componentSpecs)
{
	Base::createComponents(this, NULL != componentSpecs ? componentSpecs : this->entitySpec->componentSpecs);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destroyComponents()
{
	Base::destroyComponents(this);

	this->size = (Size){0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Entity::getInGameType()
{
	return this->entitySpec->inGameType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::suspend()
{
	Base::suspend(this);

	Entity::removeComponents(this, kSpriteComponent);
	Entity::removeComponents(this, kWireframeComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::resume()
{
	if(NULL != this->entitySpec)
	{
		Entity::addComponents(this, this->entitySpec->componentSpecs, kSpriteComponent);
		Entity::addComponents(this, this->entitySpec->componentSpecs, kWireframeComponent);
	}

	Base::resume(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::handleCommand(int32 command, va_list args)
{
	switch (command)
	{
		case kMessageShow:
		{
			Entity::show(this);			
			break;
		}

		case kMessageHide:
		{
			Entity::hide(this);			
			break;
		}

		case kMessageSetTransparency:
		{
			Entity::setTransparency(this, (uint8)va_arg(args, uint32));			
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

EntitySpec* Entity::getSpec()
{
	return this->entitySpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

EntityFactory Entity::getEntityFactory()
{
	return this->entityFactory;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getWidth()
{
	if(0 == this->size.x)
	{
		NM_ASSERT(false, "Entity::getWidth: 0 x size");
		Entity::calculateSize(this);
	}

	return this->size.x;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getHeight()
{
	if(0 == this->size.y)
	{
		NM_ASSERT(false, "Entity::getHeight: 0 y size");
		Entity::calculateSize(this);
	}

	return this->size.y;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getDepth()
{
	if(0 == this->size.z)
	{
		NM_ASSERT(false, "Entity::getDepth: 0 z size");
		Entity::calculateSize(this);
	}

	return this->size.z;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::isInCameraRange(int16 padding, bool recursive)
{
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

	if(Entity::isInsideFrustrum(position3D, rightBox))
	{
		return true;
	}

	if(VisualComponent::isAnyVisible(GameObject::safeCast(this)))
	{
		return true;
	}

	if(NULL != this->children && recursive)
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

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setSpec(void* entitySpec)
{
	// save spec
	this->entitySpec = entitySpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setExtraInfo(void* extraInfo __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::alwaysStreamIn()
{
	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::calculateSizeFromChildren(RightBox* rightBox, Vector3D environmentPosition)
{
	Vector3D globalPosition = Vector3D::sum(environmentPosition, this->localTransformation.position);

	RightBox myRightBox = {0, 0, 0, 0, 0, 0};

	if(0 == this->size.x || 0 == this->size.y || 0 == this->size.z)
	{
		bool rightBoxComputed = VisualComponent::calculateRightBox(GameObject::safeCast(this), &myRightBox);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destroyEntityFactory()
{
	if(!isDeleted(this->entityFactory))
	{
		delete this->entityFactory;
		this->entityFactory = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

