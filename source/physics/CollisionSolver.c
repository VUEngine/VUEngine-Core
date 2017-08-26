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
				Object_removeEventListener(__SAFE_CAST(Object, node->data), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingShapeDestroyed, kEventSpatialObjectDeleted);
			}

			VirtualList_clear(this->lastCollidingShape[i]);
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
u16 CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape)
{
	ASSERT(this, "CollisionSolver::getAxisOfFutureCollision: null this");

	u16 axisOfCollision = 0;
	int collisionCheckDistance = 5;

	VBVec3D displacement =
	{
		acceleration->x ? 0 < acceleration->x ? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance) : ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0,
		acceleration->y ? 0 < acceleration->y ? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance) : ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0,
		acceleration->z ? 0 < acceleration->z ? ITOFIX19_13(__ALIGN_PADDING + collisionCheckDistance) : ITOFIX19_13(-__ALIGN_PADDING - collisionCheckDistance) : 0
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

// align to colliding spatialObject
void CollisionSolver_alignToCollidingShape(CollisionSolver this, Shape collidingShape, u16 axisOfCollision, bool registerObject)
{
	ASSERT(this, "CollisionSolver::alignToCollidingShape: null this");

	int alignThreshold = __ALIGN_PADDING;

	if(__X_AXIS & axisOfCollision)
	{
		CollisionSolver_alignTo(this, collidingShape, __X_AXIS, alignThreshold);

		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingShape[kXAxis], collidingShape);
		}
	}

	if(__Y_AXIS & axisOfCollision)
	{
		CollisionSolver_alignTo(this, collidingShape, __Y_AXIS, alignThreshold);
		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingShape[kYAxis], collidingShape);
		}
	}

	if(__Z_AXIS & axisOfCollision)
	{
		CollisionSolver_alignTo(this, collidingShape, __Z_AXIS, alignThreshold);
		if(registerObject)
		{
			VirtualList_pushBack(this->lastCollidingShape[kZAxis], collidingShape);
		}
	}

	Object_addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingShapeDestroyed, kEventSpatialObjectDeleted);
}

// get axis of collision
u16 CollisionSolver_getAxisOfCollision(CollisionSolver this, Shape shape, Shape collidingShape, VBVec3D displacement)
{
	ASSERT(this, "CollisionSolver::getAxisOfCollision: null this");
	ASSERT(collidingShape, "CollisionSolver::getAxisOfCollision: collidingEntities");

	VBVec3D ownerPreviousPosition = this->ownerPreviousPosition;

	if(!displacement.x && !displacement.y && !displacement.z)
	{
		return 0;
	}

	return __VIRTUAL_CALL(Shape, getAxisOfCollision, shape, collidingShape, displacement, ownerPreviousPosition);
}

