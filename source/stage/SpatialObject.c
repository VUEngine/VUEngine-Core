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



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void SpatialObject::constructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::constructor: null this");

	// construct base object
	Base::constructor();
}

// class's destructor
void SpatialObject::destructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::destructor: null this");

	if(this->events)
	{
		Object::fireEvent(__SAFE_CAST(Object, this), kEventSpatialObjectDeleted);
	}

	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	Base::destructor();
}

bool SpatialObject::isMoving(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::isMoving: null this");

	return false;
}

// defaults to true
bool SpatialObject::isSubjectToGravity(SpatialObject this __attribute__ ((unused)), Acceleration gravity __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::isSubjectToGravity: null this");

	return false;
}

fix10_6 SpatialObject::getWidth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getWidth: null this");

	return 0;
}

fix10_6 SpatialObject::getHeight(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getHeight: null this");

	return 0;
}

fix10_6 SpatialObject::getDepth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getDepth: null this");

	return 0;
}

const Vector3D* SpatialObject::getPosition(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getPosition: null this");

	static Vector3D position =
	{
		0, 0, 0
	};

	return &position;
}

void SpatialObject::setPosition(SpatialObject this __attribute__ ((unused)), const Vector3D* position __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setPosition: null this");
}

const Rotation* SpatialObject::getRotation(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getRotation: null this");

	static Rotation rotation =
	{
		0, 0, 0
	};

	return &rotation;
}

void SpatialObject::setRotation(SpatialObject this __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setRotation: null this");
}

const Scale* SpatialObject::getScale(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getScale: null this");

	static Scale scale =
	{
		__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9
	};

	return &scale;
}
void SpatialObject::setScale(SpatialObject this __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::setScale: null this");
}

// get bounciness
fix10_6 SpatialObject::getBounciness(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getBounciness: null this");

	return 0;
}

// get friction
fix10_6 SpatialObject::getFrictionCoefficient(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getFrictionCoefficient: null this");

	return 0;
}

// get velocity
Velocity SpatialObject::getVelocity(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getVelocity: null this");

	return (Velocity){0, 0, 0};
}

bool SpatialObject::isAffectedByRelativity(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::isAffectedByRelativity: null this");

	return false;
}

bool SpatialObject::enterCollision(SpatialObject this __attribute__ ((unused)), const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::enterCollision: null this");

	return false;
}

bool SpatialObject::updateCollision(SpatialObject this __attribute__ ((unused)), const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::updateCollision: null this");

	return false;
}

void SpatialObject::exitCollision(SpatialObject this __attribute__ ((unused)), Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore __attribute__ ((unused)), bool isShapeImpenetrable __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::exitCollision: null this");
}

void SpatialObject::collidingShapeOwnerDestroyed(SpatialObject this __attribute__ ((unused)), Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore __attribute__ ((unused)), bool isShapeImpenetrable __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::collidingShapeOwnerDestroyed: null this");
}

u16 SpatialObject::getMovementState(SpatialObject this __attribute__ ((unused)))
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
VirtualList SpatialObject::getShapes(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getShapes: null this");

	return NULL;
}

/**
 * Retrieve shapes list
 *
 * @memberof	SpatialObject
 * @public
 *
 * @param this	Function scope
 *
 * @return		no type
 */
u32 SpatialObject::getInGameType(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getInGameType: null this");

	return kNoType;
}
