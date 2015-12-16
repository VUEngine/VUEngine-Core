/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SolidParticle.h>
#include <CollisionManager.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the SolidParticle
__CLASS_DEFINITION(SolidParticle, Particle);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
__CLASS_NEW_END(SolidParticle, solidParticleDefinition, spriteDefinition, lifeSpan, mass);

// class's constructor
void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "SolidParticle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(&solidParticleDefinition->particleDefinition);

	this->solidParticleDefinition = solidParticleDefinition;
	Body_setElasticity(this->body, solidParticleDefinition->elasticity);
	Force totalFriction = (Force){solidParticleDefinition->friction, solidParticleDefinition->friction, solidParticleDefinition->friction};
	Body_setFriction(this->body, totalFriction);

	// register a shape for collision detection
	this->shape = CollisionManager_registerShape(CollisionManager_getInstance(), __SAFE_CAST(SpatialObject, this), solidParticleDefinition->shapeType);
	__VIRTUAL_CALL(void, Shape, setup, this->shape);

	this->collisionSolver = __NEW(CollisionSolver, __SAFE_CAST(SpatialObject, this), &this->position, &this->position);
}

// class's destructor
void SolidParticle_destructor(SolidParticle this)
{
	ASSERT(this, "SolidParticle::destructor: null this");

	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);

	this->shape = NULL;

	if(this->collisionSolver)
	{
		__DELETE(this->collisionSolver);
		this->collisionSolver = NULL;
	}
	
	// destroy the super Container
	__DESTROY_BASE;
}

void SolidParticle_update(SolidParticle this, u16 timeElapsed, void (* behavior)(Particle particle))
{
	ASSERT(this, "SolidParticle::update: null this");

	Particle_update(__SAFE_CAST(Particle, this), timeElapsed, behavior);

	if(0 <= this->lifeSpan)
	{
		this->position = *Body_getPosition(this->body);
	
		if(this->shape)
		{
			__VIRTUAL_CALL(void, Shape, position, this->shape);
		}
		
		if(this->collisionSolver)
		{
			CollisionSolver_setOwnerPreviousPosition(this->collisionSolver, this->position);
		}
	}
}

// retrieve shape
Shape SolidParticle_getShape(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getShape: null this");

	return this->shape;
}

// get width
u16 SolidParticle_getWidth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getWidth: null this");

	return this->solidParticleDefinition->width;
}

// get height
u16 SolidParticle_getHeight(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getHeight: null this");

	return this->solidParticleDefinition->height;
}

// get depth
u16 SolidParticle_getDepth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getDepth: null this");

	// must calculate based on the scale because not affine object must be enlarged
	return this->solidParticleDefinition->depth;
}

// retrieve friction of colliding objects
static void SolidParticle_updateSourroundingFriction(SolidParticle this)
{
	ASSERT(this, "SolidParticle::updateSourroundingFriction: null this");
	ASSERT(this->body, "SolidParticle::updateSourroundingFriction: null body");

	Force totalFriction = {0, 0, 0};
	
	if(this->collisionSolver)
	{
		Force sourroundingFriction = CollisionSolver_getSourroundingFriction(this->collisionSolver);
		totalFriction.x += sourroundingFriction.x;
		totalFriction.y += sourroundingFriction.y;
		totalFriction.z += sourroundingFriction.z;
	}

	Body_setFriction(this->body, totalFriction);
}

// start bouncing after collision with another inGameEntity
static void SolidParticle_checkIfMustBounce(SolidParticle this, u8 axisOfCollision)
{
	ASSERT(this, "SolidParticle::bounce: null this");

	if(axisOfCollision)
	{
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver? CollisionSolver_getCollisingSpatialObjectsTotalElasticity(this->collisionSolver, axisOfCollision): ITOFIX19_13(1);

		Body_bounce(this->body, axisOfCollision, otherSpatialObjectsElasticity);
		
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

// resolve collision against other entities
static void SolidParticle_resolveCollision(SolidParticle this, VirtualList collidingSpatialObjects)
{
	ASSERT(this, "SolidParticle::resolveCollision: null this");
	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collidingSpatialObjects, "SolidParticle::resolveCollision: collidingSpatialObjects");

	if(this->collisionSolver)
	{
		static Scale scale = 
		{
				ITOFIX7_9(1), ITOFIX7_9(1)
		};
		
		if(this->solidParticleDefinition->ignoreParticles)
		{
			VirtualList collidingObjectsToRemove = __NEW(VirtualList);
			VirtualNode node = NULL;
	
			for(node = VirtualList_begin(collidingSpatialObjects); node; node = VirtualNode_getNext(node))
		    {
				SpatialObject spatialObject = __SAFE_CAST(SpatialObject, VirtualNode_getData(node));
				
				if(__GET_CAST(Particle, spatialObject))
				{
					VirtualList_pushBack(collidingObjectsToRemove, spatialObject);
				}
			}
	
			for(node = VirtualList_begin(collidingObjectsToRemove); node; node = VirtualNode_getNext(node))
		    {
				// whenever you process some objects of a collisions list remove them and leave the Actor handle
				// the ones you don't care about, i.e.: in most cases, the ones which are solid
				VirtualList_removeElement(collidingSpatialObjects, VirtualNode_getData(node));
			}
			
			__DELETE(collidingObjectsToRemove);
		}
		
		u8 axisOfAllignement = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, Body_isMoving(this->body), Body_getLastDisplacement(this->body), &scale);

		SolidParticle_checkIfMustBounce(this, axisOfAllignement);
		
		SolidParticle_updateSourroundingFriction(this);
	}
}

// process a telegram
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

			CollisionManager_shapeStartedMoving(CollisionManager_getInstance(), this->shape);
			CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, *(u8*)Telegram_getExtraInfo(telegram));
			return true;
			break;

		case kBodyStopped:

			if(!Body_isMoving(this->body))
            {
				//CollisionManager_shapeStoppedMoving(CollisionManager_getInstance(), this->shape);
			}
			break;

		case kBodyBounced:

			return true;
			break;
	}

	return false;
}

void SolidParticle_setPosition(SolidParticle this, const VBVec3D* position)
{
	ASSERT(this, "SolidParticle::position: null this");
	
	Particle_setPosition(__SAFE_CAST(Particle, this), position);
	CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, __XAXIS | __YAXIS | __ZAXIS);

	this->position = *position;
}
