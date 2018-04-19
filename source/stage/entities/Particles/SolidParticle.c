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

#include <SolidParticle.h>
#include <Game.h>
#include <Ball.h>
#include <CollisionManager.h>
#include <MessageDispatcher.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	SolidParticle
 * @extends Particle
 * @ingroup stage-entities-particles
 */
__CLASS_DEFINITION(SolidParticle, Particle);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(Shape);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SolidParticle_transformShape(SolidParticle this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass)
__CLASS_NEW_END(SolidParticle, solidParticleDefinition, spriteDefinition, lifeSpan, mass);

/**
 * Class constructor
 *
 * @memberof						SolidParticle
 * @public
 *
 * @param this						Function scope
 * @param solidParticleDefinition	Definition of the SolidParticle
 * @param spriteDefinition
 * @param lifeSpan
 * @param mass
 */
void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass)
{
	ASSERT(this, "SolidParticle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Particle, &solidParticleDefinition->particleDefinition, spriteDefinition, lifeSpan, mass);

	this->solidParticleDefinition = solidParticleDefinition;

	ShapeDefinition shapeDefinition =
	{
		// shape
		__TYPE(Ball),

		{solidParticleDefinition->radius, solidParticleDefinition->radius, solidParticleDefinition->radius},

		// displacement (x, y, z, p)
		{0, 0, 0, 0},

		// rotation (x, y, z)
		{0, 0, 0},

		// scale (x, y, z)
		{__I_TO_FIX7_9(1), __I_TO_FIX7_9(1), __I_TO_FIX7_9(1)},

		// check for collisions against other shapes
		true,

		/// layers in which I live
		this->solidParticleDefinition->layers,

		/// layers to ignore when checking for collisions
		this->solidParticleDefinition->layersToIgnore,
	};

	// register a shape for collision detection
	this->shape = CollisionManager_createShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), &shapeDefinition);
	CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);

	// has to set bounciness and friction myself since Particle ignores collisions
	Body_setBounciness(this->body, this->solidParticleDefinition->bounciness);
	Body_setFrictionCoefficient(this->body, this->solidParticleDefinition->frictionCoefficient);
}

/**
 * Class destructor
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 */
void SolidParticle_destructor(SolidParticle this)
{
	ASSERT(this, "SolidParticle::destructor: null this");

	// unregister the shape for collision detection
	CollisionManager_destroyShape(Game_getCollisionManager(Game_getInstance()), this->shape);

	this->shape = NULL;

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Update
 *
 * @memberof			SolidParticle
 * @public
 *
 * @param this			Function scope
 * @param timeElapsed
 * @param behavior
 *
 * @return				Boolean
 */
u32 SolidParticle_update(SolidParticle this, int timeElapsed, void (* behavior)(Particle particle))
{
	ASSERT(this, "SolidParticle::update: null this");

	u32 expired = __CALL_BASE_METHOD(Particle, update, this, timeElapsed, behavior);

	if(0 <= this->lifeSpan)
	{
		SolidParticle_transformShape(this);
	}

//	Body_print(this->body, 1, 1);
//	Shape_print(this->shape, 21, 6);

	return expired;
}

/**
 * Transform shape
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 */
static void SolidParticle_transformShape(SolidParticle this)
{
	const Rotation shapeRotation = {0, 0, 0};
	const Scale shapeScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
	const Size shapeSize = {this->solidParticleDefinition->radius, this->solidParticleDefinition->radius, this->solidParticleDefinition->radius};

	__VIRTUAL_CALL(Shape, position, this->shape, Body_getPosition(this->body), &shapeRotation, &shapeScale, &shapeSize);
}

/**
 * Retrieve shape
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Particle's shape
 */
Shape SolidParticle_getShape(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getShape: null this");

	return this->shape;
}

/**
 * Get width
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Width
 */
u16 SolidParticle_getWidth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getWidth: null this");

	return __FIX10_6_TO_I(this->solidParticleDefinition->radius);
}

/**
 * Get height
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Height
 */
u16 SolidParticle_getHeight(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getHeight: null this");

	return __FIX10_6_TO_I(this->solidParticleDefinition->radius);
}

/**
 * Get depth
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Depth
 */
u16 SolidParticle_getDepth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getDepth: null this");

	// must calculate based on the scale because not affine object must be enlarged
	return __FIX10_6_TO_I(this->solidParticleDefinition->radius);
}

/**
 * Process collisions
 *
 * @memberof							SolidParticle
 * @public
 *
 * @param this							Function scope
 * @param collisionInformation			Information about the collision
 *
 * @return								True if successfully processed, false otherwise
 */
