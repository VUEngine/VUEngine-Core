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

#include <CollisionSolver.h>
#include <Shape.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ALIGN_PADDING		1


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	CollisionSolver
 * @extends Object
 * @ingroup physics
 */
__CLASS_DEFINITION(CollisionSolver, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

static void CollisionSolver_onCollidingShapeDestroyed(CollisionSolver this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionSolver, SpatialObject owner)
__CLASS_NEW_END(CollisionSolver, owner);

// class's constructor
// must always pass the global position
void CollisionSolver_constructor(CollisionSolver this, SpatialObject owner)
{
	ASSERT(this, "CollisionSolver::constructor: null this");
	ASSERT(owner, "CollisionSolver::constructor: null owner");

	// construct base object
	__CONSTRUCT_BASE(Object);

	this->owner = owner;

	this->sensibleToFriction.x = true;
	this->sensibleToFriction.y = true;
	this->sensibleToFriction.z = true;

	this->lastCollidingShape[kXAxis] = __NEW(VirtualList);
	this->lastCollidingShape[kYAxis] = __NEW(VirtualList);
	this->lastCollidingShape[kZAxis] = __NEW(VirtualList);
}

// class's destructor
void CollisionSolver_destructor(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::destructor: null this");

	CollisionSolver_resetCollisionStatusOnAxis(this, __X_AXIS | __Y_AXIS | __Z_AXIS);

	__DELETE(this->lastCollidingShape[kXAxis]);
	__DELETE(this->lastCollidingShape[kYAxis]);
	__DELETE(this->lastCollidingShape[kZAxis]);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// update colliding entities
void CollisionSolver_resetCollisionStatusOnAxis(CollisionSolver this, u16 movementAxis)
{
	ASSERT(this, "CollisionSolver::updateCollisionStatus: null this");

	int i = 0;
	for(; i < kLastAxis; i++)
	{
		if((kXAxis == i && (__X_AXIS & movementAxis)) ||
			(kYAxis == i && (__Y_AXIS & movementAxis)) ||
			(kZAxis == i && (__Z_AXIS & movementAxis))
		)
		{
			VirtualNode node = this->lastCollidingShape[i]->head;
			for(; node; node = node->next)
			{
				Object_removeEventListener(__SAFE_CAST(Object, node->data), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingShapeDestroyed, kEventShapeDeleted);
			}

			VirtualList_clear(this->lastCollidingShape[i]);
		}
	}
}

// check if gravity must apply to this actor
u16 CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape)
{
	ASSERT(this, "CollisionSolver::getAxisOfFutureCollision: null this");

	u16 axisOfCollision = 0;
	int collisionCheckDistance = 5;

	VBVec3D displacement =
	{
		acceleration->x ? 0 < acceleration->x ? __I_TO_FIX19_13(__ALIGN_PADDING + collisionCheckDistance) : __I_TO_FIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0,
		acceleration->y ? 0 < acceleration->y ? __I_TO_FIX19_13(__ALIGN_PADDING + collisionCheckDistance) : __I_TO_FIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0,
		acceleration->z ? 0 < acceleration->z ? __I_TO_FIX19_13(__ALIGN_PADDING + collisionCheckDistance) : __I_TO_FIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0
	};

	int i = 0;
	for(; i < kLastAxis; i++)
	{
		if((kXAxis == i && (displacement.x)) ||
			(kYAxis == i && (displacement.y)) ||
			(kZAxis == i && (displacement.z))
		)
		{
			VirtualNode node = this->lastCollidingShape[i]->head;

			for(; node; node = node->next)
			{
				axisOfCollision |= __VIRTUAL_CALL(Shape, testIfCollision, shape, __SAFE_CAST(Shape, node->data), displacement);
			}
		}
	}

	return axisOfCollision;
}

// process event
static void CollisionSolver_onCollidingShapeDestroyed(CollisionSolver this, Object eventFirer)
{
	ASSERT(this, "CollisionSolver::collidingShapeDestroyed: null this");

	VirtualList_removeElement(this->lastCollidingShape[kXAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingShape[kYAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingShape[kZAxis], eventFirer);
}

// resolve collision against other entities
void CollisionSolver_resolveCollision(CollisionSolver this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "CollisionSolver::resolveCollision: null this");
	ASSERT(__SAFE_CAST(Shape, shape), "CollisionSolver::resolveCollision: null shape");
	ASSERT(collisionInformation->collidingShape, "CollisionSolver::resolveCollision: null collidingEntities");

	// retrieve the colliding spatialObject's position and gap
	VBVec3D ownerPosition = *__VIRTUAL_CALL(SpatialObject, getPosition, this->owner);

	VBVec3D minimumTranslationVector = collisionInformation->minimumTranslationVector;

	// if pending SAT check
	if(collisionInformation->pendingSATCheck)
	{
		// force it
		minimumTranslationVector = __VIRTUAL_CALL(Shape, getMinimumOverlappingVector, collisionInformation->shape, collisionInformation->collidingShape);
	}

	ownerPosition.x += minimumTranslationVector.x;
	ownerPosition.y += minimumTranslationVector.y;
	ownerPosition.z += minimumTranslationVector.z;

	__VIRTUAL_CALL(SpatialObject, setPosition, this->owner, &ownerPosition);
}

// retrieve friction of colliding objects
Force CollisionSolver_getSurroundingFriction(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::updateSurroundingFriction: null this");

	Force totalFriction =
	{
		0, 0, 0
	};

	// get friction in x axis
	VirtualNode node = this->lastCollidingShape[kXAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, Shape_getOwner(__SAFE_CAST(Shape, node->data))));
		totalFriction.y += friction;
		totalFriction.z += friction;
	}

	// get friction in y axis
	node = this->lastCollidingShape[kYAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, Shape_getOwner(__SAFE_CAST(Shape, node->data))));
		totalFriction.x += friction;
		totalFriction.z += friction;
	}

	// get friction in z axis
	node = this->lastCollidingShape[kZAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, Shape_getOwner(__SAFE_CAST(Shape, node->data))));
		totalFriction.y += friction;
		totalFriction.x += friction;
	}

	totalFriction.x *= this->sensibleToFriction.x;
	totalFriction.y *= this->sensibleToFriction.y;
	totalFriction.z *= this->sensibleToFriction.z;

	return totalFriction;
}

