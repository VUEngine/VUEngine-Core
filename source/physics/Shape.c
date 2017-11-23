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

#include <Shape.h>
#include <Game.h>
#include <CollisionManager.h>
#include <CollisionHelper.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STILL_COLLIDING_CHECK_SIZE_INCREMENT 		__I_TO_FIX19_13(1)


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Shape
 * @extends Object
 * @ingroup physics
 */
__CLASS_DEFINITION(Shape, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct CollidingShapeRegistry
{
	Shape shape;

	Vector3D collisionPlaneNormal;

} CollidingShapeRegistry;

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Shape_registerCollidingShape(Shape this, Shape collidingShape, Vector3D collisionPlaneNormal);
static void Shape_unregisterCollidingShape(Shape this, Shape collidingShape);
static void Shape_registerImpenetrableCollidingShape(Shape this, const CollidingShapeRegistry* collidingShapeRegistry);
static CollidingShapeRegistry* Shape_findCollidingShapeRegistry(Shape this, Shape shape);
static bool Shape_isCollidingShapeImpenetrable(Shape this, Shape shape);
static void Shape_onCollidingShapeDestroyed(Shape this, Object eventFirer);
static void Shape_onCollidingShapeChanged(Shape this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 * @param owner
 */
void Shape_constructor(Shape this, SpatialObject owner)
{
	ASSERT(this, "Shape::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// set the owner
	this->owner = owner;

	// not setup yet
	this->ready = false;

	// set flag
	this->checkForCollisions = false;
	this->layers = 0;
	this->layersToIgnore = 0;
	this->collidingShapes = NULL;
	this->impenetrableCollidingShapes = NULL;

	Shape_setActive(this, false);
}

/**
 * Class destructor
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 */
void Shape_destructor(Shape this)
{
	ASSERT(this, "Shape::destructor: null this");

	if(this->events)
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventShapeDeleted);
	}

	if(this->collidingShapes)
	{
		VirtualNode node = this->collidingShapes->head;

		for(; node; node = node->next)
		{
			Object_removeEventListeners(__SAFE_CAST(Object, ((CollidingShapeRegistry*)node->data)->shape), __SAFE_CAST(Object, this), kEventShapeDeleted);
			Object_removeEventListeners(__SAFE_CAST(Object, ((CollidingShapeRegistry*)node->data)->shape), __SAFE_CAST(Object, this), kEventShapeChanged);

			__DELETE_BASIC(node->data);
		}

		__DELETE(this->collidingShapes);
		this->collidingShapes = NULL;
	}

	if(this->impenetrableCollidingShapes)
	{
		__DELETE(this->impenetrableCollidingShapes);
		this->impenetrableCollidingShapes = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}


/**
 * Check if collides with other shape
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param shape					shape to check for overlapping
 */

// check if two rectangles overlap
CollisionInformation Shape_collides(Shape this, Shape shape)
{
	ASSERT(this, "Ball::collides: null this");

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, shape);

	// if shape is already in my colliding shape list
	if(collidingShapeRegistry)
	{
		bool stillColliding = false;

		// check if not colliding anymore
		CollisionInformation collisionInformation = __VIRTUAL_CALL(Shape, testForCollision, this, shape, (Vector3D){0, 0, 0}, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

		if(collisionInformation.collisionSolution.translationVectorLength)
		{
		//    B *= (1 - reduction_length/A.length());
			/*fix19_13 translationVectorCorrectionFactor = __I_TO_FIX19_13(1) - __FIX19_13_DIV(__STILL_COLLIDING_CHECK_SIZE_INCREMENT, collisionInformation.collisionSolution.translationVectorLength);
			collisionInformation.collisionSolution.translationVector = Vector3D_scalarProduct(collisionInformation.collisionSolution.translationVector, translationVectorCorrectionFactor);
			collisionInformation.collisionSolution.translationVectorLength = __FIX19_13_MULT(collisionInformation.collisionSolution.translationVectorLength, translationVectorCorrectionFactor);
*/
			stillColliding = collisionInformation.collidingShape == shape && collisionInformation.collisionSolution.translationVectorLength >= __I_TO_FIX19_13(1);
		}

		// if still colliding
		if(stillColliding)
		{
			__VIRTUAL_CALL(SpatialObject, updateCollision, this->owner, &collisionInformation);

			// just return
			return collisionInformation;
		}

		bool isImpenetrable = Shape_isCollidingShapeImpenetrable(this, shape);

		Vector3D collisionPlaneNormal = collidingShapeRegistry->collisionPlaneNormal;

		// not colliding anymore
		Shape_unregisterCollidingShape(this, shape);

		__VIRTUAL_CALL(SpatialObject, exitCollision, this->owner, this, shape, isImpenetrable);

		return (CollisionInformation){NULL, NULL, {{0, 0, 0}, {0, 0, 0}, 0}};
	}

	// check for new overlap
	CollisionInformation collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), this, shape);

	if(collisionInformation.shape && collisionInformation.collisionSolution.translationVectorLength)
	{
		// new collision
        Shape_registerCollidingShape(this, shape, collisionInformation.collisionSolution.collisionPlaneNormal);

		if(__VIRTUAL_CALL(SpatialObject, enterCollision, this->owner, &collisionInformation))
		{
			Shape_registerImpenetrableCollidingShape(this, Shape_findCollidingShapeRegistry(this, shape));
		}
	}

	return collisionInformation;
}