// resolve collision against other entities
int CollisionSolver_resolveCollision(CollisionSolver this, Shape shape, VirtualList collidingShapes, VBVec3D displacement, bool registerObjects)
{
	ASSERT(this, "CollisionSolver::resolveCollision: null this");
	ASSERT(collidingShapes, "CollisionSolver::resolveCollision: collidingEntities");

	int axisOfCollision = 0;

	VirtualNode node = collidingShapes->head;

	static VirtualList processedCollidingSpatialObjects = NULL;

	if(!processedCollidingSpatialObjects)
	{
		processedCollidingSpatialObjects = __NEW(VirtualList);
	}

	VBVec3D ownerPreviousPosition = this->ownerPreviousPosition;

	Shape collidingShapesToAlignTo[kLastAxis] = {NULL, NULL, NULL};

	for(; node; node = node->next)
	{
		Shape collidingShape = __SAFE_CAST(Shape, node->data);

		axisOfCollision = __VIRTUAL_CALL(Shape, getAxisOfCollision, shape, collidingShape, displacement, ownerPreviousPosition);

		if(axisOfCollision)
		{
			if(__X_AXIS & axisOfCollision)
			{
				if(collidingShapesToAlignTo[kXAxis])
				{
					const VBVec3D* collidingShapePosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShape);
					int collidingShapePositionHalfWidth = (int)(__VIRTUAL_CALL(SpatialObject, getWidth, collidingShape) >> 1);

					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShapesToAlignTo[kXAxis]);
					int selectedCollidingSpatialObjectPositionHalfWidth = (int)(__VIRTUAL_CALL(SpatialObject, getWidth, collidingShapesToAlignTo[kXAxis]) >> 1);

					if(0 < displacement.x)
					{
						if(collidingShapePosition->x - collidingShapePositionHalfWidth < selectedCollidingSpatialObjectPosition->x - selectedCollidingSpatialObjectPositionHalfWidth)
						{
							collidingShapesToAlignTo[kXAxis] = collidingShape;
						}
					}
					else
					{
						if(collidingShapePosition->x + collidingShapePositionHalfWidth > selectedCollidingSpatialObjectPosition->x + selectedCollidingSpatialObjectPositionHalfWidth)
						{
							collidingShapesToAlignTo[kXAxis] = collidingShape;
						}
					}
				}
				else
				{
					collidingShapesToAlignTo[kXAxis] = collidingShape;
				}
			}

			if(__Y_AXIS & axisOfCollision)
			{
				if(collidingShapesToAlignTo[kYAxis])
				{
					const VBVec3D* collidingShapePosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShape);
					int collidingShapePositionHalfHeight = (int)(__VIRTUAL_CALL(SpatialObject, getHeight, collidingShape) >> 1);

					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShapesToAlignTo[kYAxis]);
					int selectedCollidingSpatialObjectPositionHalfHeight = (int)(__VIRTUAL_CALL(SpatialObject, getHeight, collidingShapesToAlignTo[kYAxis]) >> 1);

					if(0 < displacement.y)
					{
						if(collidingShapePosition->y - collidingShapePositionHalfHeight < selectedCollidingSpatialObjectPosition->y - selectedCollidingSpatialObjectPositionHalfHeight)
						{
							collidingShapesToAlignTo[kYAxis] = collidingShape;
						}
					}
					else
					{
						if(collidingShapePosition->y + collidingShapePositionHalfHeight > selectedCollidingSpatialObjectPosition->y + selectedCollidingSpatialObjectPositionHalfHeight)
						{
							collidingShapesToAlignTo[kYAxis] = collidingShape;
						}
					}
				}
				else
				{
					collidingShapesToAlignTo[kYAxis] = collidingShape;
				}
			}

			if(__Z_AXIS & axisOfCollision)
			{
				if(collidingShapesToAlignTo[kZAxis])
				{
					const VBVec3D* collidingShapePosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShape);
					int collidingShapePositionHalfDepth = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, collidingShape) >> 1);

					const VBVec3D* selectedCollidingSpatialObjectPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShapesToAlignTo[kZAxis]);
					int selectedCollidingSpatialObjectPositionHalfDepth = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, collidingShapesToAlignTo[kZAxis]) >> 1);

					if(0 < displacement.z)
					{
						if(collidingShapePosition->z - collidingShapePositionHalfDepth < selectedCollidingSpatialObjectPosition->z - selectedCollidingSpatialObjectPositionHalfDepth)
						{
							collidingShapesToAlignTo[kZAxis] = collidingShape;
						}
					}
					else
					{
						if(collidingShapePosition->z + collidingShapePositionHalfDepth > selectedCollidingSpatialObjectPosition->z + selectedCollidingSpatialObjectPositionHalfDepth)
						{
							collidingShapesToAlignTo[kZAxis] = collidingShape;
						}
					}
				}
				else
				{
					collidingShapesToAlignTo[kZAxis] = collidingShape;
				}
			}
		}
		else
		{
			VirtualList_pushBack(processedCollidingSpatialObjects, collidingShape);
		}
	}

	if(collidingShapesToAlignTo[kXAxis])
	{
		CollisionSolver_alignToCollidingShape(this, collidingShapesToAlignTo[kXAxis], __X_AXIS, registerObjects);
	}

	if(collidingShapesToAlignTo[kYAxis])
	{
		CollisionSolver_alignToCollidingShape(this, collidingShapesToAlignTo[kYAxis], __Y_AXIS, registerObjects);
	}

	if(collidingShapesToAlignTo[kZAxis])
	{
		CollisionSolver_alignToCollidingShape(this, collidingShapesToAlignTo[kZAxis], __Z_AXIS, registerObjects);
	}

	node = processedCollidingSpatialObjects->head;

	for(; node; node = node->next)
	{
		VirtualList_removeElement(collidingShapes, node->data);
	}

	VirtualList_clear(processedCollidingSpatialObjects);

	return axisOfCollision;
}

