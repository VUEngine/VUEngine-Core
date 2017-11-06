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

#include <string.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	SpatialObject
 * @extends Object
 * @ingroup stage
 */
__CLASS_DEFINITION(SpatialObject, Object);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SpatialObject)
__CLASS_NEW_END(SpatialObject);

// class's constructor
void SpatialObject_constructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void SpatialObject_destructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::destructor: null this");

	if(this->events)
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventSpatialObjectDeleted);
	}

	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// does it move?
bool SpatialObject_moves(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::moves: null this");

	// not necessarily
	return false;
}

bool SpatialObject_isMoving(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::isMoving: null this");

	return false;
}


// defaults to true
u16 SpatialObject_getAxisAllowedForMovement(SpatialObject this __attribute__ ((unused)), const Acceleration* acceleration __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getAxisAllowedForMovement: null this");

	return false;
}

u16 SpatialObject_getWidth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getWidth: null this");

	return 0;
}

u16 SpatialObject_getHeight(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getHeight: null this");

	return 0;
}

u16 SpatialObject_getDepth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getDepth: null this");

	return 0;
}

const VBVec3D* SpatialObject_getPosition(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getPosition: null this");

	static VBVec3D position =
	{
		0, 0, 0
	};

	return &position;
}

void SpatialObject_setPosition(SpatialObject this __attribute__ ((unused)), const VBVec3D* position __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setPosition: null this");
}

const Rotation* SpatialObject_getRotation(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getRotation: null this");

	static Rotation rotation =
	{
		0, 0, 0
	};

	return &rotation;
}

void SpatialObject_setRotation(SpatialObject this __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setRotation: null this");
}

const Scale* SpatialObject_getScale(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getScale: null this");

	static Scale scale =
	{
		__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9
	};

	return &scale;
}
void SpatialObject_setScale(SpatialObject this __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setScale: null this");
}

// get elasticity
fix19_13 SpatialObject_getElasticity(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getElasticity: null this");

	return 0;
}

// get friction
fix19_13 SpatialObject_getFriction(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getFriction: null this");

	return 0;
}

// get velocity
Velocity SpatialObject_getVelocity(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getVelocity: null this");

	return (Velocity){0, 0, 0};
}

bool SpatialObject_isAffectedByRelativity(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::isAffectedByRelativity: null this");

	return false;
}

bool SpatialObject_processCollision(SpatialObject this __attribute__ ((unused)), const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::processCollision: null this");

	return false;
}


u16 SpatialObject_getMovementState(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getMovementState: null this");

	return 0;
}

/**
 * Retrieve shapes list
 *
 * @memberof	SpatialObject
 * @public
 *
 * @param this	Function scope
 *
 * @return		SpatialObject's Shape list
 */
VirtualList SpatialObject_getShapes(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getShapes: null this");

	return NULL;
}