bool SolidParticle_enterCollision(SolidParticle this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "SolidParticle::SolidParticle: null this");

	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collisionInformation->collidingShape, "SolidParticle::resolveCollision: collidingShapes");

	ASSERT(collisionInformation->collidingShape, "SolidParticle::enterCollision: collidingShapes");

	bool returnValue = false;

	if(collisionInformation->shape && collisionInformation->collidingShape)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Shape_resolveCollision(collisionInformation->shape, collisionInformation);

			fix10_6 frictionCoefficient = __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, Shape_getOwner(collisionInformation->collidingShape));
			fix10_6 bounciness = __VIRTUAL_CALL(SpatialObject, getBounciness, Shape_getOwner(collisionInformation->collidingShape));

			Body_bounce(this->body, __SAFE_CAST(Object, collisionInformation->collidingShape), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			returnValue = true;
		}
	}

	return returnValue;
}

/**
 * Can move over axis?
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param acceleration
 *
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool SolidParticle_isSubjectToGravity(SolidParticle this, Acceleration gravity)
{
	ASSERT(this, "Particle::isSubjectToGravity: null this");
	ASSERT(this->shape, "Particle::isSubjectToGravity: null shape");

	fix10_6 collisionCheckDistance = __I_TO_FIX10_6(1);

	Vector3D displacement =
	{
		gravity.x ? 0 < gravity.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.y ? 0 < gravity.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.z ? 0 < gravity.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	return Shape_canMoveTowards(this->shape, displacement, 0);
}

/**
 * Handles incoming messages
 *
 * @memberof		SolidParticle
 * @public
 *
 * @param this		Function scope
 * @param telegram
 *
 * @return			True if successfully processed, false otherwise
 */
bool SolidParticle_handleMessage(SolidParticle this, Telegram telegram)
{
	ASSERT(this, "SolidParticle::handleMessage: null this");

	switch(Telegram_getMessage(telegram))
	{
		case kBodyStartedMoving:

			CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
			return true;
			break;

		case kBodyStopped:

			if(this->solidParticleDefinition->disableCollisionOnStop && !Body_getMovementOnAllAxes(this->body))
			{
				CollisionManager_shapeStoppedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
			}
			break;
	}

	return false;
}

/**
 * Transform
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 */
void SolidParticle_transform(SolidParticle this)
{
	ASSERT(this, "SolidParticle::transform: null this");

	SolidParticle_transformShape(this);
}


/**
 * Set position
 *
 * @memberof		SolidParticle
 * @public
 *
 * @param this		Function scope
 * @param position	Position to move particle to
 */
void SolidParticle_setPosition(SolidParticle this, const Vector3D* position)
{
	ASSERT(this, "SolidParticle::position: null this");

	__CALL_BASE_METHOD(Particle, setPosition, this, position);

//	SolidParticle_transformShape(this);
}

/**
 * Retrieve shapes list
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		SolidParticle's Shape list
 */
VirtualList SolidParticle_getShapes(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getShapes: null this");

	static VirtualList shapesList = NULL;

	if(!shapesList)
	{
		shapesList = __NEW(VirtualList);
	}

	VirtualList_clear(shapesList);

	VirtualList_pushBack(shapesList, this->shape);

	return shapesList;
}

/**
 * Get in game type
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Type of entity within the game's logic
 */
u32 SolidParticle_getInGameType(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getInGameType: null this");

	return this->solidParticleDefinition->inGameType;
}

/**
 * Get velocity
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Velocity vector
 */
Velocity SolidParticle_getVelocity(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getVelocity: null this");

	return Body_getVelocity(this->body);
}

/**
 * Inform me about not colliding shape
 *
 * @memberof					SolidParticle
 * @public
 *
 * @param this					Function scope
 * @param shapeNotCollidingAnymore		Shape that is no longer colliding
 */
void SolidParticle_exitCollision(SolidParticle this, Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	ASSERT(this, "SolidParticle::exitCollision: null this");
	ASSERT(this->body, "SolidParticle::exitCollision: null this");

	if(isShapeImpenetrable)
	{
		Body_clearNormal(this->body, __SAFE_CAST(Object, shapeNotCollidingAnymore));
	}

	Body_setSurroundingFrictionCoefficient(this->body, Shape_getCollidingFrictionCoefficient(this->shape));
}


/**
 * Reset
 *
 * @memberof	SolidParticle
 * @public
 *
 * @param this	Function scope
 */
void SolidParticle_reset(SolidParticle this)
{
	ASSERT(this, "SolidParticle::reset: null this");

	__CALL_BASE_METHOD(Particle, reset, this);

	Shape_reset(this->shape);
}
