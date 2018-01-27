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
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __PHYSICS_TIME_ELAPSED					__FIX10_6_DIV(__1I_FIX10_6, __I_TO_FIX10_6(__TARGET_FPS / __PHYSICS_TIME_ELAPSED_DIVISOR))


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
		 * @var VirtualList		bodiesSentToSleep
		 * @brief				a list of bodies which must be removed from the activeBodies list
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualList	bodiesSentToSleep;																	\
		/**
		 * @var Acceleration	gravity
		 * @brief				gravity
		 * @memberof 			PhysicalWorld
		 */																								\
		Acceleration gravity;																			\
		/**
		 * @var fix10_6		frictionCoefficient
		 * @brief				frictionCoefficient
		 * @memberof 			PhysicalWorld
		 */																								\
		fix10_6 frictionCoefficient;																	\
		/**
		 * @var fix10_6		timeScale
		 * @brief				time scale
		 * @memberof 			PhysicalWorld
		 */																								\
		fix10_6 timeScale;																				\
		/**
		 * @var VirtualNode		bodyToCheckForGravityNode
		 * @brief				body to check for gravity
		 * @memberof 			PhysicalWorld
		 */																								\
		VirtualNode bodyToCheckForGravityNode;															\

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
	this->bodiesSentToSleep = __NEW(VirtualList);

	this->bodyToCheckForGravityNode = NULL;

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	this->frictionCoefficient = 0;
	this->timeScale = __1I_FIX10_6;
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
	__DELETE(this->bodiesSentToSleep);


	this->bodies = NULL;
	this->activeBodies = NULL;
	this->removedBodies = NULL;
	this->bodiesSentToSleep = NULL;

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
Body PhysicalWorld_createBody(PhysicalWorld this, BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
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
		Body body = bodyAllocator(owner, physicalSpecification, axesSubjectToGravity);
		VirtualList_pushFront(this->bodies, body);
		ASSERT(__SAFE_CAST(Body, VirtualList_front(this->bodies)), "PhysicalWorld::createBody: bad class body");

		this->bodyToCheckForGravityNode = NULL;

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
	ASSERT(__IS_OBJECT_ALIVE(body), "PhysicalWorld::destroyBody: dead body");
	ASSERT(VirtualList_find(this->bodies, body), "PhysicalWorld::destroyBody: body not registered");
	ASSERT(!VirtualList_find(this->removedBodies, body), "PhysicalWorld::destroyBody: body already being destroyed");

	if(__IS_OBJECT_ALIVE(body) && !VirtualList_find(this->removedBodies, body))
	{
		// deactivate the shape, will be removed in the next update
		Body_setActive(body, false);

		// place in the removed bodies list
		VirtualList_pushFront(this->removedBodies, body);

		this->bodyToCheckForGravityNode = NULL;
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

	// process auxiliary lists
	PhysicalWorld_processAuxiliaryBodyLists(this);

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
 * Process auxiliary body lists
 *
 * @memberof	PhysicalWorld
 * @public
 *
 * @param this	Function scope
 */
void PhysicalWorld_processAuxiliaryBodyLists(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::processAuxiliaryBodyLists: null this");
	ASSERT(this->removedBodies, "PhysicalWorld::processAuxiliaryBodyLists: null removedBodies");
	ASSERT(this->bodiesSentToSleep, "PhysicalWorld::processAuxiliaryBodyLists: null bodiesSentToSleep");

	VirtualNode node = this->removedBodies->head;

	if(node)
	{
		for(; node; node = node->next)
		{
			Body body = __SAFE_CAST(Body, node->data);

			// remove from the lists
			VirtualList_removeElement(this->bodies, body);
			VirtualList_removeElement(this->activeBodies, body);
			VirtualList_removeElement(this->bodiesSentToSleep, body);

			// delete it
			ASSERT(__IS_OBJECT_ALIVE(body), "PhysicalWorld::processAuxiliaryBodyLists: deleting dead body");
			__DELETE(body);
		}

		// clear the list
		VirtualList_clear(this->removedBodies);

		this->bodyToCheckForGravityNode = NULL;
	}

	node = this->bodiesSentToSleep->head;

	if(node)
	{
		for(; node; node = node->next)
		{
			Body body = __SAFE_CAST(Body, node->data);

			// remove from the lists
			VirtualList_removeElement(this->activeBodies, body);
		}

		// clear the list
		VirtualList_clear(this->bodiesSentToSleep);

		this->bodyToCheckForGravityNode = NULL;
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

	// give preference to the last body in the list
	VirtualNode node = !this->bodyToCheckForGravityNode ? this->bodies->tail: this->bodyToCheckForGravityNode;

	int counter = 0;

	Vector3D gravityDirection =
	{
		this->gravity.x,
		this->gravity.y,
		this->gravity.z,
	};

	// prepare bodies which move
	// this will place the shape in the owner's position
	for(; counter < __BODIES_TO_CHECK_FOR_GRAVITY && node; node = node->previous, counter++)
	{
		// load the current shape
		Body body = __SAFE_CAST(Body, node->data);

		if(body->active)
		{
			// check if necessary to apply gravity
			u16 movingState = Body_getMovementOnAllAxes(body);

			u16 gravitySensibleAxis = body->axesSubjectToGravity & ((__X_AXIS & ~(__X_AXIS & movingState) )| (__Y_AXIS & ~(__Y_AXIS & movingState)) | (__Z_AXIS & ~(__Z_AXIS & movingState)));

			if(gravitySensibleAxis && __VIRTUAL_CALL(SpatialObject, isSubjectToGravity, body->owner, gravityDirection))
			{
				// must account for the fps to avoid situations is which a collision is not detected
				// when a body starts to fall and doesn't have enough time to detect a shape below
				// when moving from one shape over another
				Body_applyGravity(body, gravitySensibleAxis);
			}
		}
	}

	this->bodyToCheckForGravityNode = node;
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

	PhysicalWorld_processAuxiliaryBodyLists(this);

	if(clock->paused)
	{
		return;
	}

	PhysicalWorld_checkForGravity(this);

	// TODO: time scale
	Body_setCurrentElapsedTime(__FIX10_6_MULT(__PHYSICS_TIME_ELAPSED, this->timeScale));
	Body_setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
	Body_setCurrentGravity(&this->gravity);

	VirtualNode node = this->activeBodies->head;

	// check the bodies
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Body, update, node->data);
	}

//	PhysicalWorld_print(this, 1, 0);
#ifdef __SHOW_PHYSICS_PROFILING
	PhysicalWorld_print(this, 1, 1);
#endif
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

	this->bodyToCheckForGravityNode = NULL;
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
fix10_6 PhysicalWorld_getFrictionCoefficient(PhysicalWorld this)
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
void PhysicalWorld_setFrictionCoefficient(PhysicalWorld this, fix10_6 frictionCoefficient)
{
	ASSERT(this, "PhysicalWorld::setFrictionCoefficient: null this");

	this->frictionCoefficient = frictionCoefficient;
	Body_setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
}

/**
 * Set time scale
 *
 * @memberof		PhysicalWorld
 * @public
 *
 * @param this		Function scope
 * @param 			timeScale
 */
void PhysicalWorld_setTimeScale(PhysicalWorld this, fix10_6 timeScale)
{
	ASSERT(this, "PhysicalWorld::setTimeScale: null this");

	this->timeScale = timeScale;
}

/**
 * Get time scale
 *
 * @memberof		PhysicalWorld
 * @public
 *
 * @param this		Function scope
 *
 * @return 			timeScale
 */
u32 PhysicalWorld_getTimeScale(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getTimeScale: null this");

	return this->timeScale;
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
	ASSERT(VirtualList_find(this->bodies, body), "PhysicalWorld::bodyAwake: body not found");

	if(!VirtualList_find(this->activeBodies, body))
	{
		VirtualList_pushBack(this->activeBodies, body);
	}

	VirtualList_removeElement(this->bodiesSentToSleep, body);
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

	if(!VirtualList_find(this->bodiesSentToSleep, body) && VirtualList_find(this->activeBodies, body))
	{
		VirtualList_pushBack(this->bodiesSentToSleep, body);
	}
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
fix10_6 PhysicalWorld_getElapsedTime(PhysicalWorld this __attribute__ ((unused)))
{
	ASSERT(this, "PhysicalWorld::getElapsedTime: null this");

	return __PHYSICS_TIME_ELAPSED;
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
	Printing_text(Printing_getInstance(), "Registered bodies:     ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->bodies), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Active bodies:         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->activeBodies), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Removed bodies:        ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->removedBodies), x + 19, y, NULL);

//	Printing_text(Printing_getInstance(), "Error:                 ", x, ++y, NULL);
//	Printing_int(Printing_getInstance(), VirtualList_getSize(this->bodies) - (VirtualList_getSize(this->activeBodies)), x + 19, y, NULL);
}
