/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SolidParticle.h>

#include <Ball.h>
#include <Body.h>
#include <CollisionManager.h>
#include <Collider.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include <DebugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Collider;
friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param solidParticleSpec	Spec of the SolidParticle
 * @param spriteSpec
 * @param lifeSpan
 * @param mass
 */
void SolidParticle::constructor(const SolidParticleSpec* solidParticleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, ParticleSystem creator)
{
	// construct base Container
	Base::constructor(&solidParticleSpec->physicalParticleSpec, spriteSpec, wireframeSpec, lifeSpan, creator);

	this->creator = creator;
	this->solidParticleSpec = solidParticleSpec;

	ColliderSpec shapeSpec =
	{
		// collider
		__TYPE(Ball),

		{solidParticleSpec->radius, solidParticleSpec->radius, solidParticleSpec->radius},

		// displacement (x, y, z, p)
		{0, 0, 0, 0},

		// rotation (x, y, z)
		{0, 0, 0},

		// scale (x, y, z)
		{1, 1, 1},

		// check for collisions against other shapes
		true,

		/// layers in which I live
		this->solidParticleSpec->layers,

		/// layers to ignore when checking for collisions
		this->solidParticleSpec->layersToIgnore,
	};

	// register a collider for collision detection
	this->collider = CollisionManager::createCollider(VUEngine::getCollisionManager(VUEngine::getInstance()), SpatialObject::safeCast(this), &shapeSpec);
	Collider::activeCollisionChecks(this->collider, true);

	// has to set bounciness and friction myself since Particle ignores collisions
	Body::setBounciness(this->body, this->solidParticleSpec->bounciness);
	Body::setFrictionCoefficient(this->body, this->solidParticleSpec->frictionCoefficient);
}

/**
 * Class destructor
 */
void SolidParticle::destructor()
{
	// unregister the collider for collision detection
	CollisionManager::destroyCollider(VUEngine::getCollisionManager(VUEngine::getInstance()), this->collider);

	this->collider = NULL;

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Transform
 *
 */
void SolidParticle::transform()
{
	Base::transform(this);

	SolidParticle::transformCollider(this);
}

/**
 * Transform collider
 */
void SolidParticle::transformCollider()
{
	const Rotation shapeRotation = Rotation::zero();
	const Scale shapeScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
	const Size shapeSize = {this->solidParticleSpec->radius, this->solidParticleSpec->radius, this->solidParticleSpec->radius};

	Collider::transform(this->collider, Body::getPosition(this->body), &shapeRotation, &shapeScale, &shapeSize);
}

/**
 * Retrieve collider
 *
 * @return		Particle's collider
 */
Collider SolidParticle::getCollider()
{
	return this->collider;
}

/**
 * Get width
 *
 * @return		Width
 */
fixed_t SolidParticle::getWidth()
{
	return this->solidParticleSpec->radius;
}

/**
 * Get height
 *
 * @return		Height
 */
fixed_t SolidParticle::getHeight()
{
	return this->solidParticleSpec->radius;
}

/**
 * Get depth
 *
 * @return		Depth
 */
fixed_t SolidParticle::getDepth()
{
	// must calculate based on the scale because not affine object must be enlarged
	return this->solidParticleSpec->radius;
}

/**
 * Process collisions
 *
 * @param collisionInformation			Information about the collision
 * @return								True if successfully processed, false otherwise
 */
bool SolidParticle::enterCollision(const CollisionInformation* collisionInformation)
{
	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collisionInformation->otherCollider, "SolidParticle::resolveCollision: otherColliders");

	ASSERT(collisionInformation->otherCollider, "SolidParticle::enterCollision: otherColliders");

	bool returnValue = false;

	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Collider::resolveCollision(collisionInformation->collider, collisionInformation, false);

			SpatialObject owner = Collider::getOwner(collisionInformation->otherCollider);
			fixed_t frictionCoefficient =  SpatialObject::getFrictionCoefficient(owner);
			fixed_t bounciness =  SpatialObject::getBounciness(owner);

			Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->otherCollider), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			returnValue = true;
		}

		if(NULL != this->solidParticleSpec->onCollisionAnimation && !isDeleted(this->creator))
		{
			Sprite::play(this->sprite, ParticleSystem::getAnimationFunctions(this->creator), this->solidParticleSpec->onCollisionAnimation, ListenerObject::safeCast(this));
		}
	}

	return returnValue;
}

/**
 * Can move over axis?
 *
 * @param acceleration
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool SolidParticle::isSubjectToGravity(Vector3D gravity)
{
	ASSERT(this->collider, "Particle::isSubjectToGravity: null collider");

	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		gravity.x ? 0 < gravity.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.y ? 0 < gravity.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.z ? 0 < gravity.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	return Collider::canMoveTowards(this->collider, displacement, 0);
}

/**
 * Handles incoming messages
 *
 * @param telegram
 * @return			True if successfully processed, false otherwise
 */
bool SolidParticle::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageBodyStartedMoving:

			Collider::activeCollisionChecks(this->collider, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(this->solidParticleSpec->disableCollisionOnStop && !Body::getMovementOnAllAxis(this->body))
			{
				Collider::activeCollisionChecks(this->collider, false);
			}
			break;
	}

	return false;
}

/**
 * Set position
 *
 * @param position	Position to move particle to
 */
void SolidParticle::setPosition(const Vector3D* position)
{
	Base::setPosition(this, position);

//	SolidParticle::transformCollider(this);
}

/**
 * Retrieve shapes list
 *
 * @return		SolidParticle's Collider list
 */
VirtualList SolidParticle::getColliders()
{
	static VirtualList shapesList = NULL;

	if(!shapesList)
	{
		shapesList = new VirtualList();
	}

	VirtualList::clear(shapesList);

	VirtualList::pushBack(shapesList, this->collider);

	return shapesList;
}

/**
 * Get in game type
 *
 * @return		Type of entity within the game's logic
 */
uint32 SolidParticle::getInGameType()
{
	return this->solidParticleSpec->inGameType;
}

/**
 * Get velocity
 *
 * @return		Vector3D vector
 */
const Vector3D* SolidParticle::getVelocity()
{
	return Body::getVelocity(this->body);
}

/**
 * Inform me about not colliding collider
 *
 * @param shapeNotCollidingAnymore		Collider that is no longer colliding
 */
void SolidParticle::exitCollision(Collider collider __attribute__ ((unused)), Collider shapeNotCollidingAnymore, bool isColliderImpenetrable)
{
	ASSERT(this->body, "SolidParticle::exitCollision: null this");

	if(isColliderImpenetrable)
	{
		Body::clearNormal(this->body, ListenerObject::safeCast(shapeNotCollidingAnymore));
	}

	Body::setSurroundingFrictionCoefficient(this->body, Collider::getCollidingFrictionCoefficient(this->collider));
}

/**
 * Reset
 */
void SolidParticle::reset()
{
	Base::reset(this);
	Collider::reset(this->collider);
}
