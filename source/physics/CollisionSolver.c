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
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											   	MACROS
//---------------------------------------------------------------------------------------------------------

#define __ALIGN_PADDING		1

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
extern const VBVec3D* _screenDisplacement;

static void CollisionSolver_onCollidingSpatialObjectDestroyed(CollisionSolver this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionSolver, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite)
__CLASS_NEW_END(CollisionSolver, owner, ownerPositionToCheck, ownerPositionToWrite);

// class's constructor
// must always pass the global position
void CollisionSolver_constructor(CollisionSolver this, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite)
{
	ASSERT(this, "CollisionSolver::constructor: null this");
	ASSERT(owner, "CollisionSolver::constructor: null owner");

	// construct base object
	__CONSTRUCT_BASE(Object);

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
void CollisionSolver_resetCollisionStatusOnAxis(CollisionSolver this, int movementAxis)
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
				Object_removeEventListener(__SAFE_CAST(Object, node->data), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingSpatialObjectDestroyed, __EVENT_SPATIAL_OBJECT_DELETED);
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
int CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape)
{
	ASSERT(this, "CollisionSolver::canMoveOverAxis: null this");

	int axisOfCollision = 0;
	int collisionCheckDistance = 5;

	VBVec3D displacement =
    {
    	acceleration->x ? 0 < acceleration->x? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance): ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance): 0,
		acceleration->y ? 0 < acceleration->y? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance): ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance): 0,
		acceleration->z ? 0 < acceleration->z? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance): ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance): 0
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
				axisOfCollision |= __VIRTUAL_CALL(Shape, testIfCollision, shape, __SAFE_CAST(SpatialObject, node->data), displacement);
	        }
		}
	}

	return axisOfCollision;
}

// process event
static void CollisionSolver_onCollidingSpatialObjectDestroyed(CollisionSolver this, Object eventFirer)
{
	ASSERT(this, "CollisionSolver::collidingSpatialObjectDestroyed: null this");

	VirtualList_removeElement(this->lastCollidingSpatialObject[kXAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingSpatialObject[kYAxis], eventFirer);
	VirtualList_removeElement(this->lastCollidingSpatialObject[kZAxis], eventFirer);
}

// align to colliding spatialObject
void CollisionSolver_alignToCollidingSpatialObject(CollisionSolver this, SpatialObject collidingSpatialObject, int axisOfCollision, bool registerObject)
{
	ASSERT(this, "CollisionSolver::alignToCollidingSpatialObject: null this");

	int alignThreshold = __ALIGN_PADDING;

	if(__XAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __XAXIS, alignThreshold);

		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingSpatialObject[kXAxis], collidingSpatialObject);
		}
	}

	if(__YAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __YAXIS, alignThreshold);
		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingSpatialObject[kYAxis], collidingSpatialObject);
		}
	}

	if(__ZAXIS & axisOfCollision)
    {
		CollisionSolver_alignTo(this, collidingSpatialObject, __ZAXIS, alignThreshold);
		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingSpatialObject[kZAxis], collidingSpatialObject);
		}
	}

	Object_addEventListener(__SAFE_CAST(Object, collidingSpatialObject), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingSpatialObjectDestroyed, __EVENT_SPATIAL_OBJECT_DELETED);
}

// get axis of collision
int CollisionSolver_getAxisOfCollision(CollisionSolver this, SpatialObject collidingSpatialObject, VBVec3D displacement)
{
	ASSERT(this, "CollisionSolver::getAxisOfCollision: null this");
	ASSERT(collidingSpatialObject, "CollisionSolver::getAxisOfCollision: collidingEntities");

	VBVec3D ownerPreviousPosition = this->ownerPreviousPosition;

    if(!displacement.x && !displacement.y && !displacement.z)
    {
        return 0;
    }

    return __VIRTUAL_CALL(Shape, getAxisOfCollision, __VIRTUAL_CALL(SpatialObject, getShape, this->owner), collidingSpatialObject, displacement, ownerPreviousPosition);
}

