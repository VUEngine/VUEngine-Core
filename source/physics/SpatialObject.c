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

#include <string.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(SpatialObject, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
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

	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

Shape SpatialObject_getShape(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getShape: null this");

	return NULL;
}

int SpatialObject_getShapeType(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getShapeType: null this");

	return kCuboid;
}

// does it move?
bool SpatialObject_moves(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::moves: null this");

	// not necessarily
	return false;
}

// defaults to true
int SpatialObject_canMoveOverAxis(SpatialObject this __attribute__ ((unused)), const Acceleration* acceleration __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::canMoveOverAxis: null this");

	return false;
}

int SpatialObject_getWidth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getWidth: null this");

	return 0;
}

int SpatialObject_getHeight(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getHeight: null this");

	return 0;
}

int SpatialObject_getDepth(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getDepth: null this");

	return 0;
}

Gap SpatialObject_getGap(SpatialObject this __attribute__ ((unused)))
{
	ASSERT(this, "SpatialObject::getGap: null this");

	Gap gap =
	{
			0, 0, 0, 0
	};

	return gap;
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

// get elasticiy
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

	static Velocity velocity = {0, 0, 0};
	return velocity;
}

