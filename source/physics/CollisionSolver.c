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

#include <CollisionSolver.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(CollisionSolver, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
const extern VBVec3D* _screenDisplacement;

static void CollisionSolver_collidingSpatialObjectDestroyed(CollisionSolver this, Object eventFirer);
static void CollisionSolver_alignToCollidingSpatialObject(CollisionSolver this, SpatialObject collidingSpatialObject, int axisOfCollision, const Scale* scale);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionSolver, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite)
__CLASS_NEW_END(CollisionSolver, owner, ownerPositionToCheck, ownerPositionToWrite);

// class's constructor
void CollisionSolver_constructor(CollisionSolver this, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite)
{
	ASSERT(this, "CollisionSolver::constructor: null this");
	ASSERT(owner, "CollisionSolver::constructor: null owner");

	// construct base object
	__CONSTRUCT_BASE();

	this->owner = owner;
	this->ownerPositionToCheck = ownerPositionToCheck;
	this->ownerPositionToWrite = ownerPositionToWrite;

	this->sensibleToFriction.x = true;
	this->sensibleToFriction.y = true;
	this->sensibleToFriction.z = true;

	this->lastCollidingSpatialObject[kXAxis] = __NEW(VirtualList);
	this->lastCollidingSpatialObject[kYAxis] = __NEW(VirtualList);
	this->lastCollidingSpatialObject[kZAxis] = __NEW(VirtualList);
}

// class's destructor
void CollisionSolver_destructor(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::destructor: null this");

	CollisionSolver_resetCollisionStatusOnAxis(this, __XAXIS | __YAXIS | __ZAXIS);
	
	__DELETE(this->lastCollidingSpatialObject[kXAxis]);
	__DELETE(this->lastCollidingSpatialObject[kYAxis]);
	__DELETE(this->lastCollidingSpatialObject[kZAxis]);
	
	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// update colliding entities
void CollisionSolver_resetCollisionStatusOnAxis(CollisionSolver this, u8 movementAxis)
{
	ASSERT(this, "CollisionSolver::updateCollisionStatus: null this");

	int i = 0;
	for(; i < kLastAxis; i++)
	{
		if((kXAxis == i && (__XAXIS & movementAxis)) ||
			(kYAxis == i && (__YAXIS & movementAxis)) ||
			(kZAxis == i && (__ZAXIS & movementAxis))
		)
		{
			VirtualNode node = this->lastCollidingSpatialObject[i]->head;
			for(; node; node = node->next)
	        {
				Object_removeEventListener(__SAFE_CAST(Object, node->data), __SAFE_CAST(Object, this), (void (*)(Object, Object))CollisionSolver_collidingSpatialObjectDestroyed, __EVENT_OBJECT_DESTROYED);
	        }
	
			VirtualList_clear(this->lastCollidingSpatialObject[i]);
		}
	}
}

// retrieve previous position
const VBVec3D* CollisionSolver_getOwnerPreviousPosition(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::getOwnerPreviousPosition: null this");
	
	return &this->ownerPreviousPosition;
}

// retrieve previous position
void CollisionSolver_setOwnerPreviousPosition(CollisionSolver this, VBVec3D position)
{
	ASSERT(this, "CollisionSolver::setOwnerPreviousPosition: null this");
	
	this->ownerPreviousPosition = position;
}


// check if gravity must apply to this actor
u8 CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape)
{
	ASSERT(this, "CollisionSolver::canMoveOverAxis: null this");

	u8 axisOfCollision = 0;

	VBVec3D displacement =
    {
    	acceleration->x ? 0 < acceleration->x? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0,
		acceleration->y ? 0 < acceleration->y? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0,
		acceleration->z ? 0 < acceleration->z? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0
	};


	int i = 0;
	for(; i < kLastAxis; i++)
	{
		if((kXAxis == i && (displacement.x)) ||
			(kYAxis == i && (displacement.y)) ||
			(kZAxis == i && (displacement.z))
		)
		{
			VirtualNode node = this->lastCollidingSpatialObject[i]->head;
			for(; node; node = node->next)
	        {
				axisOfCollision |= __VIRTUAL_CALL(bool, Shape, testIfCollision, shape, __SAFE_CAST(SpatialObject, node->data), displacement, this->ownerPreviousPosition);
	        }
		}
	}
	
	return axisOfCollision;
}

