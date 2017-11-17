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


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SolidParticle_transformShape(SolidParticle this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
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
void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "SolidParticle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Particle, &solidParticleDefinition->particleDefinition, spriteDefinition, lifeSpan, mass);

	ShapeDefinition shapeDefinition =
	{
		// shape
		__TYPE(Ball),

		{solidParticleDefinition->radius, solidParticleDefinition->radius, solidParticleDefinition->radius},

		// displacement (x, y, z)
		{__I_TO_FIX19_13(0), __I_TO_FIX19_13(0), __I_TO_FIX19_13(0)},

		// rotation (x, y, z)
		{__I_TO_FIX19_13(0), __I_TO_FIX19_13(0), __I_TO_FIX19_13(0)},

		// scale (x, y, z)
		{__I_TO_FIX7_9(1), __I_TO_FIX7_9(1), __I_TO_FIX7_9(1)},

		// check for collisions against other shapes
		true,

		/// layers in which I live
		this->solidParticleDefinition->layers,

		/// layers to ignore when checking for collisions
		this->solidParticleDefinition->layersToIgnore,
	};

	this->solidParticleDefinition = solidParticleDefinition;
	this->collisionSolver = __NEW(CollisionSolver, __SAFE_CAST(SpatialObject, this));

	// register a shape for collision detection
	this->shape = CollisionManager_createShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), &shapeDefinition);
	CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
	Shape_setActive(this->shape, true);
	Shape_setCheckForCollisions(this->shape, true);

	Body_setElasticity(this->body, this->solidParticleDefinition->elasticity);
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
		SolidParticle_transformShape(this);

		if(CollisionSolver_purgeCollidingShapesList(this->collisionSolver))
		{
			Body_clearNormal(this->body);
			Body_setFrictionCoefficient(this->body, CollisionSolver_getSurroundingFrictionCoefficient(this->collisionSolver));
		}
	}

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
	const Size shapeSize = {__FIX19_13_TO_I(this->solidParticleDefinition->radius), __FIX19_13_TO_I(this->solidParticleDefinition->radius), __FIX19_13_TO_I(this->solidParticleDefinition->radius)};

	__VIRTUAL_CALL(Shape, setup, this->shape, Body_getPosition(this->body), &shapeRotation, &shapeScale, &shapeSize, this->solidParticleDefinition->layers, this->solidParticleDefinition->layersToIgnore);
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

	return __FIX19_13_TO_I(this->solidParticleDefinition->radius);
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

	return __FIX19_13_TO_I(this->solidParticleDefinition->radius);
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
	return __FIX19_13_TO_I(this->solidParticleDefinition->radius);
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

	ASSERT(collisionInformation.collidingShape, "Actor::processCollision: collidingShapes");

	bool returnValue = false;

	if(this->collisionSolver && collisionInformation.collidingShape)
	{
		if(CollisionSolver_resolveCollision(this->collisionSolver, &collisionInformation))
		{
			if(collisionInformation.collisionSolution.translationVectorLength)
			{
				fix19_13 frictionCoefficient = this->solidParticleDefinition->frictionCoefficient + __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, Shape_getOwner(collisionInformation.collidingShape));
				fix19_13 elasticity = __VIRTUAL_CALL(SpatialObject, getElasticity, Shape_getOwner(collisionInformation.collidingShape));

				Body_bounce(this->body, collisionInformation.collisionSolution.collisionPlaneNormal, frictionCoefficient, elasticity);
				returnValue = true;
			}
			else
			{
				Body_stopMovement(this->body, __ALL_AXES);
			}
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
bool SolidParticle_canMoveTowards(SolidParticle this, Vector3D direction)
{
	ASSERT(this, "Particle::canMoveTowards: null this");

	if(CollisionSolution_hasCollidingShapes(this->collisionSolver))
	{
		fix19_13 collisionCheckDistance = __I_TO_FIX19_13(1);

		Vector3D displacement =
		{
			direction.x ? 0 < direction.x ? collisionCheckDistance : -collisionCheckDistance : 0,
			direction.y ? 0 < direction.y ? collisionCheckDistance : -collisionCheckDistance : 0,
			direction.z ? 0 < direction.z ? collisionCheckDistance : -collisionCheckDistance : 0
		};

		bool canMove = true;

		VirtualList collisionSolutionsList = CollisionSolver_testForCollisions(this->collisionSolver, displacement, 0, this->shape);

		if(collisionSolutionsList)
		{
			VirtualNode collisionSolutionNode = collisionSolutionsList->head;

			for(; collisionSolutionNode; collisionSolutionNode = collisionSolutionNode->next)
			{
				CollisionSolution* collisionSolution = (CollisionSolution*)collisionSolutionNode->data;

				if(canMove)
				{
					canMove &= __I_TO_FIX19_13(1) != __ABS(Vector3D_dotProduct(collisionSolution->collisionPlaneNormal, Vector3D_normalize(displacement)));
				}

				__DELETE_BASIC(collisionSolution);
			}

			__DELETE(collisionSolutionsList);
		}

		return canMove;
	}

	return true;
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

			if(!Body_getMovementOnAllAxes(this->body))
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

	SolidParticle_transformShape(this);
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

