/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Body;
friend class Clock;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void PhysicalWorld::constructor()
{
	Base::constructor();

	// create the shape list
	this->bodies = new VirtualList();
	this->activeBodies = new VirtualList();

	this->bodyToCheckForGravityNode = NULL;

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	this->remainingSkipCycles = 0;
	this->skipCycles = 0;

	this->frictionCoefficient = 0;
	this->timeScale = __1I_FIX10_6;

	Body::setCurrentElapsedTime(__PHYSICS_TIME_ELAPSED);
	PhysicalWorld::setTimeScale(this, __1I_FIX10_6);
}

/**
 * Class destructor
 */
void PhysicalWorld::destructor()
{
	ASSERT(this->bodies, "PhysicalWorld::destructor: null bodies");

	// delete the bodies
	VirtualNode node = this->bodies->head;

	// delete all bodies registered
	for(;node; node = node->next)
	{
		delete node->data;
	}

	// delete lists
	delete this->bodies;
	delete this->activeBodies;

	this->bodies = NULL;
	this->activeBodies = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Register a body
 *
 * @param bodyAllocator
 * @param owner
 * @param physicalSpecification
 * @return				Registered Body
 */
Body PhysicalWorld::createBody(BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity)
{
	// if the entity is already registered
	Body body = PhysicalWorld::getBody(this, owner);

	if(body)
	{
		return body;
	}

	if(bodyAllocator)
	{
		Body body = bodyAllocator(owner, physicalSpecification, axisSubjectToGravity);
		VirtualList::pushFront(this->bodies, body);
		ASSERT(Body::safeCast(VirtualList::front(this->bodies)), "PhysicalWorld::createBody: bad class body");

		this->bodyToCheckForGravityNode = NULL;

		// return created shape
		return Body::safeCast(VirtualList::front(this->bodies));
	}

	ASSERT(false, "PhysicalWorld::createBody: could not create body");

	return NULL;
}

/**
 * Remove a body
 *
 * @param body
 */
void PhysicalWorld::destroyBody(Body body)
{
	ASSERT(!isDeleted(body), "PhysicalWorld::destroyBody: dead body");
	ASSERT(VirtualList::find(this->bodies, body), "PhysicalWorld::destroyBody: body not registered");

	if(!isDeleted(body) && VirtualList::find(this->bodies, body))
	{
		// place in the removed bodies list
		VirtualList::removeElement(this->bodies, body);
		VirtualList::removeElement(this->activeBodies, body);

		delete body;
		this->bodyToCheckForGravityNode = NULL;
	}
}

/**
 * Find a body given an owner
 *
 * @param owner
 * @return		Found Body
 */
Body PhysicalWorld::getBody(SpatialObject owner)
{
	ASSERT(this->bodies, "PhysicalWorld::getBody: null bodies");

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// current body
		Body body = Body::safeCast(node->data);
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
 * Pre-calculate movable shape's position before doing collision detection on them
 *
 * @private
 */
void PhysicalWorld::checkForGravity()
{
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
		Body body = Body::safeCast(node->data);

		if(body->active)
		{
			// check if necessary to apply gravity
			uint16 movingState = Body::getMovementOnAllAxis(body);

			uint16 gravitySensibleAxis = body->axisSubjectToGravity & ((__X_AXIS & ~(__X_AXIS & movingState) )| (__Y_AXIS & ~(__Y_AXIS & movingState)) | (__Z_AXIS & ~(__Z_AXIS & movingState)));

			if(gravitySensibleAxis &&  SpatialObject::isSubjectToGravity(body->owner, gravityDirection))
			{
				// must account for the fps to avoid situations is which a collision is not detected
				// when a body starts to fall and doesn't have enough time to detect a shape below
				// when moving from one shape over another
				Body::applyGravity(body, gravitySensibleAxis);
			}
		}
	}

	this->bodyToCheckForGravityNode = node;
}

/**
 * Calculate collisions
 *
 * @param clock
 */
void PhysicalWorld::update(Clock clock)
{
	if(clock->paused)
	{
		return;
	}

	if(__1I_FIX10_6 > this->timeScale)
	{
		if(__F_TO_FIX10_6(0.5f) < this->timeScale)
		{
			if(++this->remainingSkipCycles > this->skipCycles)
			{
				this->remainingSkipCycles = 0;
				return;
			}
		}
		else
		{
			if(++this->remainingSkipCycles <= this->skipCycles)
			{
				return;
			}

			this->remainingSkipCycles = 0;
		}
	}

	PhysicalWorld::checkForGravity(this);

	// TODO: time scale
	Body::setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
	Body::setCurrentGravity(&this->gravity);

	for(VirtualNode node = this->activeBodies->head; node; node = node->next)
	{
		Body body = Body::safeCast(node->data);

		Body::update(body);
	}

#ifdef __SHOW_PHYSICS_PROFILING
	PhysicalWorld::print(this, 1, 1);
#endif
}

/**
 * Unregister all bodies
 */
void PhysicalWorld::reset()
{
	ASSERT(this->bodies, "PhysicalWorld::reset: null bodies");

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// delete it
		delete node->data;
	}

	// empty the lists
	VirtualList::clear(this->bodies);
	VirtualList::clear(this->activeBodies);

	this->bodyToCheckForGravityNode = NULL;
}

