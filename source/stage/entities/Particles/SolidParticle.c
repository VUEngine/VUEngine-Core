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

#include <SolidParticle.h>
#include <Game.h>
#include <CollisionManager.h>
#include <MessageDispatcher.h>


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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SolidParticle, const SolidParticleDefinition* shapeParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
__CLASS_NEW_END(SolidParticle, shapeParticleDefinition, spriteDefinition, lifeSpan, mass);

/**
 * Class constructor
 *
 * @memberof						SolidParticle
 * @public
 *
 * @param this						Function scope
 * @param shapeParticleDefinition	Definition of the SolidParticle
 * @param spriteDefinition
 * @param lifeSpan
 * @param mass
 */
void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* shapeParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "SolidParticle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Particle, &shapeParticleDefinition->particleDefinition, spriteDefinition, lifeSpan, mass);

	this->shapeParticleDefinition = shapeParticleDefinition;

	// register a shape for collision detection
	this->shape = CollisionManager_createShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), shapeParticleDefinition->shapeDefinition);

	Rotation rotation = {0, 0, 0};
	Scale scale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
	Size size = {SolidParticle_getWidth(this), SolidParticle_getHeight(this), SolidParticle_getDepth(this)};
	__VIRTUAL_CALL(Shape, setup, this->shape, Body_getPosition(this->body), &rotation, &scale, &size);

	this->collisionSolver = __NEW(CollisionSolver, __SAFE_CAST(SpatialObject, this));
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

	if(this->collisionSolver)
	{
		__DELETE(this->collisionSolver);
		this->collisionSolver = NULL;
	}

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
		this->position = *Body_getPosition(this->body);
	}

//	__VIRTUAL_CALL(Shape, position, this->shape, Body_getPosition(this->body), false);

	return expired;
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

	return this->shapeParticleDefinition->width;
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

	return this->shapeParticleDefinition->height;
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
	return this->shapeParticleDefinition->depth;
}

/**
 * Start bouncing after collision with another Entity
 *
 * @memberof				SolidParticle
 * @private
 *
 * @param this				Function scope
 * @param axisOfCollision
 */
static void SolidParticle_checkIfMustBounce(SolidParticle this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "SolidParticle::bounce: null this");
/*
	fix19_13 otherSpatialObjectsElasticity = this->collisionSolver ? CollisionSolver_getSurroundingElasticity(this->collisionSolver) : __1I_FIX19_13;

	Body_bounce(this->body, collisionInformation, this->shapeParticleDefinition->axisAllowedForBouncing, otherSpatialObjectsElasticity);

	if(!(axisOfCollision & Body_getMovementOnAllAxes(this->body)))
	{
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyStopped, collisionInformation);
	}
	else
	{
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyBounced, &axisOfCollision);
	}
	*/
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
bool SolidParticle_processCollision(SolidParticle this, CollisionInformation collisionInformation)
{
	ASSERT(this, "SolidParticle::SolidParticle: null this");

	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collisionInformation.collidingShape, "SolidParticle::resolveCollision: collidingShapes");

	if(this->collisionSolver)
	{
		if(this->shapeParticleDefinition->ignoreParticles)
		{
			if(__GET_CAST(Particle, Shape_getOwner(collisionInformation.collidingShape)))
			{
				return true;
			}
		}

		CollisionSolver_resolveCollision(this->collisionSolver, &collisionInformation);

		SolidParticle_checkIfMustBounce(this, &collisionInformation);

		return true;
	}

	return false;
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
			CollisionSolver_purgeCollidingShapesList(this->collisionSolver);
			return true;
			break;

		case kBodyStopped:

			if(!Body_getMovementOnAllAxes(this->body))
			{
				//CollisionManager_shapeStoppedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
			}
			break;

		case kBodyBounced:

			return true;
			break;
	}

	return false;
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

	CollisionSolver_purgeCollidingShapesList(this->collisionSolver);

	this->position = *position;
}
