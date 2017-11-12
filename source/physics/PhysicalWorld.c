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

#include <PhysicalWorld.h>
#include <Game.h>
#include <Clock.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define PhysicalWorld_ATTRIBUTES																		\
		Object_ATTRIBUTES																				\
		/**
		 * @var VirtualList		bodies
		 * @brief				registered of bodies
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualList	bodies;																				\
		/**
		 * @var VirtualList		activeBodies
		 * @brief				a list of bodies which must detect collisions
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualList	activeBodies;																		\
		/**
		 * @var VirtualList		removedBodies
		 * @brief				a list of bodies which must be removed
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualList	removedBodies;																		\
		/**
		 * @var Acceleration	gravity
		 * @brief				gravity
		 * @memberof 			PhysicalWorld
		 */																								\
		Acceleration gravity;																			\
		/**
		 * @var fix19_13		frictionCoefficient
		 * @brief				frictionCoefficient
		 * @memberof 			PhysicalWorld
		 */																								\
		fix19_13 frictionCoefficient;																				\
		/**
		 * @var fix19_13		elapsedTime
		 * @brief				elapsed time on last cycle
		 * @memberof 			PhysicalWorld
		 */																								\
		fix19_13 elapsedTime;																			\
		/**
		 * @var fix19_13		previousTime
		 * @brief				time on last cycle
		 * @memberof 			PhysicalWorld
		 */																								\
		fix19_13 previousTime;																			\
		/**
		 * @var VirtualNode		nextBodyToCheckForGravity
		 * @brief				body to check for gravity
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualNode nextBodyToCheckForGravity;															\

/**
 * @class	PhysicalWorld
 * @extends Object
 * @ingroup physics
 */
__CLASS_DEFINITION(PhysicalWorld, Object);
__CLASS_FRIEND_DEFINITION(Body);
__CLASS_FRIEND_DEFINITION(Clock);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(PhysicalWorld)
__CLASS_NEW_END(PhysicalWorld);

/**
 * Class constructor
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 */
void PhysicalWorld_constructor(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// create the shape list
	this->bodies = __NEW(VirtualList);
	this->activeBodies = __NEW(VirtualList);
	this->removedBodies = __NEW(VirtualList);
	this->nextBodyToCheckForGravity = NULL;

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	this->frictionCoefficient = 0;
	this->elapsedTime = 0;
	this->previousTime = 0;
}

/**
 * Class destructor
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 */
void PhysicalWorld_destructor(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::destructor: null this");
	ASSERT(this->bodies, "PhysicalWorld::destructor: null bodies");

	// delete the bodies
	VirtualNode node = this->bodies->head;

	// delete all bodies registered
	for(;node; node = node->next)
	{
		__DELETE(node->data);
	}

	// delete lists
	__DELETE(this->bodies);
	__DELETE(this->activeBodies);
	__DELETE(this->removedBodies);

	this->bodies = NULL;
	this->activeBodies = NULL;
	this->removedBodies = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Register a body
 *
 * @memberof			PhysicalWorld
 * @public
 *
 * @param this			Function scope
 * @param bodyAllocator
 * @param owner
 * @param physicalSpecification
 *
 * @return				Registered Body
 */
Body PhysicalWorld_createBody(PhysicalWorld this, BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification)
{
	ASSERT(this, "PhysicalWorld::createBody: null this");

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if(body)
	{
		return body;
	}

	if(bodyAllocator)
	{
		Body body = bodyAllocator(owner, physicalSpecification);
		VirtualList_pushFront(this->bodies, body);
		ASSERT(__SAFE_CAST(Body, VirtualList_front(this->bodies)), "PhysicalWorld::createBody: bad class body");

		body->awake = true;
		PhysicalWorld_bodyAwake(this, body);

		// return created shape
		return __SAFE_CAST(Body, VirtualList_front(this->bodies));
	}

	return NULL;
}

/**
 * Remove a body
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param body
 */
void PhysicalWorld_destroyBody(PhysicalWorld this, Body body)
{
	ASSERT(this, "PhysicalWorld::destroyBody: null this");
	ASSERT(VirtualList_find(this->bodies, body), "PhysicalWorld::destroyBody: body not registered");
	ASSERT(!VirtualList_find(this->removedBodies, body), "PhysicalWorld::destroyBody: body already being destroyed");

	if(body && !VirtualList_find(this->removedBodies, body))
	{
		// deactivate the shape, will be removed in the next update
		Body_setActive(body, false);

		// place in the removed bodies list
		VirtualList_pushFront(this->removedBodies, body);
	}
}

/**
 * Find a body given an owner
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param owner
 *
 * @return		Found Body
 */
Body PhysicalWorld_getBody(PhysicalWorld this, SpatialObject owner)
{
	ASSERT(this, "PhysicalWorld::getBody: null this");
	ASSERT(this->bodies, "PhysicalWorld::getBody: null bodies");

	// process removed bodies
	PhysicalWorld_processRemovedBodies(this);

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// current body
		Body body = __SAFE_CAST(Body, node->data);
		ASSERT(body, "PhysicalWorld::getBody: null body");

		// check if current shape's owner is the same as the entity calling this method
		if(owner == body->owner)
		{
			return body;
		}
	}

	return NULL;
}

