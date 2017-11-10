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

	this->collidingShapes = __NEW(VirtualList);
	this->collidingShapePurgeNode = this->collidingShapes->head;
}

// class's destructor
void CollisionSolver_destructor(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::destructor: null this");

	VirtualList_clear(this->collidingShapes);

	__DELETE(this->collidingShapes);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// update colliding entities
bool CollisionSolver_purgeCollidingShapesList(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::updateCollisionStatus: null this");

	if(!this->collidingShapePurgeNode)
	{
		this->collidingShapePurgeNode = this->collidingShapes->head;
	}

	if(this->collidingShapePurgeNode)
	{
		Shape shapeToRemove = __SAFE_CAST(Shape, this->collidingShapePurgeNode->data);

		this->collidingShapePurgeNode = this->collidingShapePurgeNode->next;

		VBVec3D displacement = {0, 0, 0};
		fix19_13 sizeIncrement = __I_TO_FIX19_13(1);

		VirtualList ownerShapes = __VIRTUAL_CALL(SpatialObject, getShapes, this->owner);

		if(ownerShapes)
		{
			VirtualNode ownerShapeNode = ownerShapes->head;

			bool collision = false;

			for(; ownerShapeNode; ownerShapeNode = ownerShapeNode->next)
			{
				CollisionSolution collisionSolution = __VIRTUAL_CALL(Shape, testForCollision, ownerShapeNode->data, __SAFE_CAST(Shape, shapeToRemove), displacement, sizeIncrement);

				if(collisionSolution.translationVectorLength)
				{
					collision = true;
					break;
				}
			}

			if(!collision)
			{
				VirtualList_removeElement(this->collidingShapes, shapeToRemove);
				this->collidingShapePurgeNode = this->collidingShapes->head;

				return true;
			}
		}
	}

	return false;
}

VirtualList CollisionSolver_testForCollisions(CollisionSolver this, VBVec3D displacement, fix19_13 sizeIncrement, const Shape shape)
{
	ASSERT(this, "CollisionSolver::testForCollisions: null this");

	if(!(displacement.x | displacement.y | displacement.z))
	{
		return 0;
	}

	VirtualList collisionSolutionList = __NEW(VirtualList);
	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		CollisionSolution collisionSolution = __VIRTUAL_CALL(Shape, testForCollision, shape, __SAFE_CAST(Shape, node->data), displacement, sizeIncrement);

		if(collisionSolution.translationVectorLength)
		{
			CollisionSolution* collisionSolutionEntry = __NEW_BASIC(CollisionSolution);
			*collisionSolutionEntry = collisionSolution;
			VirtualList_pushBack(collisionSolutionList, collisionSolutionEntry);
		}
	}

	return collisionSolutionList;
}

// process event
static void CollisionSolver_onCollidingShapeDestroyed(CollisionSolver this, Object eventFirer)
{
	ASSERT(this, "CollisionSolver::collidingShapeDestroyed: null this");

	VirtualList_removeElement(this->collidingShapes, eventFirer);
	this->collidingShapePurgeNode = this->collidingShapes->head;
}

// resolve collision against other entities
bool CollisionSolver_resolveCollision(CollisionSolver this, CollisionInformation* collisionInformation)
{
	ASSERT(this, "CollisionSolver::resolveCollision: null this");
	ASSERT(collisionInformation->shape, "CollisionSolver::resolveCollision: null shape");
	ASSERT(collisionInformation->collidingShape, "CollisionSolver::resolveCollision: null collidingEntities");

	// retrieve the colliding spatialObject's position and gap
	VBVec3D ownerPosition = *__VIRTUAL_CALL(SpatialObject, getPosition, this->owner);

	// if pending SAT check
	if(!collisionInformation->isCollisionSolutionValid)
	{
		// force it
		collisionInformation->collisionSolution = __VIRTUAL_CALL(Shape, getCollisionSolution, collisionInformation->shape, collisionInformation->collidingShape);
	}

	ownerPosition.x += collisionInformation->collisionSolution.translationVector.x;
	ownerPosition.y += collisionInformation->collisionSolution.translationVector.y;
	ownerPosition.z += collisionInformation->collisionSolution.translationVector.z;

	__VIRTUAL_CALL(SpatialObject, setPosition, this->owner, &ownerPosition);

	if(collisionInformation->collisionSolution.translationVectorLength)
	{
		VirtualList_removeElement(this->collidingShapes, collisionInformation->collidingShape);
		VirtualList_pushBack(this->collidingShapes, collisionInformation->collidingShape);
		Object_addEventListener(__SAFE_CAST(Object, collisionInformation->collidingShape), __SAFE_CAST(Object, this), (EventListener)CollisionSolver_onCollidingShapeDestroyed, kEventShapeDeleted);

		this->collidingShapePurgeNode = this->collidingShapes->head;

		return true;
	}

	return false;
}

bool CollisionSolution_hasCollidingShapes(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::hasCollidingShapes: null this");

	return VirtualList_getSize(this->collidingShapes) ? true : false;
}

// retrieve friction of colliding objects
fix19_13 CollisionSolver_getSurroundingFrictionCoefficient(CollisionSolver this)
{
	ASSERT(this, "CollisionSolver::getSurroundingFrictionCoefficient: null this");

	fix19_13 totalFriction = 0;

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		totalFriction += __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, __SAFE_CAST(SpatialObject, Shape_getOwner(__SAFE_CAST(Shape, node->data))));
	}

	return totalFriction;
}
