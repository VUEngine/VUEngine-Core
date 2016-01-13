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

#include <PhysicalWorld.h>
#include <Game.h>
#include <Clock.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __CHECK_GRAVITY_CYCLE	(__TARGET_FPS / 10)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define PhysicalWorld_ATTRIBUTES																		\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* registered of bodies  */																			\
	VirtualList	bodies;																					\
																										\
	/* a list of bodies which must detect collisions */													\
	VirtualList	activeBodies;																			\
																										\
	/* a list of bodies which must be removed */														\
	VirtualList	removedBodies;																			\
																										\
	/* gravity */																						\
	Acceleration gravity;																				\
																										\
	/* friction */																						\
	fix19_13 friction;																					\

// define the PhysicalWorld
__CLASS_DEFINITION(PhysicalWorld, Object);

__CLASS_FRIEND_DEFINITION(Body);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void PhysicalWorld_constructor(PhysicalWorld this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(PhysicalWorld);

// class's constructor
static void PhysicalWorld_constructor(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::constructor: null this");

	__CONSTRUCT_BASE();

	// create the shape list
	this->bodies = __NEW(VirtualList);
	this->activeBodies = __NEW(VirtualList);
	this->removedBodies = __NEW(VirtualList);

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;
}

// class's destructor
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

	// allow a new construct
	__SINGLETON_DESTROY;
}

// register a body
Body PhysicalWorld_registerBody(PhysicalWorld this, SpatialObject owner, fix19_13 mass)
{
	ASSERT(this, "PhysicalWorld::registerBody: null this");

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if(body)
	{
		return body;
	}

	VirtualList_pushFront(this->bodies, (void*)__NEW(Body, owner, mass));
	ASSERT(__SAFE_CAST(Body, VirtualList_front(this->bodies)), "PhysicalWorld::registerBody: bad class body");

	// return created shape
	return __SAFE_CAST(Body, VirtualList_front(this->bodies));
}

// remove a body
void PhysicalWorld_unregisterBody(PhysicalWorld this, SpatialObject owner)
{
	ASSERT(this, "PhysicalWorld::unregisterBody: null this");

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);
	ASSERT(body, "PhysicalWorld::unregisterBody: body not found");

	if(body)
	{
		// deactivate the shape, will be removed in the next update
		Body_setActive(body, false);

		// place in the removed bodies list
		VirtualList_pushFront(this->removedBodies, (BYTE*)body);
	}
}

// find a body given an owner
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

// process removed bodies
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
			VirtualList_removeElement(this->bodies, (BYTE*) body);
			VirtualList_removeElement(this->activeBodies, (BYTE*) body);

			// delete it
			__DELETE(body);
		}

		// clear the list
		VirtualList_clear(this->removedBodies);
	}
}

// precalculate movable shape's position before doing collision detection on them
static void PhysicalWorld_checkForGravity(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::checkForGravity: null this");
	ASSERT(this->bodies, "PhysicalWorld::checkForGravity: null bodies");

	VirtualNode node = NULL;

	// prepare bodies which move
	// this will place the shape in the owner's position
	for(node = this->bodies->head; node; node = node->next)
	{
		// load the current shape
		Body body = __SAFE_CAST(Body, node->data);

		// check if necessary to apply gravity
		bool gravitySensibleAxis = body->axisSubjectToGravity & __VIRTUAL_CALL(bool, SpatialObject, canMoveOverAxis, body->owner, &this->gravity);

		if(gravitySensibleAxis)
		{
			Acceleration gravity =
			{
				gravitySensibleAxis & __XAXIS ? this->gravity.x : 0,
				gravitySensibleAxis & __YAXIS ? this->gravity.y : 0,
				gravitySensibleAxis & __ZAXIS ? this->gravity.z : 0
			};

			// add gravity
			Body_applyGravity(body, &gravity);
		}
	}
}

// calculate collisions
void PhysicalWorld_update(PhysicalWorld this, fix19_13 elapsedTime)
{
	ASSERT(this, "PhysicalWorld::update: null this");

	// process removed bodies
	PhysicalWorld_processRemovedBodies(this);

	static int checkForGravity = __CHECK_GRAVITY_CYCLE;

	// get the elapsed time
	elapsedTime = FIX19_13_DIV(elapsedTime, ITOFIX19_13(__MILLISECONDS_IN_SECOND));

	if(0 == elapsedTime)
	{
		return;
	}

	if(!--checkForGravity)
	{
		checkForGravity = __CHECK_GRAVITY_CYCLE;
		PhysicalWorld_checkForGravity(this);
	}

	VirtualNode node = this->activeBodies->head;

	// check the bodies
	for(; node; node = node->next)
	{
		Body_update(__SAFE_CAST(Body, node->data), &this->gravity, elapsedTime);
	}
}

// unregister all bodies
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
}

// check if an entity has been registered
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
		if(__GET_CAST(SpatialObject, owner) == body->owner)
		{
			// check if body is active.... maybe a body must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found body
			return Body_isActive(body);
		}
	}

	return false;
}

// retrieve friction
fix19_13 PhysicalWorld_getFriction(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getFriction: null this");

	return this->friction;
}

// set friction
void PhysicalWorld_setFriction(PhysicalWorld this, fix19_13 friction)
{
	ASSERT(this, "PhysicalWorld::setFriction: null this");

	this->friction = friction;
}

// a body has awaked
void PhysicalWorld_bodyAwaked(PhysicalWorld this, Body body)
{
	ASSERT(this, "PhysicalWorld::bodyAwaked: null this");
	ASSERT(body, "PhysicalWorld::bodyAwaked: null body");

	if(!VirtualList_find(this->activeBodies, body))
	{
		VirtualList_pushBack(this->activeBodies, body);
	}
}

// inform of a change in the body
void PhysicalWorld_bodySleep(PhysicalWorld this, Body body)
{
	ASSERT(this, "PhysicalWorld::bodySleep: null this");

	ASSERT(body, "PhysicalWorld::bodySleep: null body");

	VirtualList_removeElement(this->activeBodies, body);
}
// set gravity
void PhysicalWorld_setGravity(PhysicalWorld this, Acceleration gravity)
{
	this->gravity = gravity;
}

// retrieve gravity
const VBVec3D* PhysicalWorld_getGravity(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getGravity: null this");

	return (const VBVec3D*)&this->gravity;
}

// get last elapsed time
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getElapsedTime: null this");

	return FIX19_13_DIV(ITOFIX19_13(Clock_getElapsedTime(Game_getPhysicsClock(Game_getInstance()))), ITOFIX19_13(__MILLISECONDS_IN_SECOND));
}

// print status
void PhysicalWorld_print(PhysicalWorld this, int x, int y)
{
	ASSERT(this, "PhysicalWorld::print: null this");

	Printing_text(Printing_getInstance(), "PHYSICS STATUS", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Registered bodies: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->bodies), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Active bodies: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->activeBodies), x + 19, y, NULL);
}