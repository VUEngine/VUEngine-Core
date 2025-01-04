/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <DebugConfig.h>
#include <ActorFactory.h>
#include <Optics.h>
#include <Mesh.h>
#include <Printing.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Actor.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define ENTITY_MIN_SIZE							16
#define ENTITY_HALF_MIN_SIZE					(ENTITY_MIN_SIZE >> 1)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Actor Actor::createActor(const PositionedActor* const positionedActor, int16 internalId)
{
	NM_ASSERT(NULL != positionedActor, "Actor::createActor: null positionedActor");
	NM_ASSERT(NULL != positionedActor->actorSpec, "Actor::createActor: null spec");
	NM_ASSERT(NULL != positionedActor->actorSpec->allocator, "Actor::createActor: no allocator defined");

	if(NULL == positionedActor)
	{
		return NULL;
	}

	Actor actor = Actor::instantiate(positionedActor, internalId, positionedActor->name);
	ASSERT(actor, "Actor::loadFromSpec: actor not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedActor->onScreenPosition);
	Rotation rotation = Rotation::getFromScreenPixelRotation(positionedActor->onScreenRotation);
	Scale scale = Scale::getFromScreenPixelScale(positionedActor->onScreenScale);

	Actor::setLocalPosition(actor, &position);
	Actor::setLocalRotation(actor, &rotation);
	Actor::setLocalScale(actor, &scale);

	// add children if defined
	if(NULL != positionedActor->childrenSpecs)
	{
		Actor::addChildActors(actor, positionedActor->childrenSpecs);
	}

	if(NULL != positionedActor->actorSpec->childrenSpecs)
	{
		Actor::addChildActors(actor, positionedActor->actorSpec->childrenSpecs);
	}

	return actor;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Actor Actor::createActorDeferred(const PositionedActor* const positionedActor, int16 internalId)
{
	NM_ASSERT(NULL != positionedActor, "Actor::createActorDeferred: null positionedActor");
	NM_ASSERT(NULL != positionedActor->actorSpec, "Actor::createActorDeferred: null spec");
	NM_ASSERT(NULL != positionedActor->actorSpec->allocator, "Actor::createActorDeferred: no allocator defined");

	if(!positionedActor)
	{
		return NULL;
	}

	Actor actor = Actor::instantiate(positionedActor, internalId, positionedActor->name);

	NM_ASSERT(!isDeleted(actor), "Actor::createActorDeferred: actor not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedActor->onScreenPosition);
	Rotation rotation = Rotation::getFromScreenPixelRotation(positionedActor->onScreenRotation);
	Scale scale = Scale::getFromScreenPixelScale(positionedActor->onScreenScale);

	Actor::setLocalPosition(actor, &position);
	Actor::setLocalRotation(actor, &rotation);
	Actor::setLocalScale(actor, &scale);

	// add children if defined
	if(positionedActor->childrenSpecs)
	{
		Actor::addChildActorsDeferred(actor, positionedActor->childrenSpecs);
	}

	if(positionedActor->actorSpec->childrenSpecs)
	{
		Actor::addChildActorsDeferred(actor, positionedActor->actorSpec->childrenSpecs);
	}

	return actor;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static RightBox Actor::getRightBoxFromSpec(const PositionedActor* positionedActor, const Vector3D* environmentPosition)
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	Actor::getRightBoxFromChildrenSpec(positionedActor, environmentPosition, &rightBox);

	Vector3D globalPosition = Vector3D::getFromScreenPixelVector(positionedActor->onScreenPosition);

	rightBox.x0 -= globalPosition.x;
	rightBox.x1 -= globalPosition.x;
	rightBox.y0 -= globalPosition.y;
	rightBox.y1 -= globalPosition.y;
	rightBox.z0 -= globalPosition.z;
	rightBox.z1 -= globalPosition.z;

	return rightBox;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Actor::getRightBoxFromChildrenSpec
(
	const PositionedActor* positionedActor, const Vector3D* environmentPosition, RightBox* rightBox
)
{
	ASSERT(positionedActor, "Actor::getRightBoxFromChildrenSpec: null positionedActor");
	ASSERT(positionedActor->actorSpec, "Actor::getRightBoxFromChildrenSpec: null actorSpec");

	RightBox myRightBox = {0, 0, 0, 0, 0, 0};

	if
	(
		0 != positionedActor->actorSpec->pixelSize.x 
		|| 
		0 != positionedActor->actorSpec->pixelSize.y 
		|| 
		0 != positionedActor->actorSpec->pixelSize.z
	)
	{
		fixed_t halfWidth = __PIXELS_TO_METERS(positionedActor->actorSpec->pixelSize.x) >> 1;
		fixed_t halfHeight = __PIXELS_TO_METERS(positionedActor->actorSpec->pixelSize.y) >> 1;
		fixed_t halfDepth = __PIXELS_TO_METERS(positionedActor->actorSpec->pixelSize.z) >> 1;

		myRightBox.x0 = -halfWidth;
		myRightBox.x1 = halfWidth;
		myRightBox.y0 = -halfHeight;
		myRightBox.y1 = halfHeight;
		myRightBox.z0 = -halfDepth;
		myRightBox.z1 = halfDepth;
	}
	else 
	{
		if(NULL != positionedActor->actorSpec->componentSpecs && NULL != positionedActor->actorSpec->componentSpecs[0])
		{
			for(int16 i = 0; NULL != positionedActor->actorSpec->componentSpecs[i]; i++)
			{
				RightBox helperRightBox = {0, 0, 0, 0, 0, 0};

				switch (positionedActor->actorSpec->componentSpecs[i]->componentType)
				{
					case kSpriteComponent:
					{
						SpriteSpec* spriteSpec = (SpriteSpec*)positionedActor->actorSpec->componentSpecs[i];

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
						helperRightBox = Mesh::getRightBoxFromSpec((MeshSpec*)positionedActor->actorSpec->componentSpecs[i]);
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

	Vector3D globalPosition = 
		Vector3D::sum(*environmentPosition, Vector3D::getFromScreenPixelVector(positionedActor->onScreenPosition));

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

	if(NULL != positionedActor->childrenSpecs)
	{
		for(int32 i = 0; positionedActor->childrenSpecs[i].actorSpec; i++)
		{
			Actor::getRightBoxFromChildrenSpec(&positionedActor->childrenSpecs[i], &globalPosition, rightBox);
		}
	}

	if(NULL != positionedActor->actorSpec->childrenSpecs)
	{
		for(int32 i = 0; positionedActor->actorSpec->childrenSpecs[i].actorSpec; i++)
		{
			Actor::getRightBoxFromChildrenSpec(&positionedActor->actorSpec->childrenSpecs[i], &globalPosition, rightBox);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Actor Actor::instantiate(const PositionedActor* const positionedActor, int16 internalId, const char* const name)
{
	NM_ASSERT(NULL != positionedActor, "Actor::instantiate: null positionedActor");
	NM_ASSERT(NULL != positionedActor->actorSpec, "Actor::instantiate: null spec");
	NM_ASSERT(NULL != positionedActor->actorSpec->allocator, "Actor::instantiate: no allocator defined");

	if(NULL == positionedActor || NULL == positionedActor->actorSpec || NULL == positionedActor->actorSpec->allocator)
	{
		return NULL;
	}

	// Call the appropriate allocator to support inheritance
	Actor actor = 
		((Actor (*)(ActorSpec*, int16, const char* const)) positionedActor->actorSpec->allocator)
		(
			(ActorSpec*)positionedActor->actorSpec, internalId, name
		);

	// process extra info
	if(NULL != positionedActor->extraInfo)
	{
		Actor::setExtraInfo(actor, positionedActor->extraInfo);
	}

	if(NULL != positionedActor->actorSpec->extraInfo)
	{
		Actor::setExtraInfo(actor, positionedActor->actorSpec->extraInfo);
	}

	return actor;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::constructor(ActorSpec* actorSpec, int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor(internalId, name);

	this->actorSpec = actorSpec;
	this->size = Size::getFromPixelSize(actorSpec->pixelSize);
	this->centerDisplacement = NULL;
	this->actorFactory = NULL;
	this->playingAnimationName = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::destructor()
{
	Actor::destroyComponents(this);
	Actor::destroyActorFactory(this);

	if(NULL != this->centerDisplacement)
	{
		delete this->centerDisplacement;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::createComponents(ComponentSpec** componentSpecs)
{
	Base::createComponents(this, NULL != componentSpecs ? componentSpecs : this->actorSpec->componentSpecs);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::destroyComponents()
{
	Base::destroyComponents(this);

	this->size = (Size){0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Actor::getRadius()
{
	fixed_t width = Actor::getWidth(this);
	fixed_t height = Actor::getHeight(this);
	fixed_t depth = Actor::getDepth(this);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Actor::getInGameType()
{
	return this->actorSpec->inGameType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::ready(bool recursive)
{
	ASSERT(this->actorSpec, "Actor::ready: null actorSpec");

	Base::ready(this, recursive);

	Actor::playAnimation(this, ((ActorSpec*)this->actorSpec)->initialAnimation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::suspend()
{
	Base::suspend(this);

	Actor::removeComponents(this, kSpriteComponent);
	Actor::removeComponents(this, kWireframeComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::resume()
{
	if(NULL != this->actorSpec)
	{
		Actor::addComponents(this, this->actorSpec->componentSpecs, kSpriteComponent);
		Actor::addComponents(this, this->actorSpec->componentSpecs, kWireframeComponent);
	}

	Base::resume(this);

	Actor::playAnimation(this, this->playingAnimationName);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::handleCommand(int32 command, va_list args)
{
	switch (command)
	{
		case kMessageShow:
		{
			Actor::show(this);			
			break;
		}

		case kMessageHide:
		{
			Actor::hide(this);			
			break;
		}

		case kMessageSetTransparency:
		{
			Actor::setTransparency(this, (uint8)va_arg(args, uint32));			
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::handlePropagatedString(const char* string __attribute__ ((unused)))
{
	/* TODO: play only if the string contains the correct command */
	/*
	if (NULL == strnstr(string, __MAX_ANIMATION_FUNCTION_NAME_LENGTH, __ANIMATION_COMMAND)) 
	{
		return false;
	}
	*/

	Actor::playAnimation(this, string);
	
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ActorSpec* Actor::getSpec()
{
	return this->actorSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ActorFactory Actor::getActorFactory()
{
	return this->actorFactory;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor Actor::spawnChildActor(const PositionedActor* const positionedActor)
{
	if(NULL != positionedActor)
	{
		Actor actor = Actor::createActor(positionedActor, this->internalId + Actor::getChildrenCount(this));
		NM_ASSERT(!isDeleted(actor), "Stage::doAddChildActor: actor not loaded");

		if(!isDeleted(actor))
		{
			// create the actor and add it to the world
			Actor::addChild(this, Container::safeCast(actor));
		}

		return actor;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::addChildActors(const PositionedActor* childrenSpecs)
{
	if(NULL == childrenSpecs)
	{
		return;
	}

	int16 internalId = this->internalId + (!isDeleted(this->children) ? VirtualList::getCount(this->children) : 1);

	for(int32 i = 0; NULL != childrenSpecs[i].actorSpec; i++)
	{
		Actor actor = Actor::createActor(&childrenSpecs[i], internalId++);

		if(!isDeleted(actor))
		{
			Actor::addChild(this, Container::safeCast(actor));
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::addChildActorsDeferred(const PositionedActor* childrenSpecs)
{
	ASSERT(NULL != childrenSpecs, "Actor::addChildActorsDeferred: null childrenSpecs");

	if(NULL == childrenSpecs)
	{
		return;
	}

	if(isDeleted(this->actorFactory))
	{
		this->actorFactory = new ActorFactory();

		Actor::addEventListener
		(
			this, ListenerObject::safeCast(this), (EventListener)Actor::onActorLoadedDeferred, kEventActorLoaded
		);
	}

	for(int32 i = 0; NULL != childrenSpecs[i].actorSpec; i++)
	{
		ActorFactory::spawnActor
		(
			this->actorFactory, &childrenSpecs[i], Container::safeCast(this), NULL, this->internalId + Actor::getChildrenCount(this)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::calculateSize()
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	Actor::calculateSizeFromChildren(this, &rightBox, Vector3D::zero());

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

	NM_ASSERT(0 < this->size.x, "Actor::calculateSize: 0 x size");
	NM_ASSERT(0 < this->size.y, "Actor::calculateSize: 0 y size");
	NM_ASSERT(0 < this->size.z, "Actor::calculateSize: 0 z size");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Actor::getWidth()
{
	if(0 == this->size.x)
	{
		NM_ASSERT(false, "Actor::getWidth: 0 x size");
		Actor::calculateSize(this);
	}

	return this->size.x;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Actor::getHeight()
{
	if(0 == this->size.y)
	{
		NM_ASSERT(false, "Actor::getHeight: 0 y size");
		Actor::calculateSize(this);
	}

	return this->size.y;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Actor::getDepth()
{
	if(0 == this->size.z)
	{
		NM_ASSERT(false, "Actor::getDepth: 0 z size");
		Actor::calculateSize(this);
	}

	return this->size.z;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::isInCameraRange(int16 padding, bool recursive)
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

	if(Actor::isInsideFrustrum(position3D, rightBox))
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
			Actor child = Actor::safeCast(VirtualNode::getData(childNode));

			if(child->hidden)
			{
				continue;
			}

			if(Actor::isInCameraRange(child, padding, true))
			{
				return true;
			}
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::playAnimation(const char* animationName)
{
	this->playingAnimationName = animationName;

	SpriteManager::propagateCommand
	(
		SpriteManager::getInstance(), cVisualComponentCommandPlay, GameObject::safeCast(this), 
		((ActorSpec*)this->actorSpec)->animationFunctions, animationName, ListenerObject::safeCast(this), 
		(EventListener)Actor::onAnimationComplete
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::pauseAnimation(bool pause)
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandPause, GameObject::safeCast(this), pause);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::stopAnimation()
{
	this->playingAnimationName = NULL;

	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandStop, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::setActualFrame(int16 frame)
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandSetFrame, GameObject::safeCast(this), frame);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::nextFrame()
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandNextFrame, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::previousFrame()
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandPreviousFrame, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::isPlaying()
{
	return NULL != this->playingAnimationName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::isPlayingAnimation(char* animationName)
{
	return 0 == strcmp(this->playingAnimationName, animationName);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const char* Actor::getPlayingAnimationName()
{
	return this->playingAnimationName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::setSpec(void* actorSpec)
{
	// save spec
	this->actorSpec = actorSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::setExtraInfo(void* extraInfo __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::alwaysStreamIn()
{
	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::calculateSizeFromChildren(RightBox* rightBox, Vector3D environmentPosition)
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
			Actor::calculateSizeFromChildren(childNode->data, rightBox, globalPosition);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::destroyActorFactory()
{
	if(!isDeleted(this->actorFactory))
	{
		delete this->actorFactory;
		this->actorFactory = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::onActorLoadedDeferred(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(ListenerObject::safeCast(this) != eventFirer)
	{
		return false;
	} 

	if(isDeleted(this->actorFactory))
	{
		return false;
	}

	if(!ActorFactory::hasActorsPending(this->actorFactory))
	{
		Actor::destroyActorFactory(this);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::onAnimationComplete(ListenerObject eventFirer __attribute__((unused)))
{
	this->playingAnimationName = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
