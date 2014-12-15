/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <PhysicalWorld.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __CHECK_GRAVITY_CYCLE	(__TARGET_FPS >> 3)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define PhysicalWorld_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* registered of bodies  */													\
	VirtualList	bodies;															\
																				\
	/* a list of bodies which must detect collisions */							\
	VirtualList	activeBodies;													\
																				\
	/* a list of bodies which must be removed */								\
	VirtualList	removedBodies;													\
																				\
	/* gravity */																\
	Acceleration gravity;														\
																				\
	/* friction */																\
	fix19_13 friction;															\
																				\
	/* time elapsed between updates*/											\
	fix19_13 elapsedTime;														\
																				\
	/* time for movement over each axis	*/										\
	unsigned long time;															\
																				\
	/* in game clock */															\
	Clock clock;																\

// define the PhysicalWorld
__CLASS_DEFINITION(PhysicalWorld);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void PhysicalWorld_constructor(PhysicalWorld this);

// only process bodies which move and are active
//Body bodies[__MAX_BODIES_PER_LEVEL] = {NULL};


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(PhysicalWorld);

// class's constructor
static void PhysicalWorld_constructor(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// create the shape list
	this->bodies = __NEW(VirtualList);
	this->activeBodies = __NEW(VirtualList);
	this->removedBodies = __NEW(VirtualList);
	this->clock = NULL;

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	// record this update's time
	this->time = 0;

	//bodies[0] = NULL;
}

