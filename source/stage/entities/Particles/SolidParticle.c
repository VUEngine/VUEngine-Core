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
#include <Game.h>
#include <Ball.h>
#include <CollisionManager.h>
#include <MessageDispatcher.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class Shape;


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
void SolidParticle::constructor(const SolidParticleSpec* solidParticleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan)
{
	// construct base Container
	Base::constructor(&solidParticleSpec->physicalParticleSpec, spriteSpec, wireframeSpec, lifeSpan);

	this->solidParticleSpec = solidParticleSpec;

	ShapeSpec shapeSpec =
	{
		// shape
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

	// register a shape for collision detection
	this->shape = CollisionManager::createShape(Game::getCollisionManager(Game::getInstance()), SpatialObject::safeCast(this), &shapeSpec);
	Shape::activeCollisionChecks(this->shape, true);

	// has to set bounciness and friction myself since Particle ignores collisions
	Body::setBounciness(this->body, this->solidParticleSpec->bounciness);
	Body::setFrictionCoefficient(this->body, this->solidParticleSpec->frictionCoefficient);
}

/**
 * Class destructor
 */
void SolidParticle::destructor()
{
	// unregister the shape for collision detection
	CollisionManager::destroyShape(Game::getCollisionManager(Game::getInstance()), this->shape);

	this->shape = NULL;

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Transform
 */
void SolidParticle::transform()
{
	Base::transform(this);

	SolidParticle::transformShape(this);
}

/**
 * Transform shape
 */
void SolidParticle::transformShape()
{
	const Rotation shapeRotation = Rotation::zero();
	const Scale shapeScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
	const Size shapeSize = {this->solidParticleSpec->radius, this->solidParticleSpec->radius, this->solidParticleSpec->radius};

	Shape::position(this->shape, Body::getPosition(this->body), &shapeRotation, &shapeScale, &shapeSize);
}

/**
 * Retrieve shape
 *
 * @return		Particle's shape
 */
Shape SolidParticle::getShape()
{
	return this->shape;
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
	ASSERT(collisionInformation->collidingShape, "SolidParticle::resolveCollision: collidingShapes");

	ASSERT(collisionInformation->collidingShape, "SolidParticle::enterCollision: collidingShapes");

	bool returnValue = false;

	if(collisionInformation->shape && collisionInformation->collidingShape)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Shape::resolveCollision(collisionInformation->shape, collisionInformation, false);

			fixed_t frictionCoefficient =  SpatialObject::getFrictionCoefficient(Shape::getOwner(collisionInformation->collidingShape));
			fixed_t bounciness =  SpatialObject::getBounciness(Shape::getOwner(collisionInformation->collidingShape));

			Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->collidingShape), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			returnValue = true;
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
bool SolidParticle::isSubjectToGravity(Acceleration gravity)
{
	ASSERT(this->shape, "Particle::isSubjectToGravity: null shape");

	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		gravity.x ? 0 < gravity.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.y ? 0 < gravity.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.z ? 0 < gravity.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	return Shape::canMoveTowards(this->shape, displacement, 0);
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

			Shape::activeCollisionChecks(this->shape, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(this->solidParticleSpec->disableCollisionOnStop && !Body::getMovementOnAllAxis(this->body))
			{
				Shape::activeCollisionChecks(this->shape, false);
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

//	SolidParticle::transformShape(this);
}

/**
 * Retrieve shapes list
 *
 * @return		SolidParticle's Shape list
 */
VirtualList SolidParticle::getShapes()
{
	static VirtualList shapesList = NULL;

	if(!shapesList)
	{
		shapesList = new VirtualList();
	}

	VirtualList::clear(shapesList);

	VirtualList::pushBack(shapesList, this->shape);

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
 * @return		Velocity vector
 */
Velocity SolidParticle::getVelocity()
{
	return Body::getVelocity(this->body);
}

/**
 * Inform me about not colliding shape
 *
 * @param shapeNotCollidingAnymore		Shape that is no longer colliding
 */
void SolidParticle::exitCollision(Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	ASSERT(this->body, "SolidParticle::exitCollision: null this");

	if(isShapeImpenetrable)
	{
		Body::clearNormal(this->body, ListenerObject::safeCast(shapeNotCollidingAnymore));
	}

	Body::setSurroundingFrictionCoefficient(this->body, Shape::getCollidingFrictionCoefficient(this->shape));
}

/**
 * Reset
 */
void SolidParticle::reset()
{
	Base::reset(this);
	Shape::reset(this->shape);
}