// process event
static void CollisionSolver_collidingSpatialObjectDestroyed(CollisionSolver this, Object eventFirer)
{
	ASSERT(this, "CollisionSolver::collidingSpatialObjectDestroyed: null this");

	VirtualList_removeElement(this->lastCollidingSpatialObject[kXAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingSpatialObject[kYAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingSpatialObject[kZAxis], eventFirer);
}


// align to colliding spatialObject
static void CollisionSolver_alignToCollidingSpatialObject(CollisionSolver this, SpatialObject collidingSpatialObject, int axisOfCollision, const Scale* scale)
{
	ASSERT(this, "CollisionSolver::alignToCollidingSpatialObject: null this");
	ASSERT(scale->y, "CollisionSolver::alignToCollidingSpatialObject: 0 scale y");

	/*
	int alignThreshold = FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(1), scale->y));

	if(1 > alignThreshold)
	{
		alignThreshold = 1;
	}
	*/
	int alignThreshold = 1;


	if(__XAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __XAXIS, alignThreshold);
		VirtualList_pushBack(this->lastCollidingSpatialObject[kXAxis], collidingSpatialObject);
	}

	if(__YAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __YAXIS, alignThreshold);
		VirtualList_pushBack(this->lastCollidingSpatialObject[kYAxis], collidingSpatialObject);
	}

	if(__ZAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __ZAXIS, alignThreshold);
		VirtualList_pushBack(this->lastCollidingSpatialObject[kZAxis], collidingSpatialObject);
	}	
	
	Object_addEventListener(__SAFE_CAST(Object, collidingSpatialObject), __SAFE_CAST(Object, this), (void (*)(Object, Object))CollisionSolver_collidingSpatialObjectDestroyed, __EVENT_OBJECT_DESTROYED);
}

// resolve collision against other entities
u8 CollisionSolver_resolveCollision(CollisionSolver this, VirtualList collidingSpatialObjects, u8 movementAxis, VBVec3D displacement, const Scale* scale)
{
	ASSERT(this, "CollisionSolver::resolveCollision: null this");
	ASSERT(collidingSpatialObjects, "CollisionSolver::resolveCollision: collidingEntities");

	CollisionSolver_resetCollisionStatusOnAxis(this, movementAxis);

	u8 axisOfCollision = 0;

	VirtualNode node = collidingSpatialObjects->head;

	SpatialObject collidingSpatialObject = NULL;

	// TODO: solve when more than one spatialObject has been touched
	for(; node && !axisOfCollision; node = node->next)
	{
		collidingSpatialObject = node->data;
		axisOfCollision = __VIRTUAL_CALL(int, Shape, getAxisOfCollision, __VIRTUAL_CALL_UNSAFE(Shape, SpatialObject, getShape, this->owner), collidingSpatialObject, displacement, this->ownerPreviousPosition);
		CollisionSolver_alignToCollidingSpatialObject(this, collidingSpatialObject, axisOfCollision, scale);
	}
	
	return axisOfCollision;
}

// align character to other spatialObject on collision
void CollisionSolver_alignTo(CollisionSolver this, SpatialObject collidingSpatialObject, int axis, int pad)
{
	ASSERT(this, "CollisionSolver::alignTo: null this");

	// retrieve the colliding spatialObject's position and gap
	Gap myOwnerGap = __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, this->owner);
	VBVec3D myOwnerPosition = *this->ownerPositionToWrite;
	const VBVec3D* otherPosition = __VIRTUAL_CALL_UNSAFE(const VBVec3D*, SpatialObject, getPosition, collidingSpatialObject);
	Gap otherGap = __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, collidingSpatialObject);

	// pointers to the dimensions to affect
	const fix19_13* myPositionAxisToCheck = NULL;
	fix19_13* myPositionAxis = NULL;
	const fix19_13* otherPositionAxis = NULL;

	// used to the width, height or depth
	u16 myHalfSize = 0;
	u16 otherHalfSize = 0;

	// gap to use based on the axis
	int otherLowGap = 0;
	int otherHighGap = 0;
	int myLowGap = 0;
	int myHighGap = 0;

	// calculate gap again (scale, etc)
	//__VIRTUAL_CALL(void, SpatialObject, setGap, this->owner);

	// select the axis to affect
	switch(axis)
	{
		case __XAXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->x;
			myPositionAxis = &myOwnerPosition.x;
			otherPositionAxis = &otherPosition->x;

			myHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getWidth, this->owner) >> 1;
			otherHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getWidth, collidingSpatialObject) >> 1;

			otherLowGap = otherGap.left;
			otherHighGap = otherGap.right;
			myLowGap = myOwnerGap.left;
			myHighGap = myOwnerGap.right;
			break;

		case __YAXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->y;
			myPositionAxis = &myOwnerPosition.y;
			otherPositionAxis = &otherPosition->y;

			myHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getHeight, this->owner) >> 1;
			otherHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getHeight, collidingSpatialObject) >> 1;

			otherLowGap = otherGap.up;
			otherHighGap = otherGap.down;
			myLowGap = myOwnerGap.up;
			myHighGap = myOwnerGap.down;
			break;

		case __ZAXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->z;
			myPositionAxis = &myOwnerPosition.z;
			otherPositionAxis = &otherPosition->z;

			// TODO: must make depth work as the width and high
			if(this->ownerPositionToCheck->z < otherPosition->z)
			{
				myHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getDepth, this->owner);
				otherHalfSize = 0;
			}
			else
			{
				myHalfSize = 0;
				otherHalfSize = __VIRTUAL_CALL(u16, SpatialObject, getDepth, collidingSpatialObject);
			}
			
			myLowGap = 0;
			myHighGap = 0;

			break;
	}

	// decide to which side of the spatialObject align myself
	if(*myPositionAxisToCheck > *otherPositionAxis)
    {
        // pad -= (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1) ? 1 : 0);
		// align right / below / behind
		*myPositionAxis = *otherPositionAxis +
							ITOFIX19_13(otherHalfSize - otherHighGap
							+ myHalfSize - myLowGap
							+ pad);
	}
	else
	{
		// align left / over / in front
		*myPositionAxis = *otherPositionAxis -
							ITOFIX19_13(otherHalfSize - otherLowGap
							+ myHalfSize - myHighGap
							+ pad);
	}

	__VIRTUAL_CALL(void, SpatialObject, setPosition, this->owner, &myOwnerPosition);

	// save owner's new position
	this->ownerPreviousPosition = *__VIRTUAL_CALL_UNSAFE(const VBVec3D*, SpatialObject, getPosition, this->owner);

}

