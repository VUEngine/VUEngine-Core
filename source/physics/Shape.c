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

	SolutionVector solutionVector;

	bool isImpenetrable;

} CollidingShapeRegistry;

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Shape_registerCollidingShape(Shape this, Shape collidingShape, SolutionVector solutionVector, bool isImpenetrable);
static bool Shape_unregisterCollidingShape(Shape this, Shape collidingShape);
static CollidingShapeRegistry* Shape_findCollidingShapeRegistry(Shape this, Shape shape);
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

	// unset owner now
	this->owner = NULL;

	if(this->events)
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventShapeDeleted);
	}

	if(this->collidingShapes)
	{
		VirtualNode node = this->collidingShapes->head;

		for(; node; node = node->next)
		{
			CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

			if(__IS_OBJECT_ALIVE(collidingShapeRegistry->shape))
			{
				Object_removeEventListeners(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), kEventShapeDeleted);
				Object_removeEventListeners(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), kEventShapeChanged);
			}

			__DELETE_BASIC(node->data);
		}

		__DELETE(this->collidingShapes);
		this->collidingShapes = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
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
 * Check if collides with other shape
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param shape					shape to check for overlapping
 */

// check if two rectangles overlap
bool Shape_collides(Shape this, Shape shape)
{
	ASSERT(this, "Ball::collides: null this");

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return false;
	}

	CollisionInformation collisionInformation;

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, shape);
	bool collision = false;

	// test if new collision
	if(!collidingShapeRegistry)
	{
		// check for new overlap
		collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), this, shape);

		if(collisionInformation.shape && collisionInformation.solutionVector.magnitude)
		{
			// new collision
			Shape_registerCollidingShape(this, shape, collisionInformation.solutionVector, false);

			__VIRTUAL_CALL(SpatialObject, enterCollision, this->owner, &collisionInformation);

			collision = true;
		}

		return collision;
	}
	// impenetrable registered colliding shapes require a another test
	// to determine if I'm not colliding against them anymore
	else if(collidingShapeRegistry->isImpenetrable)
	{
		collisionInformation = __VIRTUAL_CALL(Shape, testForCollision, this, shape, (Vector3D){0, 0, 0}, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

		collision = collisionInformation.shape == this && collisionInformation.solutionVector.magnitude >= __STILL_COLLIDING_CHECK_SIZE_INCREMENT;

		if(collision && collisionInformation.solutionVector.magnitude > __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
		{
//			collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), this, shape);

			Shape_resolveCollision(this, &collisionInformation);
		}
	}
	else
	{
		// otherwise make a normal collision test
		collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), this, shape);

		collision = collisionInformation.shape == this && collisionInformation.solutionVector.magnitude;
	}

	if(collision)
	{
		__VIRTUAL_CALL(SpatialObject, updateCollision, this->owner, &collisionInformation);
	}
	else
	{
		Shape_unregisterCollidingShape(this, shape);
		__VIRTUAL_CALL(SpatialObject, exitCollision, this->owner, this, shape, collidingShapeRegistry->isImpenetrable);
	}

	return collision;
}

/**
 * Check if there is a collision in the magnitude
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

	if(!this->collidingShapes)
	{
		return true;
	}

	bool canMove = true;

	Vector3D normalizedDisplacement = Vector3D_normalize(displacement);

	VirtualNode node = this->collidingShapes->head;

	for(; canMove && node; node = node->next)
	{
		CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

		if(collidingShapeRegistry->isImpenetrable)
		{
			fix19_13 cosAngle = Vector3D_dotProduct(collidingShapeRegistry->solutionVector.direction, normalizedDisplacement);
			canMove &= -__F_TO_FIX19_13(1 - 0.1f) < cosAngle;
		}
	}

	// not colliding anymore
	return canMove;
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
SolutionVector Shape_resolveCollision(Shape this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "Shape::resolveCollision: null this");
	ASSERT(collisionInformation->shape, "Shape::resolveCollision: null shape");
	ASSERT(collisionInformation->collidingShape, "Shape::resolveCollision: null collidingEntities");

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return (SolutionVector) {{0, 0, 0}, 0};
	}

	SolutionVector solutionVector = collisionInformation->solutionVector;

	if(collisionInformation->shape == this && solutionVector.magnitude)
	{
		Vector3D displacement = Vector3D_scalarProduct(solutionVector.direction, solutionVector.magnitude);

		// retrieve the colliding spatialObject's position and gap
		Vector3D ownerPosition = *__VIRTUAL_CALL(SpatialObject, getPosition, this->owner);

		ownerPosition.x += displacement.x;
		ownerPosition.y += displacement.y;
		ownerPosition.z += displacement.z;

		__VIRTUAL_CALL(SpatialObject, setPosition, this->owner, &ownerPosition);

		Shape_registerCollidingShape(this, collisionInformation->collidingShape, collisionInformation->solutionVector, true);
	}

	return solutionVector;
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
static void Shape_registerCollidingShape(Shape this, Shape collidingShape, SolutionVector solutionVector, bool isImpenetrable)
{
	ASSERT(this, "Shape::registerCollidingShape: null this");

	if(!this->collidingShapes)
	{
		this->collidingShapes = __NEW(VirtualList);
	}

	bool newEntry = false;
	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, __SAFE_CAST(Shape, collidingShape));

	if(!collidingShapeRegistry)
	{
		newEntry = true;
		collidingShapeRegistry = __NEW_BASIC(CollidingShapeRegistry);
	}

	collidingShapeRegistry->shape = collidingShape;
	collidingShapeRegistry->solutionVector = solutionVector;
	collidingShapeRegistry->isImpenetrable = isImpenetrable;

	if(newEntry)
	{
		VirtualList_pushBack(this->collidingShapes, collidingShapeRegistry);

		Object_addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
		Object_addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
	}
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
static bool Shape_unregisterCollidingShape(Shape this, Shape collidingShape)
{
	ASSERT(this, "Shape::removeCollidingShape: null this");

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, __SAFE_CAST(Shape, collidingShape));

	if(!collidingShapeRegistry)
	{
		return false;
	}

	VirtualList_removeElement(this->collidingShapes, collidingShapeRegistry);

	__DELETE_BASIC(collidingShapeRegistry);

	if(__IS_OBJECT_ALIVE(collidingShape))
	{
		Object_removeEventListeners(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), kEventShapeDeleted);
		Object_removeEventListeners(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), kEventShapeChanged);
	}

	return true;
}

/**
 * Shape destroying listener
 *
 * @memberof				Shape
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		Destroyed shape
 */