/**
 * Setup
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param position				Vector3D*
 * @param rotation				Rotation*
 * @param scale					Scale*
 * @param size					Size*
 */
void Shape_setup(Shape this __attribute__ ((unused)), const Vector3D* position __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size __attribute__ ((unused)), u32 layers, u32 layersToIgnore)
{
	ASSERT(this, "Shape::setup: null this");

	this->layers = layers;
	this->layersToIgnore = layersToIgnore;

	if(this->events)
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventShapeChanged);
	}

	// no more setup needed
	this->ready = true;

#ifdef __DRAW_SHAPES
	__VIRTUAL_CALL(Shape, show, this);
#endif
}

/**
 * Retrieve owner
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 *
 * @return		Owning SpatialObject
 */
SpatialObject Shape_getOwner(Shape this)
{
	ASSERT(this, "Shape::getOwner: null this");

	return this->owner;
}

/**
 * Set active
 *
 * @memberof		Shape
 * @public
 *
 * @param this		Function scope
 * @param active
 */
void Shape_setActive(Shape this, bool active)
{
	ASSERT(this, "Shape::setActive: null this");

	if(active)
	{
		CollisionManager_shapeBecameActive(Game_getCollisionManager(Game_getInstance()), this);
	}
	else
	{
		CollisionManager_shapeBecameInactive(Game_getCollisionManager(Game_getInstance()), this);
	}
}

/**
 * Has been configured?
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 *
 * @return		Configured status
 */
bool Shape_isReady(Shape this)
{
	return this->ready;
}

/**
 * Set configured flag
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 * @param ready
 */
void Shape_setReady(Shape this, bool ready)
{
	ASSERT(this, "Shape::setReady: null this");

	this->ready = ready;
}

/**
 * Set flag
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param checkForCollisions
 */
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions)
{
	ASSERT(this, "Shape::setCheckForCollisions: null this");

	this->checkForCollisions = checkForCollisions;
}

/**
 * Get flag
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 *
 * @return		Collision check status
 */
bool Shape_checkForCollisions(Shape this)
{
	ASSERT(this, "Shape::checkForCollisions: null this");

	return this->checkForCollisions;
}

/**
 * Register colliding shape from the lists
 *
 * @memberof				Shape
 * @private
 *
 * @param this				Function scope
 * @param collidingShape	Colliding shape to register
 */