// retrieve friction of colliding objects
Force CollisionSolver_getSourroundingFriction(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::updateSourroundingFriction: null this");

	Force friction = 
	{
		0, 0, 0
	};

	if(this->sensibleToFriction.x)
	{
		// get friction in y axis
		VirtualNode node = this->lastCollidingSpatialObject[kYAxis]->head;
		for(; node; node = node->next)
		{
			friction.x += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in z axis
		node = this->lastCollidingSpatialObject[kZAxis]->head;
		for(; node; node = node->next)
		{
			friction.x += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	if(this->sensibleToFriction.y)
	{
		// get friction in x axis
		VirtualNode node = this->lastCollidingSpatialObject[kXAxis]->head;
		for(; node; node = node->next)
		{
			friction.y += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in z axis
		node = this->lastCollidingSpatialObject[kZAxis]->head;
		for(; node; node = node->next)
		{
			friction.y += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	if(this->sensibleToFriction.z)
	{
		// get friction in x axis
		VirtualNode node = this->lastCollidingSpatialObject[kXAxis]->head;
		for(; node; node = node->next)
		{
			friction.z += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in y axis
		node = this->lastCollidingSpatialObject[kYAxis]->head;
		for(; node; node = node->next)
		{
			friction.z += __VIRTUAL_CALL(fix19_13, SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	return friction;
}

fix19_13 CollisionSolver_getCollisingSpatialObjectsTotalElasticity(CollisionSolver this, u8 axis)
{
	ASSERT(this, "CollisionSolver::getCollisingSpatialObjectsTotalElasticity: null this");
	ASSERT(!((__XAXIS & axis) && (__YAXIS & axis)), "CollisionSolver::getCollisingSpatialObjectsTotalElasticity: more than one axis x, y");
	ASSERT(!((__XAXIS & axis) && (__ZAXIS & axis)), "CollisionSolver::getCollisingSpatialObjectsTotalElasticity: more than one axis x, z");
	ASSERT(!((__YAXIS & axis) && (__ZAXIS & axis)), "CollisionSolver::getCollisingSpatialObjectsTotalElasticity: more than one axis y, z");

	fix19_13 totalElasticiy = 0;
	int collidingSpatialObjectListIndex = -1;
	
	if(__XAXIS & axis)
    {
		collidingSpatialObjectListIndex = kXAxis;
    }

	if(__YAXIS & axis)
    {
		collidingSpatialObjectListIndex = kYAxis;
    }

	if(__ZAXIS & axis)
    {
		collidingSpatialObjectListIndex = kZAxis;
    }

	VirtualNode node = this->lastCollidingSpatialObject[collidingSpatialObjectListIndex]->head;

	for(; 0 <= collidingSpatialObjectListIndex && node; node = node->next)
	{
		totalElasticiy += __VIRTUAL_CALL(fix19_13, SpatialObject, getElasticity, __SAFE_CAST(SpatialObject, node->data));
	}
	
	return totalElasticiy;
}