static void Shape_onCollidingShapeDestroyed(Shape this, Object eventFirer)
{
	ASSERT(this, "Shape::onCollidingShapeDestroyed: null this");

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return;
	}

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, __SAFE_CAST(Shape, eventFirer));

	if(!collidingShapeRegistry)
	{
		return;
	}

	if(Shape_unregisterCollidingShape(this, collidingShapeRegistry->shape))
	{
		__VIRTUAL_CALL(SpatialObject, collidingShapeOwnerDestroyed, this->owner, this, collidingShapeRegistry->shape, collidingShapeRegistry->isImpenetrable);
	}
}

/**
 * Shape changed listener
 *
 * @memberof				Shape
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		Changed shape
 */
static void Shape_onCollidingShapeChanged(Shape this, Object eventFirer)
{
	ASSERT(this, "Shape::onCollidingShapeChanged: null this");

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return;
	}

	CollidingShapeRegistry* collidingShapeRegistry = Shape_findCollidingShapeRegistry(this, __SAFE_CAST(Shape, eventFirer));

	if(!collidingShapeRegistry)
	{
		return;
	}

	if(Shape_unregisterCollidingShape(this, collidingShapeRegistry->shape))
	{
		__VIRTUAL_CALL(SpatialObject, exitCollision, this->owner, this, collidingShapeRegistry->shape, collidingShapeRegistry->isImpenetrable);
	}
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
	ASSERT(shape, "Shape::findCollidingShapeRegistry: null shape");

	if(!this->collidingShapes || !shape)
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

		if(__IS_OBJECT_ALIVE(collidingShape->owner))
		{
			totalFrictionCoefficient += __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, collidingShape->owner);
		}
	}

	return totalFrictionCoefficient;
}

int Shape_getNumberOfImpenetrableCollidingShapes(Shape this)
{
	ASSERT(this, "Shape::getNumberOfImpenetrableCollidingShapes: null this");

	if(!this->collidingShapes)
	{
		return 0;
	}

	int count = 0;

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		count += ((CollidingShapeRegistry*)node->data)->isImpenetrable ? 1 : 0;
	}

	return count;
}

u32 Shape_getLayers(Shape this)
{
	ASSERT(this, "Shape::getLayers: null this");

	return this->layers;
}

void Shape_setLayers(Shape this, u32 layers)
{
	ASSERT(this, "Shape::setLayers: null this");

	this->layers = layers;
}

u32 Shape_getLayersToIgnore(Shape this)
{
	ASSERT(this, "Shape::getLayersToIgnore: null this");

	return this->layersToIgnore;
}

void Shape_setLayersToIgnore(Shape this, u32 layersToIgnore)
{
	ASSERT(this, "Shape::setLayersToIgnore: null this");

	this->layersToIgnore = layersToIgnore;
}


void Shape_print(Shape this, int x, int y)
{
	ASSERT(this, "Shape::print: null this");

	Printing_text(Printing_getInstance(), "SHAPE ", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Owner:            ", x, y, NULL);
	Printing_text(Printing_getInstance(), this->owner ? __GET_CLASS_NAME(this->owner) : "No owner", x + 7, y++, NULL);
	Printing_hex(Printing_getInstance(), (int)this->owner, x + 7, y++, 8, NULL);

	Printing_text(Printing_getInstance(), "Colliding shapes:            ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->collidingShapes ? VirtualList_getSize(this->collidingShapes) : 0, x + 21, y++, NULL);
	Printing_text(Printing_getInstance(), "Impenetrable shapes:            ", x, y, NULL);
	Printing_int(Printing_getInstance(), Shape_getNumberOfImpenetrableCollidingShapes(this), x + 21, y++, NULL);
}