static void Shape_registerCollidingShape(Shape this, Shape collidingShape, Vector3D collisionPlaneNormal)
{
	ASSERT(this, "Shape::registerCollidingShape: null this");
	ASSERT(!Shape_findCollidingShapeRegistry(this, collidingShape), "Shape::registerCollidingShape: already registered shape");

	if(!this->collidingShapes)
	{
		this->collidingShapes = __NEW(VirtualList);
	}

	CollidingShapeRegistry* collidingShapeRegistry = __NEW_BASIC(CollidingShapeRegistry);
	collidingShapeRegistry->shape = collidingShape;
	collidingShapeRegistry->collisionPlaneNormal = collisionPlaneNormal;

	VirtualList_pushBack(this->collidingShapes, collidingShapeRegistry);

	Object_addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
	Object_addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
}

/**
 * Register an impenetrable colliding shape
 *
 * @memberof				Shape
 * @private
 *
 * @param this				Function scope
 * @param collidingShape	Impenetrable colliding shape
 */
static void Shape_registerImpenetrableCollidingShape(Shape this, const CollidingShapeRegistry* collidingShapeRegistry)
{
	ASSERT(this, "Shape::registerImpenetrableCollidingShape: null this");
	ASSERT(collidingShapeRegistry, "Shape::registerImpenetrableCollidingShape: null registry");

	if(!collidingShapeRegistry)
	{
		return;
	}

	if(!this->impenetrableCollidingShapes)
	{
		this->impenetrableCollidingShapes = __NEW(VirtualList);
	}

	VirtualList_pushBack(this->impenetrableCollidingShapes, collidingShapeRegistry);
}

/**
 * Remove colliding shape from the lists
 *
 * @memberof				Shape
 * @private
 *
 * @param this				Function scope
 * @param collidingShape	Colliding shape to remove
 */
static void Shape_unregisterCollidingShape(Shape this, Shape collidingShape)
{
	ASSERT(this, "Shape::removeCollidingShape: null this");

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, __SAFE_CAST(Shape, collidingShape));

	if(collidingShapeRegistry)
	{
		VirtualList_removeElement(this->collidingShapes, collidingShapeRegistry);

		if(this->impenetrableCollidingShapes)
		{
			VirtualList_removeElement(this->impenetrableCollidingShapes, collidingShapeRegistry);
		}

		__DELETE_BASIC(collidingShapeRegistry);

		Object_removeEventListeners(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), kEventShapeDeleted);
		Object_removeEventListeners(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), kEventShapeChanged);
	}
}

/**
 * Shape destroying listener
 *
 * @memberof				Shape
 * @public
 *
 * @param this				Function scope
 * @param eventFirer		Destroyed shape
 */
static void Shape_onCollidingShapeDestroyed(Shape this, Object eventFirer)
{
	ASSERT(this, "Shape::onCollidingShapeDestroyed: null this");

	Shape_unregisterCollidingShape(this, __SAFE_CAST(Shape, eventFirer));
}

/**
 * Shape changed listener
 *
 * @memberof				Shape
 * @public
 *
 * @param this				Function scope
 * @param eventFirer		Changed shape
 */
static void Shape_onCollidingShapeChanged(Shape this, Object eventFirer)
{
	ASSERT(this, "Shape::onCollidingShapeChanged: null this");

	Shape_unregisterCollidingShape(this, __SAFE_CAST(Shape, eventFirer));
}

/**
 * Check if there is a collision in the direction
 *
 * @memberof				Shape
 * @public
 *
 * @param this				Function scope
 * @param displacement		shape displacement
 */
bool Shape_canMoveTowards(Shape this, Vector3D displacement, fix19_13 sizeIncrement __attribute__ ((unused)))
{
	ASSERT(this, "Shape::canMoveTowards: null this");

	if(!this->impenetrableCollidingShapes)
	{
		return true;
	}

	bool canMove = true;

	Vector3D normalizedDisplacement = Vector3D_normalize(displacement);

	VirtualNode node = this->impenetrableCollidingShapes->head;

	for(; canMove && node; node = node->next)
	{
		CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

		canMove &= -__F_TO_FIX19_13(1 - 0.1f) < Vector3D_dotProduct(collidingShapeRegistry->collisionPlaneNormal, normalizedDisplacement);

	}

	return canMove;
}