fix19_13 CollisionSolver_getCollidingTotalElasticity(CollisionSolver this, u16 axis)
{
	ASSERT(this, "CollisionSolver::getCollidingTotalElasticity: null this");
	ASSERT(!((__X_AXIS & axis) && (__Y_AXIS & axis)), "CollisionSolver::getCollidingTotalElasticity: more than one axis x, y");
	ASSERT(!((__X_AXIS & axis) && (__Z_AXIS & axis)), "CollisionSolver::getCollidingTotalElasticity: more than one axis x, z");
	ASSERT(!((__Y_AXIS & axis) && (__Z_AXIS & axis)), "CollisionSolver::getCollidingTotalElasticity: more than one axis y, z");

	fix19_13 totalElasticity = 0;
	int collidingShapesListIndex = -1;

	if(__X_AXIS & axis)
	{
		collidingShapesListIndex = kXAxis;
	}

	if(__Y_AXIS & axis)
	{
		collidingShapesListIndex = kYAxis;
	}

	if(__Z_AXIS & axis)
	{
		collidingShapesListIndex = kZAxis;
	}

	VirtualNode node = this->lastCollidingShape[collidingShapesListIndex]->head;

	for(; 0 <= collidingShapesListIndex && node; node = node->next)
	{
		totalElasticity += __VIRTUAL_CALL(SpatialObject, getElasticity, Shape_getOwner(__SAFE_CAST(Shape, node->data)));
	}

	return totalElasticity;
}