// class's destructor
void PhysicalWorld_destructor(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::destructor: null this");
	ASSERT(this->bodies, "PhysicalWorld::destructor: null bodies");

	// delete the bodies
	VirtualNode node = VirtualList_begin(this->bodies);

	// delete all bodies registered
	for (;node; node = VirtualNode_getNext(node))
{
		__DELETE((Body)VirtualNode_getData(node));
	}

	// delete lists
	__DELETE(this->bodies);
	__DELETE(this->activeBodies);
	__DELETE(this->removedBodies);

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// register a body
Body PhysicalWorld_registerBody(PhysicalWorld this, Actor owner, fix19_13 weight)
{
	ASSERT(this, "PhysicalWorld::registerBody: null this");

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if (body)
{
		return body;
	}

	VirtualList_pushFront(this->bodies, (void*)__NEW(Body, __ARGUMENTS((Object)owner, weight)));

	// return created shape
	return (Body)VirtualList_front(this->bodies);
}

// remove a body
void PhysicalWorld_unregisterBody(PhysicalWorld this, Actor owner)
{
	ASSERT(this, "PhysicalWorld::unregisterBody: null this");

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if (body)
{
		// deactivate the shape,
		// will be removed in the next update
		Body_setActive(body, false);

		// place in  the removed bodies list
		VirtualList_pushFront(this->removedBodies, (BYTE*)body);
	}
}

// find a body given an owner
Body PhysicalWorld_getBody(PhysicalWorld this, Actor owner)
{
	ASSERT(this, "PhysicalWorld::getBody: null this");
	ASSERT(this->bodies, "PhysicalWorld::getBody: null bodies");
	VirtualNode node = VirtualList_begin(this->bodies);

	for (; node; node = VirtualNode_getNext(node))
{
		// current body
		Body body = (Body)VirtualNode_getData(node);

		// check if current shape's owner is the same as the entity calling this method
		if ((Object)owner == Body_getOwner(body) && Body_isActive(body))
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

	VirtualNode node = VirtualList_begin(this->removedBodies);

	if (node)
{
		for (; node; node = VirtualNode_getNext(node))
{
			Body body = (Body)VirtualNode_getData(node);

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
	for (node = VirtualList_begin(this->bodies); node; node = VirtualNode_getNext(node))
{
		// load the current shape
		Body body = (Body)VirtualNode_getData(node);

		// check if must apply gravity
		int gravitySensibleAxis = Actor_canMoveOverAxis((Actor)Body_getOwner(body), &this->gravity);

		if (gravitySensibleAxis)

{
			Acceleration gravity = {
				gravitySensibleAxis & __XAXIS? this->gravity.x: 0,
				gravitySensibleAxis & __YAXIS? this->gravity.y: 0,
				gravitySensibleAxis & __ZAXIS? this->gravity.z: 0
			};

			// add gravity
			Body_applyGravity(body, &gravity);
		}
	}
}

// calculate collisions
void PhysicalWorld_start(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::start: null this");

	if (!this->clock)

{
		this->clock = Game_getInGameClock(Game_getInstance());
	}

	this->time = Clock_getTime(this->clock);
}

// calculate collisions
void PhysicalWorld_update(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::update: null this");

	// process removed bodies
	PhysicalWorld_processRemovedBodies(this);

	static int checkForGravity = __CHECK_GRAVITY_CYCLE;

#ifdef __DEBUG
	if (!this->clock)

{
		return;
	}
#endif

	// get the elapsed time
	this->elapsedTime = FIX19_13_DIV(ITOFIX19_13(Clock_getTime(this->clock) - this->time), ITOFIX19_13(__MILISECONDS_IN_SECOND / 10));

	if (0 == this->elapsedTime)
{
		return;
	}

	if (!checkForGravity--)
{
		checkForGravity = __CHECK_GRAVITY_CYCLE;
		PhysicalWorld_checkForGravity(this);
	}

	VirtualNode node = VirtualList_begin(this->activeBodies);

	// check the bodies
	for (; node; node = VirtualNode_getNext(node))
{
		Body_update((Body)VirtualNode_getData(node), &this->gravity, this->elapsedTime);
	}

	// record this update's time
	this->time = Clock_getTime(this->clock);
}

// unregister all bodies
void PhysicalWorld_reset(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::reset: null this");
	ASSERT(this->bodies, "PhysicalWorld::reset: null bodies");

	VirtualNode node = VirtualList_begin(this->bodies);

	for (; node; node = VirtualNode_getNext(node))
{
		// delete it
		__DELETE((Body)VirtualNode_getData(node));
	}

	// empty the lists
	VirtualList_clear(this->bodies);
	VirtualList_clear(this->activeBodies);
	VirtualList_clear(this->removedBodies);

	this->time = 0;
}

// check if an entity has been registered
int PhysicalWorld_isEntityRegistered(PhysicalWorld this, Actor owner)
{
	ASSERT(this, "PhysicalWorld::isEntityRegistered: null this");
	ASSERT(this->bodies, "PhysicalWorld::isEntityRegistered: null bodies");

	VirtualNode node = VirtualList_begin(this->bodies);

	for (; node; node = VirtualNode_getNext(node))
{
		// current body
		Body body = (Body)VirtualNode_getData(node);

		// check if current body's owner is the same as the entity calling this method
		if ((Object)owner == Body_getOwner(body))
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

	if (!VirtualList_find(this->activeBodies, body))
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
	this->gravity.x = gravity.x;
	this->gravity.y = gravity.y;
	this->gravity.z = gravity.z;
}

// retrieve gravity
const VBVec3D* PhysicalWorld_getGravity(PhysicalWorld this)
{
	return (const VBVec3D*)&this->gravity.x;
}

// get last elapsed time
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this)
{
	ASSERT(this, "PhysicalWorld::getElapsedTime: null this");

	return this->elapsedTime;
}

// print status
void PhysicalWorld_print(PhysicalWorld this, int x, int y)
{
	ASSERT(this, "PhysicalWorld::print: null this");

	Printing_text("PHYSICS' STATUS", x, y++);
	Printing_text("Registered bodies: ", x, ++y);
	Printing_int(VirtualList_getSize(this->bodies), x + 19, y);
	Printing_text("Active bodies: ", x, ++y);
	Printing_int(VirtualList_getSize(this->activeBodies), x + 19, y);
}
