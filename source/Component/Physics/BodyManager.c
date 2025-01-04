/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "BodyManager.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Body;
friend class Clock;
friend class VirtualNode;
friend class VirtualList;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

static fixed_t BodyManager::getElapsedTimeStep()
{
	return __PHYSICS_TIME_ELAPSED_STEP;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Body BodyManager::createComponent(GameObject owner, const BodySpec* bodySpec)
{
	if(NULL == bodySpec)
	{
		return NULL;
	}

	Base::createComponent(this, owner, (ComponentSpec*)bodySpec);

	return BodyManager::createBody(this, owner, bodySpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::destroyComponent(GameObject owner, Body body) 
{
	if(isDeleted(body))
	{
		return;
	}

	Base::destroyComponent(this, owner, Component::safeCast(body));

	BodyManager::destroyBody(this, body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::reset()
{
	this->cycle = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::update()
{
	if(__TARGET_FPS < ++this->cycle)
	{
		this->cycle = 1;
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

	this->dirty = false;

	for(VirtualNode node = this->components->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Body body = Body::safeCast(node->data);

		NM_ASSERT(!isDeleted(body), "BodyManager::update: deleted body");

		if(body->deleteMe)
		{
			// place in the removed bodies list
			VirtualList::removeNode(this->components, node);

			delete body;
			continue;
		}

		if(!body->awake)
		{
			continue;
		}

		if(__NO_AXIS != body->axisSubjectToGravity && GameObject::isSubjectToGravity(body->owner, this->gravity))
		{
			// check if necessary to apply gravity
			uint16 movingState = Body::getMovementOnAllAxis(body);

			uint16 gravitySensibleAxis = 
				body->axisSubjectToGravity 
				& 
				(
					(__X_AXIS & ~(__X_AXIS & movingState) ) | (__Y_AXIS & ~(__Y_AXIS & movingState)) | 
					(__Z_AXIS & ~(__Z_AXIS & movingState))
				);

			if(__NO_AXIS != gravitySensibleAxis)
			{
				fixed_t mass = Body::getMass(body);

				Vector3D force =
				{
					__X_AXIS & gravitySensibleAxis ? __FIXED_MULT(this->gravity.x, mass) : 0,
					__Y_AXIS & gravitySensibleAxis ? __FIXED_MULT(this->gravity.y, mass) : 0,
					__Z_AXIS & gravitySensibleAxis ? __FIXED_MULT(this->gravity.z, mass) : 0,
				};

				Body::applyForce(body, &force);
			}
		}

		Body::update(body, this->cycle, __PHYSICS_TIME_ELAPSED_STEP);
	}

#ifdef __SHOW_PHYSICS_PROFILING
	BodyManager::print(this, 1, 1);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Body BodyManager::createBody(GameObject owner, const BodySpec* bodySpec)
{
	if(this->dirty)
	{
		for(VirtualNode node = this->components->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Body body = Body::safeCast(node->data);

			if(body->deleteMe)
			{
				// place in the removed bodies list
				VirtualList::removeNode(this->components, node);

				delete body;
			}
		}
	}

	// if the entity is already registered
	Body body = new Body(owner, bodySpec);
	VirtualList::pushFront(this->components, body);
	ASSERT(Body::safeCast(VirtualList::front(this->components)), "BodyManager::createBody: bad class body");

	// return created body
	return body;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::destroyBody(Body body)
{
	ASSERT(!isDeleted(body), "BodyManager::destroyBody: dead body");
	ASSERT(VirtualList::find(this->components, body), "BodyManager::destroyBody: body not registered");

	if(!isDeleted(body))
	{
		body->deleteMe = true;
		this->dirty = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::setTimeScale(fixed_t timeScale)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 BodyManager::getTimeScale()
{
	return this->timeScale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::setGravity(Vector3D gravity)
{
	this->gravity = gravity;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D BodyManager::getGravity()
{
	return this->gravity;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::setFrictionCoefficient(fixed_t frictionCoefficient)
{
	this->frictionCoefficient = frictionCoefficient;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t BodyManager::getFrictionCoefficient()
{
	return this->frictionCoefficient;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
/*
 * Print status
 *
 * @param x
 * @param y
 */
void BodyManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "PHYSICS STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Registered bodies:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->components), x + 19, y, NULL);

	for(VirtualNode node = this->components->head; y < 28 && NULL != node; y++, node = node->next)
	{
		Printing::text(Printing::getInstance(), "                         ", x, y, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME((Body::safeCast(node->data))->owner), x, y, NULL);
	}

	Printing::text(Printing::getInstance(), "                         ", x, y, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;

	this->remainingSkipCycles = 0;
	this->skipCycles = 0;

	this->frictionCoefficient = 0;
	this->timeScale = __1I_FIXED;
	this->dirty = false;
	this->cycle = 0;

	BodyManager::setTimeScale(this, __1I_FIXED);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void BodyManager::destructor()
{
	ASSERT(this->components, "BodyManager::destructor: null bodies");

	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
	}
	

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Body BodyManager::getBody(GameObject owner)
{
	ASSERT(this->components, "BodyManager::getBody: null bodies");

	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		// current body
		Body body = Body::safeCast(node->data);
		ASSERT(body, "BodyManager::getBody: null body");

		// check if current body's owner is the same as the entity calling this method
		if(owner == body->owner)
		{
			return body;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BodyManager::isGameObjectRegistered(GameObject owner)
{
	ASSERT(this->components, "BodyManager::isGameObjectRegistered: null bodies");

	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		// current body
		Body body = Body::safeCast(node->data);

		// check if current body's owner is the same as the entity calling this method
		if(GameObject::safeCast(owner) == body->owner)
		{
			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

