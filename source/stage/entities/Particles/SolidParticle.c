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
 */
__CLASS_DEFINITION(SolidParticle, Particle);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


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

	this->solidParticleDefinition = solidParticleDefinition;
	Body_setElasticity(this->body, solidParticleDefinition->elasticity);
	Force totalFriction = (Force){solidParticleDefinition->friction, solidParticleDefinition->friction, solidParticleDefinition->friction};
	Body_setFriction(this->body, totalFriction);

	// register a shape for collision detection
	this->shape = CollisionManager_registerShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), solidParticleDefinition->shapeType);
	__VIRTUAL_CALL(Shape, setup, this->shape);

	this->collisionSolver = __NEW(CollisionSolver, __SAFE_CAST(SpatialObject, this), &this->position, &this->position);
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
	CollisionManager_unregisterShape(Game_getCollisionManager(Game_getInstance()), this->shape);

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

	u32 expired = Particle_update(__SAFE_CAST(Particle, this), timeElapsed, behavior);

	if(0 <= this->lifeSpan)
	{
		this->position = *Body_getPosition(this->body);

		if(this->collisionSolver)
		{
			CollisionSolver_setOwnerPreviousPosition(this->collisionSolver, this->position);
		}
	}

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
int SolidParticle_getWidth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getWidth: null this");

	return this->solidParticleDefinition->width;
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
int SolidParticle_getHeight(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getHeight: null this");

	return this->solidParticleDefinition->height;
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
int SolidParticle_getDepth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getDepth: null this");

	// must calculate based on the scale because not affine object must be enlarged
	return this->solidParticleDefinition->depth;
}

/**
 * Retrieve friction of colliding objects
 *
 * @memberof	SolidParticle
 * @private
 *
 * @param this	Function scope
 */
static void SolidParticle_updateSurroundingFriction(SolidParticle this)
{
	ASSERT(this, "SolidParticle::updateSurroundingFriction: null this");
	ASSERT(this->body, "SolidParticle::updateSurroundingFriction: null body");

	Force totalFriction = {0, 0, 0};

	if(this->collisionSolver)
	{
		Force surroundingFriction = CollisionSolver_getSurroundingFriction(this->collisionSolver);
		totalFriction.x += surroundingFriction.x;
		totalFriction.y += surroundingFriction.y;
		totalFriction.z += surroundingFriction.z;
	}

	Body_setFriction(this->body, totalFriction);
}

/**
 * Start bouncing after collision with another inGameEntity
 *
 * @memberof				SolidParticle
 * @private
 *
 * @param this				Function scope
 * @param axisOfCollision
 */
static void SolidParticle_checkIfMustBounce(SolidParticle this, u8 axisOfCollision)
{
	ASSERT(this, "SolidParticle::bounce: null this");

	if(axisOfCollision)
	{
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver ? CollisionSolver_getCollidingSpatialObjectsTotalElasticity(this->collisionSolver, axisOfCollision) : __1I_FIX19_13;

		Body_bounce(this->body, axisOfCollision, this->solidParticleDefinition->axisAllowedForBouncing, otherSpatialObjectsElasticity);

		if(!(axisOfCollision & Body_isMoving(this->body)))
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyStopped, &axisOfCollision);
		}
		else
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyBounced, &axisOfCollision);
		}
	}
}

/**
 * Resolve collisions against other entities
 *
 * @memberof						SolidParticle
 * @private
 *
 * @param this						Function scope
 * @param collidingSpatialObjects
 */
static void SolidParticle_resolveCollision(SolidParticle this, VirtualList collidingSpatialObjects)
{
	ASSERT(this, "SolidParticle::resolveCollision: null this");
	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collidingSpatialObjects, "SolidParticle::resolveCollision: collidingSpatialObjects");

	if(this->collisionSolver)
	{
		if(this->solidParticleDefinition->ignoreParticles)
		{
			VirtualList collidingObjectsToRemove = __NEW(VirtualList);
			VirtualNode node = NULL;

			for(node = collidingSpatialObjects->head; node; node = node->next)
			{
				SpatialObject spatialObject = __SAFE_CAST(SpatialObject, node->data);

				if(__GET_CAST(Particle, spatialObject))
				{
					VirtualList_pushBack(collidingObjectsToRemove, spatialObject);
				}
			}

			for(node = collidingObjectsToRemove->head; node; node = node->next)
			{
				// whenever you process some objects of a collisions list remove them and leave the Actor handle
				// the ones you don't care about, i.e.: in most cases, the ones which are solid
				VirtualList_removeElement(collidingSpatialObjects, node->data);
			}

			__DELETE(collidingObjectsToRemove);
		}

		u8 axisOfAllignement = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, Body_getLastDisplacement(this->body), false);

		SolidParticle_checkIfMustBounce(this, axisOfAllignement);

		SolidParticle_updateSurroundingFriction(this);
	}
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
		case kCollision:

			SolidParticle_resolveCollision(this, __SAFE_CAST(VirtualList, Telegram_getExtraInfo(telegram)));
			return true;
			break;

		case kBodyStartedMoving:

			CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
			CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, *(u8*)Telegram_getExtraInfo(telegram));
			return true;
			break;

		case kBodyStopped:

			if(!Body_isMoving(this->body))
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
void SolidParticle_setPosition(SolidParticle this, const VBVec3D* position)
{
	ASSERT(this, "SolidParticle::position: null this");

	Particle_setPosition(__SAFE_CAST(Particle, this), position);
	CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, __XAXIS | __YAXIS | __ZAXIS);

	this->position = *position;
}