// resolve collision against other entities
int CollisionSolver_resolveCollision(CollisionSolver this, VirtualList collidingSpatialObjects, VBVec3D displacement, bool registerObjects)
{
	ASSERT(this, "CollisionSolver::resolveCollision: null this");
	ASSERT(collidingSpatialObjects, "CollisionSolver::resolveCollision: collidingEntities");

	int axisOfCollision = 0;

	VirtualNode node = collidingSpatialObjects->head;

	VirtualList processedCollidingSpatialObjects = __NEW(VirtualList);

	VBVec3D ownerPreviousPosition = this->ownerPreviousPosition;

	SpatialObject collidingSpatialObjectsToAlignTo[kLastAxis] = {NULL, NULL, NULL};

	for(; node; node = node->next)
	{
		SpatialObject collidingSpatialObject = __SAFE_CAST(SpatialObject, node->data);

		axisOfCollision = __VIRTUAL_CALL(Shape, getAxisOfCollision, __VIRTUAL_CALL(SpatialObject, getShape, this->owner), collidingSpatialObject, displacement, ownerPreviousPosition);

		if(axisOfCollision)
		{
			const VBVec3D* collidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingSpatialObject);
			int collidingSpatialObjectPositionHalfWidth = __VIRTUAL_CALL(SpatialObject, getWidth, collidingSpatialObject) >> 1;
			int collidingSpatialObjectPositionHalfHeight = __VIRTUAL_CALL(SpatialObject, getHeight, collidingSpatialObject) >> 1;
			int collidingSpatialObjectPositionHalfDepth = __VIRTUAL_CALL(SpatialObject, getDepth, collidingSpatialObject) >> 1;

			if(__XAXIS & axisOfCollision)
			{
				if(collidingSpatialObjectsToAlignTo[kXAxis])
				{
					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingSpatialObjectsToAlignTo[kXAxis]);
					int selectedCollidingSpatialObjectPositionHalfWidth = __VIRTUAL_CALL(SpatialObject, getWidth, collidingSpatialObjectsToAlignTo[kXAxis]) >> 1;

					if(0 < displacement.x)
					{
						if(collidingSpatialObjectPosition->x - collidingSpatialObjectPositionHalfWidth < selectedCollidingSpatialObjectPosition->x - selectedCollidingSpatialObjectPositionHalfWidth)
						{
							collidingSpatialObjectsToAlignTo[kXAxis] = collidingSpatialObject;
						}
					}
					else
					{
						if(collidingSpatialObjectPosition->x + collidingSpatialObjectPositionHalfWidth > selectedCollidingSpatialObjectPosition->x + selectedCollidingSpatialObjectPositionHalfWidth)
						{
							collidingSpatialObjectsToAlignTo[kXAxis] = collidingSpatialObject;
						}
					}
				}
				else
				{
					collidingSpatialObjectsToAlignTo[kXAxis] = collidingSpatialObject;
				}
			}

			if(__YAXIS & axisOfCollision)
			{
				if(collidingSpatialObjectsToAlignTo[kYAxis])
				{
					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingSpatialObjectsToAlignTo[kYAxis]);
					int selectedCollidingSpatialObjectPositionHalfHeight = __VIRTUAL_CALL(SpatialObject, getHeight, collidingSpatialObjectsToAlignTo[kYAxis]) >> 1;

					if(0 < displacement.y)
					{
						if(collidingSpatialObjectPosition->y - collidingSpatialObjectPositionHalfHeight < selectedCollidingSpatialObjectPosition->y - selectedCollidingSpatialObjectPositionHalfHeight)
						{
							collidingSpatialObjectsToAlignTo[kYAxis] = collidingSpatialObject;
						}
					}
					else
					{
						if(collidingSpatialObjectPosition->y + collidingSpatialObjectPositionHalfHeight > selectedCollidingSpatialObjectPosition->y + selectedCollidingSpatialObjectPositionHalfHeight)
						{
							collidingSpatialObjectsToAlignTo[kYAxis] = collidingSpatialObject;
						}
					}
				}
				else
				{
					collidingSpatialObjectsToAlignTo[kYAxis] = collidingSpatialObject;
				}
			}

			if(__ZAXIS & axisOfCollision)
			{
				if(collidingSpatialObjectsToAlignTo[kZAxis])
				{
					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingSpatialObjectsToAlignTo[kZAxis]);
					int selectedCollidingSpatialObjectPositionHalfDepth = __VIRTUAL_CALL(SpatialObject, getDepth, collidingSpatialObjectsToAlignTo[kZAxis]) >> 1;

					if(0 < displacement.z)
					{
						if(collidingSpatialObjectPosition->z - collidingSpatialObjectPositionHalfDepth < selectedCollidingSpatialObjectPosition->z - selectedCollidingSpatialObjectPositionHalfDepth)
						{
							collidingSpatialObjectsToAlignTo[kZAxis] = collidingSpatialObject;
						}
					}
					else
					{
						if(collidingSpatialObjectPosition->z + collidingSpatialObjectPositionHalfDepth > selectedCollidingSpatialObjectPosition->z + selectedCollidingSpatialObjectPositionHalfDepth)
						{
							collidingSpatialObjectsToAlignTo[kZAxis] = collidingSpatialObject;
						}
					}
				}
				else
				{
					collidingSpatialObjectsToAlignTo[kZAxis] = collidingSpatialObject;
				}
			}
		}
		else
		{
			VirtualList_pushBack(processedCollidingSpatialObjects, collidingSpatialObject);
		}
	}

	if(collidingSpatialObjectsToAlignTo[kXAxis])
	{
		CollisionSolver_alignToCollidingSpatialObject(this, collidingSpatialObjectsToAlignTo[kXAxis], __XAXIS, registerObjects);
	}

	if(collidingSpatialObjectsToAlignTo[kYAxis])
	{
		CollisionSolver_alignToCollidingSpatialObject(this, collidingSpatialObjectsToAlignTo[kYAxis], __YAXIS, registerObjects);
	}

	if(collidingSpatialObjectsToAlignTo[kZAxis])
	{
		CollisionSolver_alignToCollidingSpatialObject(this, collidingSpatialObjectsToAlignTo[kZAxis], __ZAXIS, registerObjects);
	}

	node = processedCollidingSpatialObjects->head;

	for(; node; node = node->next)
	{
		VirtualList_removeElement(collidingSpatialObjects, node->data);
	}

	__DELETE(processedCollidingSpatialObjects);

	return axisOfCollision;
}