/**
 * Process removed bodies
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 */
void PhysicalWorld_processRemovedBodies(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::processRemovedBodies: null this");
	ASSERT(this->removedBodies, "PhysicalWorld::processRemovedBodies: null bodies");

	VirtualNode node = this->removedBodies->head;

	if(node)
	{
		for(; node; node = node->next)
		{
			Body body = __SAFE_CAST(Body, node->data);

			// remove from the lists
			VirtualList_removeElement(this->bodies, body);
			VirtualList_removeElement(this->activeBodies, body);

			// delete it
			ASSERT(__IS_OBJECT_ALIVE(body), "PhysicalWorld::processRemovedBodies: deleting dead body");
			__DELETE(body);
		}

		// clear the list
		VirtualList_clear(this->removedBodies);

		this->nextBodyToCheckForGravity = NULL;
	}
}

/**
 * Pre-calculate movable shape's position before doing collision detection on them
 *
 * @memberof	PhysicalWorld
 * @private
 *
 * @param this	Function scope
 */
static void PhysicalWorld_checkForGravity(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::checkForGravity: null this");
	ASSERT(this->bodies, "PhysicalWorld::checkForGravity: null bodies");

	VirtualNode node = !this->nextBodyToCheckForGravity ? this->bodies->head: this->nextBodyToCheckForGravity;

	int counter = 0;

	Vector3D gravityDirection =
	{
		this->gravity.x,
		this->gravity.y,
		this->gravity.z,
	};

	// prepare bodies which move
	// this will place the shape in the owner's position
	for(; counter < __BODIES_TO_CHECK_FOR_GRAVITY && node; node = node->next, counter++)
	{
		// load the current shape
		Body body = __SAFE_CAST(Body, node->data);

		if(body->active)
		{
			// check if necessary to apply gravity
			u16 movingState = Body_getMovementOnAllAxes(body);

			u16 gravitySensibleAxis = body->axisSubjectToGravity & ((__X_AXIS & ~(__X_AXIS & movingState) )| (__Y_AXIS & ~(__Y_AXIS & movingState)) | (__Z_AXIS & ~(__Z_AXIS & movingState)));

			if(gravitySensibleAxis && __VIRTUAL_CALL(SpatialObject, canMoveTowards, body->owner, gravityDirection))
			{
				// must account for the fps to avoid situations is which a collision is not detected
				// when a body starts to fall and doesn't have enough time to detect a shape below
				// when moving from one shape over another
				Body_applyGravity(body, gravitySensibleAxis);
			}
		}
	}

	this->nextBodyToCheckForGravity = node;
}

/**
 * Calculate collisions
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param clock
 */
void PhysicalWorld_update(PhysicalWorld this, Clock clock)
{
	ASSERT(this, "PhysicalWorld::update: null this");

	PhysicalWorld_processRemovedBodies(this);

	if(clock->paused)
	{
		// prevent that the initial update after unpausing the clock
		// gets a bigger that usual elapsed time
		this->previousTime = 0;
		this->elapsedTime = 0;
		return;
	}

	Clock_pause(clock, true);

	fix19_13 currentTime = __I_TO_FIX19_13(Clock_getTime(clock));
	fix19_13 elapsedTime = __FIX19_13_DIV(currentTime - this->previousTime, __I_TO_FIX19_13(__MILLISECONDS_IN_SECOND));

	Clock_pause(clock, false);

	if(this->previousTime)
	{
		this->elapsedTime = elapsedTime;

		PhysicalWorld_checkForGravity(this);

		if(!elapsedTime)
		{
			this->previousTime = __I_TO_FIX19_13(Clock_getTime(clock));
			return;
		}

		VirtualNode node = this->activeBodies->head;

		Body_setCurrentElapsedTime(elapsedTime);
		Body_setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
		Body_setCurrentGravity(&this->gravity);

		// check the bodies
		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Body, update, __SAFE_CAST(Body, node->data));
		}
	}

	this->previousTime = __I_TO_FIX19_13(Clock_getTime(clock));
}

