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

#define __STILL_COLLIDING_CHECK_SIZE_INCREMENT 		__F_TO_FIX19_13(1.05f)
#define __STILL_COLLIDING_CHECK_SIZE_DECREMENT		__FIX19_13_DIV(__I_TO_FIX19_13(1), __STILL_COLLIDING_CHECK_SIZE_INCREMENT)


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



//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Shape_onCollidingShapeDestroyed(Shape this, Object eventFirer);


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
			Object_removeEventListeners(__SAFE_CAST(Object, node->data), __SAFE_CAST(Object, this), kEventShapeDeleted);
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

	// if shape is already in my colliding shape list
	if(this->collidingShapes && VirtualList_find(this->collidingShapes, shape))
	{
		// check if not colliding anymore
		CollisionInformation collisionInformation = __VIRTUAL_CALL(Shape, testForCollision, this, shape, (Vector3D){0, 0, 0}, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

		bool stillColliding = collisionInformation.collidingShape == shape && collisionInformation.collisionSolution.translationVectorLength;

		// if broad overlap
		if(stillColliding)
		{
			__PRINT_IN_GAME_TIME(20, 1);
			collisionInformation.collisionSolution.translationVector = Vector3D_scalarProduct(collisionInformation.collisionSolution.translationVector, __STILL_COLLIDING_CHECK_SIZE_DECREMENT);
			collisionInformation.collisionSolution.translationVectorLength = __FIX19_13_MULT(collisionInformation.collisionSolution.translationVectorLength, __STILL_COLLIDING_CHECK_SIZE_DECREMENT);

			__VIRTUAL_CALL(SpatialObject, updateCollision, this->owner, &collisionInformation);

			// if non impenetrable colliding shape
			if(this->impenetrableCollidingShapes && !VirtualList_find(this->impenetrableCollidingShapes, shape))
			{
				// just return
				return collisionInformation;
			}

			// otherwise need to allow the proper check to take place because I could have moved since the last
			// cycle
		}
		else
		{
			__PRINT_IN_GAME_TIME(20, 2);
			// remove and tell my owner that I'm not colliding with it anymore
			Object_removeEventListener(__SAFE_CAST(Object, shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
			VirtualList_removeElement(this->collidingShapes, shape);


			__VIRTUAL_CALL(SpatialObject, exitCollision, this->owner, this, shape, VirtualList_find(this->impenetrableCollidingShapes, shape) ? true : false);

			if(this->impenetrableCollidingShapes)
			{
				VirtualList_removeElement(this->impenetrableCollidingShapes, shape);
			}

			return (CollisionInformation){NULL, NULL, {{0, 0, 0}, {0, 0, 0}, 0}};;
		}
	}

	// check for new overlap
	CollisionInformation collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), this, shape);

	if(collisionInformation.shape && collisionInformation.collisionSolution.translationVectorLength)
	{
		__PRINT_IN_GAME_TIME(20, 0);

		if(!this->collidingShapes)
		{
			this->collidingShapes = __NEW(VirtualList);
		}

		if(!VirtualList_find(this->collidingShapes, shape))
		{
			ASSERT(!VirtualList_find(this->collidingShapes, shape), "Shape::collides: already registered shape");
			VirtualList_pushBack(this->collidingShapes, shape);
			Object_addEventListener(__SAFE_CAST(Object, shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
		}


		if(__VIRTUAL_CALL(SpatialObject, enterCollision, this->owner, &collisionInformation))
		{
			if(!this->impenetrableCollidingShapes)
			{
				this->impenetrableCollidingShapes = __NEW(VirtualList);
			}

			if(!VirtualList_find(this->impenetrableCollidingShapes, shape))
			{
				VirtualList_pushBack(this->impenetrableCollidingShapes, shape);
			}
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

	if(this->collidingShapes)
	{
		VirtualList_removeElement(this->collidingShapes, eventFirer);
	}

	if(this->impenetrableCollidingShapes)
	{
		VirtualList_removeElement(this->impenetrableCollidingShapes, eventFirer);
	}
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
bool Shape_canMoveTowards(Shape this, Vector3D displacement, fix19_13 sizeIncrement)
{
	ASSERT(this, "Shape::canMoveTowards: null this");

	if(!this->impenetrableCollidingShapes)
	{
		return true;
	}

	bool canMove = true;

	VirtualNode node = this->impenetrableCollidingShapes->head;

	bool hasNormalizedDisplacement = false;
	Vector3D normalizedDisplacement = (Vector3D){0, 0, 0};

	for(; canMove && node; node = node->next)
	{
		Shape shape = __SAFE_CAST(Shape, node->data);

		CollisionInformation collisionInformation = __VIRTUAL_CALL(Shape, testForCollision, this, shape, displacement, sizeIncrement);

		if(collisionInformation.collisionSolution.translationVectorLength)
		{
			if(canMove)
			{
				if(!hasNormalizedDisplacement)
				{
					hasNormalizedDisplacement = true;
					normalizedDisplacement = Vector3D_normalize(displacement);
				}

				canMove &= __F_TO_FIX19_13(1 - 0.1f) > __ABS(Vector3D_dotProduct(collisionInformation.collisionSolution.collisionPlaneNormal, normalizedDisplacement));
//				canMove &= __I_TO_FIX19_13(1) != __ABS(Vector3D_dotProduct(collisionSolution->collisionPlaneNormal, normalizedDisplacement));
			}
		}
	}

	return canMove;
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
		totalFrictionCoefficient += __VIRTUAL_CALL(SpatialObject, getFrictionCoefficient, Shape_getOwner(__SAFE_CAST(Shape, node->data)));
	}

	return totalFrictionCoefficient;
}


void Shape_print(Shape this, int x, int y)
{
	ASSERT(this, "Shape::print: null this");

	if(this->collidingShapes)
	{
		Printing_int(Printing_getInstance(), VirtualList_getSize(this->collidingShapes), x, y, NULL);
	}
}