// align character to other spatialObject on collision
void CollisionSolver_alignTo(CollisionSolver this, SpatialObject collidingSpatialObject, int axis, int pad)
{
	ASSERT(this, "CollisionSolver::alignTo: null this");

	// retrieve the colliding spatialObject's position and gap
	Gap myOwnerGap = __VIRTUAL_CALL(SpatialObject, getGap, this->owner);
	VBVec3D myOwnerPosition = *this->ownerPositionToWrite;
	const VBVec3D* otherPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingSpatialObject);
	Gap otherGap = __VIRTUAL_CALL(SpatialObject, getGap, collidingSpatialObject);

	// pointers to the dimensions to affect
	const fix19_13* myPositionAxisToCheck = NULL;
	fix19_13* myPositionAxis = NULL;
	const fix19_13* otherPositionAxis = NULL;

	// used to the width, height or depth
	int myHalfSize = 0;
	int otherHalfSize = 0;

	// gap to use based on the axis
	int otherLowGap = 0;
	int otherHighGap = 0;
	int myLowGap = 0;
	int myHighGap = 0;

	// calculate gap again (scale, etc)
	//__VIRTUAL_CALL(SpatialObject, setGap, this->owner);

	// select the axis to affect
	switch(axis)
	{
		case __XAXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->x;
			myPositionAxis = &myOwnerPosition.x;
			otherPositionAxis = &otherPosition->x;

			myHalfSize = __VIRTUAL_CALL(SpatialObject, getWidth, this->owner) >> 1;
			otherHalfSize = __VIRTUAL_CALL(SpatialObject, getWidth, collidingSpatialObject) >> 1;

			otherLowGap = otherGap.left;
			otherHighGap = otherGap.right;
			myLowGap = myOwnerGap.left;
			myHighGap = myOwnerGap.right;
			break;

		case __YAXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->y;
			myPositionAxis = &myOwnerPosition.y;
			otherPositionAxis = &otherPosition->y;

			myHalfSize = __VIRTUAL_CALL(SpatialObject, getHeight, this->owner) >> 1;
			otherHalfSize = __VIRTUAL_CALL(SpatialObject, getHeight, collidingSpatialObject) >> 1;

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
				myHalfSize = __VIRTUAL_CALL(SpatialObject, getDepth, this->owner);
				otherHalfSize = 0;
			}
			else
			{
				myHalfSize = 0;
				otherHalfSize = __VIRTUAL_CALL(SpatialObject, getDepth, collidingSpatialObject);
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

	__VIRTUAL_CALL(SpatialObject, setPosition, this->owner, &myOwnerPosition);

	// save owner's new position
	this->ownerPreviousPosition = *__VIRTUAL_CALL(SpatialObject, getPosition, this->owner);
}

// retrieve friction of colliding objects
Force CollisionSolver_getSurroundingFriction(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::updateSurroundingFriction: null this");

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
			friction.x += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in z axis
		node = this->lastCollidingSpatialObject[kZAxis]->head;
		for(; node; node = node->next)
		{
			friction.x += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	if(this->sensibleToFriction.y)
	{
		// get friction in x axis
		VirtualNode node = this->lastCollidingSpatialObject[kXAxis]->head;
		for(; node; node = node->next)
		{
			friction.y += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in z axis
		node = this->lastCollidingSpatialObject[kZAxis]->head;
		for(; node; node = node->next)
		{
			friction.y += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	if(this->sensibleToFriction.z)
	{
		// get friction in x axis
		VirtualNode node = this->lastCollidingSpatialObject[kXAxis]->head;
		for(; node; node = node->next)
		{
			friction.z += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}

		// get friction in y axis
		node = this->lastCollidingSpatialObject[kYAxis]->head;
		for(; node; node = node->next)
		{
			friction.z += __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		}
	}

	return friction;
}

fix19_13 CollisionSolver_getCollisingSpatialObjectsTotalElasticity(CollisionSolver this, int axis)
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
		totalElasticiy += __VIRTUAL_CALL(SpatialObject, getElasticity, __SAFE_CAST(SpatialObject, node->data));
	}

	return totalElasticiy;
}