/**
 * Unregister all bodies
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 */
void PhysicalWorld_reset(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::reset: null this");
	ASSERT(this->bodies, "PhysicalWorld::reset: null bodies");

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// delete it
		__DELETE(node->data);
	}

	// empty the lists
	VirtualList_clear(this->bodies);
	VirtualList_clear(this->activeBodies);
	VirtualList_clear(this->removedBodies);

	this->elapsedTime = 0;
	this->previousTime = 0;
	this->nextBodyToCheckForGravity = NULL;
}

/**
 * Check if an entity has been registered
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param owner
 *
 * @return		Whether the given SpatialObject has been registered
 */
bool PhysicalWorld_isSpatialObjectRegistered(PhysicalWorld this, SpatialObject owner)
{
	ASSERT(this, "PhysicalWorld::isSpatialObjectRegistered: null this");
	ASSERT(this->bodies, "PhysicalWorld::isSpatialObjectRegistered: null bodies");

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// current body
		Body body = __SAFE_CAST(Body, node->data);

		// check if current body's owner is the same as the entity calling this method
		if(__SAFE_CAST(SpatialObject, owner) == body->owner)
		{
			// check if body is active, maybe a body must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found body
			return Body_isActive(body);
		}
	}

	return false;
}

/**
 * Retrieve frictionCoefficient
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 *
 * @return		PhysicalWorld's frictionCoefficient
 */
fix19_13 PhysicalWorld_getFrictionCoefficient(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getFrictionCoefficient: null this");

	return this->frictionCoefficient;
}

/**
 * Set frictionCoefficient
 *
 * @memberof		PhysicalWorld
 * @public
 *
 * @param this		Function scope
 * @param frictionCoefficient
 */
void PhysicalWorld_setFrictionCoefficient(PhysicalWorld this, fix19_13 frictionCoefficient)
{
	ASSERT(this, "PhysicalWorld::setFrictionCoefficient: null this");

	this->frictionCoefficient = frictionCoefficient;
	Body_setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
}

/**
 * A body has awoken
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param body
 */
void PhysicalWorld_bodyAwake(PhysicalWorld this, Body body)
{
	ASSERT(this, "PhysicalWorld::bodyAwake: null this");
	ASSERT(body, "PhysicalWorld::bodyAwake: null body");
	ASSERT(__SAFE_CAST(Body, body), "PhysicalWorld::bodyAwake: non body");
	ASSERT(__SAFE_CAST(SpatialObject, body->owner), "PhysicalWorld::bodyAwake: body's owner is not an spatial object");

	if(!VirtualList_find(this->activeBodies, body))
	{
		VirtualList_pushBack(this->activeBodies, body);
	}
}

/**
 * Inform of a change in the body
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param body
 */
void PhysicalWorld_bodySleep(PhysicalWorld this, Body body)
{
	ASSERT(this, "PhysicalWorld::bodySleep: null this");
	ASSERT(body, "PhysicalWorld::bodySleep: null body");
	ASSERT(__SAFE_CAST(Body, body), "PhysicalWorld::bodySleep: non body");

	VirtualList_removeElement(this->activeBodies, body);
}
// set gravity
void PhysicalWorld_setGravity(PhysicalWorld this, Acceleration gravity)
{
	this->gravity = gravity;
	Body_setCurrentGravity(&this->gravity);
}

/**
 * Retrieve gravity
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 *
 * @return		PhysicalWorld's gravity
 */
const Vector3D* PhysicalWorld_getGravity(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getGravity: null this");

	return (const Vector3D*)&this->gravity;
}

/**
 * Get last elapsed time
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 *
 * @return		Elapsed time
 */
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getElapsedTime: null this");

	return this->elapsedTime;
}

/**
 * Print status
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 * @param x
 * @param y
 */
void PhysicalWorld_print(PhysicalWorld this, int x, int y)
{
	ASSERT(this, "PhysicalWorld::print: null this");

	Printing_text(Printing_getInstance(), "PHYSICS' STATUS", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Registered bodies: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->bodies), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Active bodies: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->activeBodies), x + 19, y, NULL);
}