// align character to other spatialObject on collision
void CollisionSolver_alignTo(CollisionSolver this, Shape collidingShape, u16 axis, int pad)
{
	ASSERT(this, "CollisionSolver::alignTo: null this");

	// retrieve the colliding spatialObject's position and gap
	VBVec3D myOwnerPosition = *this->ownerPositionToWrite;
	const VBVec3D* otherPosition = __VIRTUAL_CALL(SpatialObject, getPosition, collidingShape);

	// pointers to the dimensions to affect
	const fix19_13* myPositionAxisToCheck = NULL;
	fix19_13* myPositionAxis = NULL;
	const fix19_13* otherPositionAxis = NULL;

	// used to the width, height or depth
	int myHalfSize = 0;
	int otherHalfSize = 0;

	// select the axis to affect
	switch(axis)
	{
		case __X_AXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->x;
			myPositionAxis = &myOwnerPosition.x;
			otherPositionAxis = &otherPosition->x;

			myHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getWidth, this->owner) >> 1);
			otherHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getWidth, collidingShape) >> 1);
			break;

		case __Y_AXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->y;
			myPositionAxis = &myOwnerPosition.y;
			otherPositionAxis = &otherPosition->y;

			myHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getHeight, this->owner) >> 1);
			otherHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getHeight, collidingShape) >> 1);
			break;

		case __Z_AXIS:

			myPositionAxisToCheck = &this->ownerPositionToCheck->z;
			myPositionAxis = &myOwnerPosition.z;
			otherPositionAxis = &otherPosition->z;

			// TODO: must make depth work as the width and high
			if(this->ownerPositionToCheck->z < otherPosition->z)
			{
				myHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, this->owner) >> 1);
				otherHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, collidingShape) >> 1);
			}
			else
			{
				myHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, this->owner) >> 1);
				otherHalfSize = (int)(__VIRTUAL_CALL(SpatialObject, getDepth, collidingShape) >> 1);
			}

			break;
	}

	// decide to which side of the spatialObject align myself
	if(*myPositionAxisToCheck > *otherPositionAxis)
	{
		// pad -= (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1) ? 1 : 0);
		// align right / below / behind
		*myPositionAxis = *otherPositionAxis +
							ITOFIX19_13(otherHalfSize
							+ myHalfSize
							+ pad);
	}
	else
	{
		// align left / over / in front
		*myPositionAxis = *otherPositionAxis -
							ITOFIX19_13(otherHalfSize
							+ myHalfSize
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

	Force totalFriction =
	{
		0, 0, 0
	};

	// get friction in x axis
	VirtualNode node = this->lastCollidingShape[kXAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		totalFriction.y += friction;
		totalFriction.z += friction;
	}

	// get friction in y axis
	node = this->lastCollidingShape[kYAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
		totalFriction.x += friction;
		totalFriction.z += friction;
	}

	// get friction in z axis
	node = this->lastCollidingShape[kZAxis]->head;
	for(; node; node = node->next)
	{
		fix19_13 friction = __VIRTUAL_CALL(SpatialObject, getFriction, __SAFE_CAST(SpatialObject, node->data));
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
	ASSERT(this, "CollisionSolver::getCollidingSpatialObjectsTotalElasticity: null this");
	ASSERT(!((__X_AXIS & axis) && (__Y_AXIS & axis)), "CollisionSolver::getCollidingSpatialObjectsTotalElasticity: more than one axis x, y");
	ASSERT(!((__X_AXIS & axis) && (__Z_AXIS & axis)), "CollisionSolver::getCollidingSpatialObjectsTotalElasticity: more than one axis x, z");
	ASSERT(!((__Y_AXIS & axis) && (__Z_AXIS & axis)), "CollisionSolver::getCollidingSpatialObjectsTotalElasticity: more than one axis y, z");

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