/**
 * Check if an entity has been registered
 *
 * @param owner
 * @return		Whether the given SpatialObject has been registered
 */
bool PhysicalWorld::isSpatialObjectRegistered(SpatialObject owner)
{
	ASSERT(this->bodies, "PhysicalWorld::isSpatialObjectRegistered: null bodies");

	VirtualNode node = this->bodies->head;

	for(; node; node = node->next)
	{
		// current body
		Body body = Body::safeCast(node->data);

		// check if current body's owner is the same as the entity calling this method
		if(SpatialObject::safeCast(owner) == body->owner)
		{
			// check if body is active, maybe a body must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found body
			return Body::isActive(body);
		}
	}

	return false;
}

/**
 * Retrieve frictionCoefficient
 *
 * @return		PhysicalWorld's frictionCoefficient
 */
fix10_6 PhysicalWorld::getFrictionCoefficient()
{
	return this->frictionCoefficient;
}

/**
 * Set frictionCoefficient
 *
 * @param frictionCoefficient
 */
void PhysicalWorld::setFrictionCoefficient(fix10_6 frictionCoefficient)
{
	this->frictionCoefficient = frictionCoefficient;
	Body::setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
}

/**
 * Set time scale
 *
 * @param 			timeScale
 */
void PhysicalWorld::setTimeScale(fix10_6 timeScale)
{
	this->timeScale = timeScale;

	if(this->timeScale > __1I_FIX10_6)
	{
		this->timeScale = __1I_FIX10_6;
	}
	else if(0 >= timeScale)
	{
		this->timeScale = __F_TO_FIX10_6(0.1f);
	}

	this->remainingSkipCycles = 0;
	this->skipCycles = 0;

	if(__F_TO_FIX10_6(0.5f) < this->timeScale)
	{
		uint32 gameFramesPerSecond = __TARGET_FPS / __PHYSICS_TIME_ELAPSED_DIVISOR;
		fix10_6 targetUpdatesPerSecond = __FIX10_6_MULT(__I_TO_FIX10_6(gameFramesPerSecond), this->timeScale);
		fix10_6 targetSkipsPerSecond = __I_TO_FIX10_6(gameFramesPerSecond) - targetUpdatesPerSecond;

		if(targetSkipsPerSecond)
		{
			this->skipCycles = __FIX10_6_TO_I(__FIX10_6_DIV(targetUpdatesPerSecond, targetSkipsPerSecond) + __05F_FIX10_6);
		}
	}
	else
	{
		this->skipCycles = __FIX10_6_TO_I(__FIX10_6_DIV(__1I_FIX10_6, this->timeScale) - __1I_FIX10_6 + __05F_FIX10_6);
	}
}

/**
 * Get time scale
 *
 * @return 			timeScale
 */
uint32 PhysicalWorld::getTimeScale()
{
	return this->timeScale;
}

/**
 * A body has awoken
 *
 * @param body
 */
void PhysicalWorld::bodyAwake(Body body)
{
	ASSERT(body, "PhysicalWorld::bodyAwake: null body");
	ASSERT(Body::safeCast(body), "PhysicalWorld::bodyAwake: non body");
	ASSERT(SpatialObject::safeCast(body->owner), "PhysicalWorld::bodyAwake: body's owner is not an spatial object");
	ASSERT(VirtualList::find(this->bodies, body), "PhysicalWorld::bodyAwake: body not found");

	if(!VirtualList::find(this->activeBodies, body))
	{
		VirtualList::pushBack(this->activeBodies, body);
	}
}

/**
 * Inform of a change in the body
 *
 * @param body
 */
void PhysicalWorld::bodySleep(Body body)
{
	ASSERT(body, "PhysicalWorld::bodySleep: null body");
	ASSERT(Body::safeCast(body), "PhysicalWorld::bodySleep: non body");

	VirtualList::removeElement(this->activeBodies, body);
}

/**
 * Inform that body has been inactivated
 *
 * @param body
 */
void PhysicalWorld::bodySetInactive(Body body)
{
	ASSERT(body, "PhysicalWorld::bodySetInactive: null body");
	ASSERT(Body::safeCast(body), "PhysicalWorld::bodySleep: non body");

	VirtualList::removeElement(this->activeBodies, body);
}

// set gravity
void PhysicalWorld::setGravity(Acceleration gravity)
{
	this->gravity = gravity;
	Body::setCurrentGravity(&this->gravity);
}

/**
 * Retrieve gravity
 *
 * @return		PhysicalWorld's gravity
 */
const Vector3D* PhysicalWorld::getGravity()
{
	return (const Vector3D*)&this->gravity;
}

/**
 * Get last elapsed time
 *
 * @return		Elapsed time
 */
fix10_6 PhysicalWorld::getElapsedTime()
{
	return __PHYSICS_TIME_ELAPSED;
}

/**
 * Print status
 *
 * @param x
 * @param y
 */
void PhysicalWorld::print(int x, int y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "PHYSICS STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Registered bodies:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->bodies), x + 19, y, NULL);
	Printing::text(Printing::getInstance(), "Active bodies:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->activeBodies), x + 19, y, NULL);

//	Printing::text(Printing::getInstance(), "Error:                 ", x, ++y, NULL);
//	Printing::int(Printing::getInstance(), VirtualList::getSize(this->bodies) - (VirtualList::getSize(this->activeBodies)), x + 19, y, NULL);
}
