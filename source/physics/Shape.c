/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#define __STILL_COLLIDING_CHECK_SIZE_INCREMENT 		__PIXELS_TO_METERS(2)


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Shape
 * @extends Object
 * @ingroup physics
 */

friend class VirtualNode;
friend class VirtualList;


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
void Shape::constructor(SpatialObject owner)
{
	// construct base object
	Base::constructor();

	// set the owner
	this->owner = owner;

	// not setup yet
	this->ready = false;
	this->isActive = true;

	this->wireframe = NULL;

	// set flag
	this->checkForCollisions = false;
	this->layers = 0;
	this->layersToIgnore = 0;
	this->collidingShapes = NULL;
	this->isVisible = true;
	this->moved = false;
}

/**
 * Class destructor
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 */
void Shape::destructor()
{
	// unset owner now
	this->owner = NULL;

	Shape::hide(this);

	if(this->events)
	{
		Object::fireEvent(__SAFE_CAST(Object, this), kEventShapeDeleted);
	}

	if(this->collidingShapes)
	{
		VirtualNode node = this->collidingShapes->head;

		for(; node; node = node->next)
		{
			CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

			ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::destructor: dead collidingShapeRegistry");

			if(__IS_OBJECT_ALIVE(collidingShapeRegistry->shape))
			{
				Object::removeEventListener(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
				Object::removeEventListener(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
			}

			__DELETE_BASIC(collidingShapeRegistry);
		}

		delete this->collidingShapes;
		this->collidingShapes = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Reset
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 */
void Shape::reset()
{
	if(this->collidingShapes)
	{
		VirtualNode node = this->collidingShapes->head;

		for(; node; node = node->next)
		{
			CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

			ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::reset: dead collidingShapeRegistry");

			if(__IS_OBJECT_ALIVE(collidingShapeRegistry->shape))
			{
				Object::removeEventListener(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
				Object::removeEventListener(__SAFE_CAST(Object, collidingShapeRegistry->shape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
			}

			__DELETE_BASIC(collidingShapeRegistry);
		}

		delete this->collidingShapes;
		this->collidingShapes = NULL;
	}
}

/**
 * Setup
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param layers				u32
 * @param layersToIgnore		u32
 */
void Shape::setup(u32 layers, u32 layersToIgnore)
{
	this->layers = layers;
	this->layersToIgnore = layersToIgnore;

	if(this->events)
	{
		Object::fireEvent(__SAFE_CAST(Object, this), kEventShapeChanged);
	}
}

/**
 * Position
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param position				Vector3d*
 * @param rotation				Rotation*
 * @param scale					Scale*
 * @param size					Size*
 */
void Shape::position(const Vector3D* position __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size __attribute__ ((unused)))
{
	if(this->isActive && this->events)
	{
		Object::fireEvent(__SAFE_CAST(Object, this), kEventShapeChanged);
	}

	this->ready = true;
	this->moved = true;
}

/**
 * Process enter collision event
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param collisionData			Collision data
 */
void Shape::enterCollision(CollisionData* collisionData)
{
	if( SpatialObject::enterCollision(this->owner, &collisionData->collisionInformation))
	{
		CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, collisionData->collisionInformation.collidingShape);

		if(collidingShapeRegistry)
		{
			collidingShapeRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collisionData->collisionInformation.collidingShape->owner);
		}
	}
}

/**
 * Process update collision event
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param collisionData			Collision data
 */
void Shape::updateCollision(CollisionData* collisionData)
{
	if(collisionData->isImpenetrableCollidingShape)
	{
		if(collisionData->collisionInformation.solutionVector.magnitude > __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
		{
			Shape::resolveCollision(this, &collisionData->collisionInformation);
		}
	}
	else
	{
		 SpatialObject::updateCollision(this->owner, &collisionData->collisionInformation);
	}
}

/**
 * Process exit collision event
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param collisionData			Collision data
 */
void Shape::exitCollision(CollisionData* collisionData)
{
	SpatialObject::exitCollision(this->owner, collisionData->collisionInformation.shape, collisionData->shapeNotCollidingAnymore, collisionData->isImpenetrableCollidingShape);
	Shape::unregisterCollidingShape(this, collisionData->shapeNotCollidingAnymore);
}

/**
 * Check if collides with other shape
 *
 * @memberof					Shape
 * @public
 *
 * @param this					Function scope
 * @param shape					shape to check for overlapping
 *
  * @return						CollisionData
 */
// check if two rectangles overlap
CollisionData Shape::collides(Shape shape)
{
	CollisionData collisionData =
	{
		// result
		kNoCollision,

		// collision information
		{
			// shape
			NULL,
			// colliding shape
			NULL,
			// solution vector
			{
				// direction
				{0, 0, 0},
				// magnitude
				0
			}
		},

		// out-of-collision shape
		NULL,

		// is impenetrable colliding shape
		false,
	};

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return collisionData;
	}

	CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, shape);

	// test if new collision
	if(!collidingShapeRegistry)
	{
		// check for new overlap
		collisionData.collisionInformation = CollisionHelper::checkIfOverlap(CollisionHelper::getInstance(), this, shape);

		if(collisionData.collisionInformation.shape && collisionData.collisionInformation.solutionVector.magnitude)
		{
			// new collision
			collisionData.result = kEnterCollision;
			collidingShapeRegistry = Shape::registerCollidingShape(this, shape, collisionData.collisionInformation.solutionVector, false);
		}

		//return collisionData;
	}
	// impenetrable registered colliding shapes require a another test
	// to determine if I'm not colliding against them anymore
	else if(collidingShapeRegistry->isImpenetrable && collidingShapeRegistry->solutionVector.magnitude)
	{
		collisionData.collisionInformation =  Shape::testForCollision(this, shape, (Vector3D){0, 0, 0}, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

		if(collisionData.collisionInformation.shape == this && collisionData.collisionInformation.solutionVector.magnitude >= __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
		{
			collisionData.result = kUpdateCollision;
			collisionData.isImpenetrableCollidingShape = true;
		}
		else
		{
			collisionData.collisionInformation.shape = this;
			collisionData.result = kExitCollision;
			collisionData.isImpenetrableCollidingShape = true;
			collisionData.shapeNotCollidingAnymore = shape;
		}
	}
	else
	{
		// otherwise make a normal collision test
		collisionData.collisionInformation = CollisionHelper::checkIfOverlap(CollisionHelper::getInstance(), this, shape);

		if(collisionData.collisionInformation.shape == this && collisionData.collisionInformation.solutionVector.magnitude)
		{
			collisionData.result = kUpdateCollision;
		}
		else
		{
			collisionData.collisionInformation.shape = this;
			collisionData.result = kExitCollision;
			collisionData.isImpenetrableCollidingShape = collidingShapeRegistry->isImpenetrable;
			collisionData.shapeNotCollidingAnymore = shape;
		}
	}

	switch(collisionData.result)
	{
		case kEnterCollision:

			Shape::enterCollision(this, &collisionData);
			break;

		case kUpdateCollision:

			Shape::updateCollision(this, &collisionData);
			break;

		case kExitCollision:

			Shape::exitCollision(this, &collisionData);
			break;

		default:
			break;
	}

	return collisionData;
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
bool Shape::canMoveTowards(Vector3D displacement, fix10_6 sizeIncrement __attribute__ ((unused)))
{
	if(!this->collidingShapes)
	{
		return true;
	}

	bool canMove = true;

	Vector3D normalizedDisplacement = Vector3D::normalize(displacement);

	VirtualNode node = this->collidingShapes->head;

	for(; canMove && node; node = node->next)
	{
		CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

		ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::canMoveTowards: dead collidingShapeRegistry");

		if(collidingShapeRegistry->isImpenetrable)
		{
			// check if solution is valid
			if(collidingShapeRegistry->solutionVector.magnitude)
			{
				fix10_6 cosAngle = Vector3D::dotProduct(collidingShapeRegistry->solutionVector.direction, normalizedDisplacement);
				canMove &= -__F_TO_FIX10_6(1 - 0.01f) < cosAngle;
			}
		}
	}

	// not colliding anymore
	return canMove;
}

/*
void Shape::checkPreviousCollisions(Shape collidingShape)
{
	if(!this->collidingShapes)
	{
		return;
	}

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;

		ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::invalidateSolutionVectors: dead collidingShapeRegistry");

		if(collidingShapeRegistry->isImpenetrable && collidingShapeRegistry->shape != collidingShape)
		{
			CollisionInformation collisionInformation =  Shape::testForCollision(this, collidingShapeRegistry->shape, (Vector3D){0, 0, 0}, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

			if(collisionInformation.shape == this && 0 < collisionInformation.solutionVector.magnitude)
			{
				if(collisionInformation.solutionVector.magnitude > __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
				{
					if(Shape::canMoveTowards(this, Vector3D::scalarProduct(collidingShapeRegistry->solutionVector.direction, collisionInformation.solutionVector.magnitude), 0))
					{
						Shape::displaceOwner(this, Vector3D::scalarProduct(collisionInformation.solutionVector.direction, collisionInformation.solutionVector.magnitude));
					}
				}
				else if(collisionInformation.solutionVector.magnitude < collidingShapeRegistry->solutionVector.magnitude)
				{
					// since I'm not close to that shape anymore, we can discard it
					collidingShapeRegistry->solutionVector.magnitude = 0;
				}
			}
			else
			{
				// since I'm not close to that shape anymore, we can discard it
				collidingShapeRegistry->solutionVector.magnitude = 0;
			}
		}
	}
}
*/

/**
 * Displace owner
 *
 * @memberof				Shape
 * @public
 *
 * @param this				Function scope
 * @param displacement		Displacement to apply to owner
 */
void Shape::displaceOwner(Vector3D displacement)
{
	// retrieve the colliding spatialObject's position and gap
	Vector3D ownerPosition = * SpatialObject::getPosition(this->owner);

	ownerPosition.x += displacement.x;
	ownerPosition.y += displacement.y;
	ownerPosition.z += displacement.z;

	 SpatialObject::setPosition(this->owner, &ownerPosition);
}

/**
 * Solve the collision by moving owner
 *
 * @memberof			Shape
 * @public
 *
 * @param this			Function scope
 */
void Shape::resolveCollision(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->shape, "Shape::resolveCollision: null shape");
	ASSERT(collisionInformation->collidingShape, "Shape::resolveCollision: null collidingEntities");

	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return;
	}

	SolutionVector solutionVector = collisionInformation->solutionVector;

	if(collisionInformation->shape == this && solutionVector.magnitude)
	{
		Shape::displaceOwner(this, Vector3D::scalarProduct(solutionVector.direction, solutionVector.magnitude));

		// need to invalidate solution vectors for other colliding shapes
		//Shape::checkPreviousCollisions(this, collisionInformation->collidingShape);

		CollidingShapeRegistry* collidingShapeRegistry = Shape::registerCollidingShape(this, collisionInformation->collidingShape, collisionInformation->solutionVector, true);
		ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::resolveCollision: dead collidingShapeRegistry");
		collidingShapeRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collisionInformation->collidingShape->owner);
	}
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
SpatialObject Shape::getOwner()
{
	return this->owner;
}

/**
 * Is active?
 *
 * @memberof	Shape
 * @public
 *
 * @param this	Function scope
 *
 * @return		Active status
 */
bool Shape::isActive()
{
	return this->isActive;
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
void Shape::setActive(bool active)
{
	this->isActive = active;
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
bool Shape::isReady()
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
void Shape::setReady(bool ready)
{
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
void Shape::setCheckForCollisions(bool checkForCollisions)
{
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
bool Shape::checkForCollisions()
{
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
CollidingShapeRegistry* Shape::registerCollidingShape(Shape collidingShape, SolutionVector solutionVector, bool isImpenetrable)
{
	if(!this->collidingShapes)
	{
		this->collidingShapes = new VirtualList();
	}

	bool newEntry = false;
	CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, __SAFE_CAST(Shape, collidingShape));

	if(!collidingShapeRegistry)
	{
		newEntry = true;
		collidingShapeRegistry = new CollidingShapeRegistry;
	}

	collidingShapeRegistry->shape = collidingShape;
	collidingShapeRegistry->solutionVector = solutionVector;
	collidingShapeRegistry->isImpenetrable = isImpenetrable;
	collidingShapeRegistry->frictionCoefficient = 0;

	if(newEntry)
	{
		VirtualList::pushBack(this->collidingShapes, collidingShapeRegistry);

		Object::addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
		Object::addEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
	}

	return collidingShapeRegistry;
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
bool Shape::unregisterCollidingShape(Shape collidingShape)
{
	ASSERT(__IS_OBJECT_ALIVE(collidingShape), "Shape::removeCollidingShape: dead collidingShape");

	CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, __SAFE_CAST(Shape, collidingShape));

	if(!collidingShapeRegistry)
	{
		return false;
	}

	ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::removeCollidingShape: dead collidingShapeRegistry");
	VirtualList::removeElement(this->collidingShapes, collidingShapeRegistry);
	__DELETE_BASIC(collidingShapeRegistry);

	if(__IS_OBJECT_ALIVE(collidingShape))
	{
		Object::removeEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeDestroyed, kEventShapeDeleted);
		Object::removeEventListener(__SAFE_CAST(Object, collidingShape), __SAFE_CAST(Object, this), (EventListener)Shape_onCollidingShapeChanged, kEventShapeChanged);
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
void Shape::onCollidingShapeDestroyed(Object eventFirer)
{
	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return;
	}

	Shape shapeNotCollidingAnymore = __SAFE_CAST(Shape, eventFirer);

	CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, shapeNotCollidingAnymore);
	ASSERT(collidingShapeRegistry, "Shape::onCollidingShapeDestroyed: onCollidingShapeDestroyed not found");

	if(!collidingShapeRegistry)
	{
		return;
	}

	bool isImpenetrable = collidingShapeRegistry->isImpenetrable;

	if(Shape::unregisterCollidingShape(this, shapeNotCollidingAnymore))
	{
		 SpatialObject::collidingShapeOwnerDestroyed(this->owner, this, shapeNotCollidingAnymore, isImpenetrable);
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
void Shape::onCollidingShapeChanged(Object eventFirer)
{
	if(!__IS_OBJECT_ALIVE(this->owner))
	{
		return;
	}

	Shape shapeNotCollidingAnymore = __SAFE_CAST(Shape, eventFirer);

	Shape::registerCollidingShape(this, shapeNotCollidingAnymore, (SolutionVector){{0, 0, 0}, 0}, true);

	CollidingShapeRegistry* collidingShapeRegistry = Shape::findCollidingShapeRegistry(this, shapeNotCollidingAnymore);
	ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::removeCollidingShape: dead collidingShapeRegistry");

	bool isImpenetrable = collidingShapeRegistry->isImpenetrable;

	if(Shape::unregisterCollidingShape(this, shapeNotCollidingAnymore))
	{
		 SpatialObject::exitCollision(this->owner, this, shapeNotCollidingAnymore, isImpenetrable);
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
CollidingShapeRegistry* Shape::findCollidingShapeRegistry(Shape shape)
{
	ASSERT(shape, "Shape::findCollidingShapeRegistry: null shape");

	if(!this->collidingShapes || !shape)
	{
		return NULL;
	}

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		ASSERT(__IS_BASIC_OBJECT_ALIVE(node->data), "Shape::findCollidingShapeRegistry: deleted registry");

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
fix10_6 Shape::getCollidingFrictionCoefficient()
{
	if(!this->collidingShapes)
	{
		return 0;
	}

	fix10_6 totalFrictionCoefficient = 0;

	VirtualNode node = this->collidingShapes->head;

	for(; node; node = node->next)
	{
		CollidingShapeRegistry* collidingShapeRegistry = (CollidingShapeRegistry*)node->data;
		ASSERT(__IS_BASIC_OBJECT_ALIVE(collidingShapeRegistry), "Shape::getCollidingFriction: dead collidingShapeRegistry");

		ASSERT(collidingShapeRegistry->shape, "Shape::getCollidingFriction: null collidingShape");

		if(__IS_OBJECT_ALIVE(collidingShapeRegistry->shape->owner))
		{
			totalFrictionCoefficient += collidingShapeRegistry->frictionCoefficient;
		}
	}

	return totalFrictionCoefficient;
}

int Shape::getNumberOfImpenetrableCollidingShapes()
{
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

u32 Shape::getLayers()
{
	return this->layers;
}

void Shape::setLayers(u32 layers)
{
	this->layers = layers;
}

u32 Shape::getLayersToIgnore()
{
	return this->layersToIgnore;
}

void Shape::setLayersToIgnore(u32 layersToIgnore)
{
	this->layersToIgnore = layersToIgnore;
}


// show me
void Shape::show()
{
	if(this->moved)
	{
		Shape::hide(this);
	}

	 Shape::configureWireframe(this);

	// show the wireframe
	Wireframe::show(__SAFE_CAST(Wireframe, this->wireframe));
}

// hide polyhedron
void Shape::hide()
{
	if(this->wireframe)
	{
		// delete the Polyhedron
		delete this->wireframe;
		this->wireframe = NULL;
	}
}

void Shape::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "SHAPE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Owner:            ", x, y, NULL);
	Printing::text(Printing::getInstance(), this->owner ? __GET_CLASS_NAME(this->owner) : "No owner", x + 7, y++, NULL);
	Printing::hex(Printing::getInstance(), (int)this->owner, x + 7, y++, 8, NULL);

	Printing::text(Printing::getInstance(), "Colliding shapes:            ", x, y, NULL);
	Printing::int(Printing::getInstance(), this->collidingShapes ? VirtualList::getSize(this->collidingShapes) : 0, x + 21, y++, NULL);
	Printing::text(Printing::getInstance(), "Impenetrable shapes:            ", x, y, NULL);
	Printing::int(Printing::getInstance(), Shape::getNumberOfImpenetrableCollidingShapes(this), x + 21, y++, NULL);
}