/**
 * Check if shape is in the impenetrable shape list
 *
 * @memberof		Shape
 * @private
 *
 * @param this		Function scope
 * @param shape		Shape to find
 *
 * @return			true or false
 */
static bool Shape_isCollidingShapeImpenetrable(Shape this, Shape shape)
{
	ASSERT(this, "Shape::findCollidingShapeRegistry: null this");

	if(!this->impenetrableCollidingShapes)
	{
		return false;
	}

	VirtualNode node = this->impenetrableCollidingShapes->head;

	for(; node; node = node->next)
	{
		if(shape == ((CollidingShapeRegistry*)node->data)->shape)
		{
			return true;
		}
	}

	return false;
}

/**
 * Get CollidingShapeRegistry
 *
 * @memberof	Shape
 * @private
 *
 * @param this	Function scope
 * @param shape	Shape to find
 *
 * @return		CollidingShapeRegistry*
 */
static CollidingShapeRegistry* Shape_findCollidingShapeRegistry(Shape this, Shape shape)
{
	ASSERT(this, "Shape::findCollidingShapeRegistry: null this");

	if(!this->collidingShapes)
	{
		return NULL;
	}

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		if(shape == ((CollidingShapeRegistry*)node->data)->shape)
		{
			return (CollidingShapeRegistry*)node->data;
		}
	}

	return NULL;
}

/**
 * Get total friction of colliding shapes
 *
 * @memberof			Shape
 * @public
 *
 * @param this			Function scope
 *
 * @return				The sum of friction coefficients
 */
fix19_13 Shape_getCollidingFrictionCoefficient(Shape this)
{
	ASSERT(this, "Shape::getCollidingFriction: null this");

	if(!this->collidingShapes)
	{
		return 0;
	}

	fix19_13 totalFrictionCoefficient = 0;

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		Shape collidingShape = __SAFE_CAST(Shape, ((CollidingShapeRegistry*)node->data)->shape);

		ASSERT(collidingShape, "Shape::getCollidingFriction: null collidingShape");

		totalFrictionCoefficient += __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, Shape_getOwner(collidingShape));
	}

	return totalFrictionCoefficient;
}

/**
 * Solve the collision by moving owner
 *
 * @memberof			Shape
 * @public
 *
 * @param this			Function scope
 *
 * @return				The sum of friction coefficients
 */
CollisionSolution Shape_resolveCollision(Shape this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "Shape::resolveCollision: null this");
	ASSERT(collisionInformation->shape, "Shape::resolveCollision: null shape");
	ASSERT(collisionInformation->collidingShape, "Shape::resolveCollision: null collidingEntities");

	// retrieve the colliding spatialObject's position and gap
	Vector3D ownerPosition = *__VIRTUAL_CALL(SpatialObject, getPosition, this->owner);

	CollisionSolution collisionSolution = collisionInformation->collisionSolution;

	ownerPosition.x += collisionSolution.translationVector.x;
	ownerPosition.y += collisionSolution.translationVector.y;
	ownerPosition.z += collisionSolution.translationVector.z;

	__VIRTUAL_CALL(SpatialObject, setPosition, this->owner, &ownerPosition);

	return collisionSolution;
}

void Shape_print(Shape this, int x, int y)
{
	ASSERT(this, "Shape::print: null this");

	Printing_text(Printing_getInstance(), "SHAPE ", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Owner:            ", x, y, NULL);
	Printing_text(Printing_getInstance(), __GET_CLASS_NAME(this->owner), x + 7, y++, NULL);
	Printing_hex(Printing_getInstance(), (int)this->owner, x + 7, y++, 8, NULL);

	Printing_text(Printing_getInstance(), "Colliding shapes:            ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->collidingShapes ? VirtualList_getSize(this->collidingShapes) : 0, x + 21, y++, NULL);
	Printing_text(Printing_getInstance(), "Impenetrable shapes:            ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->impenetrableCollidingShapes ? VirtualList_getSize(this->impenetrableCollidingShapes) : 0, x + 21, y++, NULL);
}
