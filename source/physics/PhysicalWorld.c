/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <PhysicalWorld.h>
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

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	this->remainingSkipCycles = 0;
	this->skipCycles = 0;

	this->frictionCoefficient = 0;
	this->timeScale = __1I_FIXED;
	this->dirty = false;

	Body::setCurrentElapsedTime(__PHYSICS_TIME_ELAPSED);
	PhysicalWorld::setTimeScale(this, __1I_FIXED);
}

/**
 * Class destructor
 */
void PhysicalWorld::destructor()
{
	ASSERT(this->bodies, "PhysicalWorld::destructor: null bodies");

	VirtualList::deleteData(this->bodies);
	delete this->bodies;
	this->bodies = NULL;

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
	if(this->dirty)
	{
		for(VirtualNode node = this->bodies->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Body body = Body::safeCast(node->data);

			if(body->destroy)
			{
				// place in the removed bodies list
				VirtualList::removeNode(this->bodies, node);

				delete body;
			}
		}
	}

	// if the entity is already registered
	if(bodyAllocator)
	{
		Body body = bodyAllocator(owner, physicalSpecification, axisSubjectToGravity);
		VirtualList::pushFront(this->bodies, body);
		ASSERT(Body::safeCast(VirtualList::front(this->bodies)), "PhysicalWorld::createBody: bad class body");

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

	if(!isDeleted(body))
	{
		body->destroy = true;
		this->dirty = true;
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

	for(; NULL != node; node = node->next)
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

	if(__1I_FIXED > this->timeScale)
	{
		if(__F_TO_FIXED(0.5f) < this->timeScale)
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

	// TODO: time scale
	Body::setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
	Body::setCurrentGravity(&this->gravity);

	this->dirty = false;

	for(VirtualNode node = this->bodies->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Body body = Body::safeCast(node->data);

		NM_ASSERT(!isDeleted(body), "PhysicalWorld::update: deleted body");

		if(body->destroy)
		{
			// place in the removed bodies list
			VirtualList::removeNode(this->bodies, node);

			delete body;
			continue;
		}

		if(!body->active || !body->awake)
		{
			continue;
		}

		if(__NO_AXIS != body->axisSubjectToGravity && SpatialObject::isSubjectToGravity(body->owner, this->gravity))
		{
			// check if necessary to apply gravity
			uint16 movingState = Body::getMovementOnAllAxis(body);

			uint16 gravitySensibleAxis = body->axisSubjectToGravity & ((__X_AXIS & ~(__X_AXIS & movingState) )| (__Y_AXIS & ~(__Y_AXIS & movingState)) | (__Z_AXIS & ~(__Z_AXIS & movingState)));

			if(__NO_AXIS != gravitySensibleAxis)
			{
				// must account for the fps to avoid situations is which a collision is not detected
				// when a body starts to fall and doesn't have enough time to detect a shape below
				// when moving from one shape over another
				Body::applyGravity(body, gravitySensibleAxis);
			}
		}

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

	VirtualList::deleteData(this->bodies);
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

	for(; NULL != node; node = node->next)
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
fixed_t PhysicalWorld::getFrictionCoefficient()
{
	return this->frictionCoefficient;
}

/**
 * Set frictionCoefficient
 *
 * @param frictionCoefficient
 */
void PhysicalWorld::setFrictionCoefficient(fixed_t frictionCoefficient)
{
	this->frictionCoefficient = frictionCoefficient;
	Body::setCurrentWorldFrictionCoefficient(this->frictionCoefficient);
}

/**
 * Set time scale
 *
 * @param 			timeScale
 */
void PhysicalWorld::setTimeScale(fixed_t timeScale)
{
	this->timeScale = timeScale;

	if(this->timeScale > __1I_FIXED)
	{
		this->timeScale = __1I_FIXED;
	}
	else if(0 >= timeScale)
	{
		this->timeScale = __F_TO_FIXED(0.1f);
	}

	this->remainingSkipCycles = 0;
	this->skipCycles = 0;

	if(__F_TO_FIXED(0.5f) < this->timeScale)
	{
		uint32 gameFramesPerSecond = __TARGET_FPS / __PHYSICS_TIME_ELAPSED_DIVISOR;
		fixed_t targetUpdatesPerSecond = __FIXED_MULT(__I_TO_FIXED(gameFramesPerSecond), this->timeScale);
		fixed_t targetSkipsPerSecond = __I_TO_FIXED(gameFramesPerSecond) - targetUpdatesPerSecond;

		if(targetSkipsPerSecond)
		{
			this->skipCycles = __FIXED_TO_I(__FIXED_DIV(targetUpdatesPerSecond, targetSkipsPerSecond) + __05F_FIXED);
		}
	}
	else
	{
		this->skipCycles = __FIXED_TO_I(__FIXED_DIV(__1I_FIXED, this->timeScale) - __1I_FIXED + __05F_FIXED);
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
fixed_t PhysicalWorld::getElapsedTime()
{
	return __PHYSICS_TIME_ELAPSED;
}

/**
 * Print status
 *
 * @param x
 * @param y
 */
void PhysicalWorld::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "PHYSICS STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Registered bodies:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->bodies), x + 19, y, NULL);

	for(VirtualNode node = this->bodies->head; y < 28 && NULL != node; y++, node = node->next)
	{
		Printing::text(Printing::getInstance(), "                         ", x, y, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME((Body::safeCast(node->data))->owner), x, y, NULL);
	}

	Printing::text(Printing::getInstance(), "                         ", x, y, NULL);
}
