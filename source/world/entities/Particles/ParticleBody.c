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

#include <ParticleBody.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ParticleBody
__CLASS_DEFINITION(ParticleBody, Body);


//---------------------------------------------------------------------------------------------------------
// 											    PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern fix19_13 _currentWorldFriction;
extern fix19_13 _currentElapsedTime;
extern const Acceleration* _currentGravity;

int Body_updateMovement(Body this, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 frictionForce);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ParticleBody, SpatialObject owner, fix19_13 mass)
__CLASS_NEW_END(ParticleBody, owner, mass);

// class's constructor
void ParticleBody_constructor(ParticleBody this, SpatialObject owner, fix19_13 mass)
{
	ASSERT(this, "ParticleBody::constructor: null this");

	__CONSTRUCT_BASE(Body, owner, mass);
}

// class's destructor
void ParticleBody_destructor(ParticleBody this)
{
	ASSERT(this, "ParticleBody::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// update movement
void ParticleBody_update(ParticleBody this)
{
	ASSERT(this, "Body::update: null this");

	if(this->active && this->awake)
	{
        Force frictionForce = {0, 0, 0};

        // update each axis
        if(this->velocity.x || this->acceleration.x || this->appliedForce.x || ((__ACCELERATED_MOVEMENT & this->movementType.x) && _currentGravity->x && this->acceleration.x))
        {
            Body_updateMovement(__SAFE_CAST(Body, this), __XAXIS & this->axisSubjectToGravity? _currentGravity->x: 0, &this->position.x, &this->velocity.x, &this->acceleration.x, this->appliedForce.x, this->movementType.x, frictionForce.x);
        }

        if(this->velocity.y || this->acceleration.y || this->appliedForce.y || ((__ACCELERATED_MOVEMENT & this->movementType.y) && _currentGravity->y && this->acceleration.y))
        {
            Body_updateMovement(__SAFE_CAST(Body, this), __YAXIS & this->axisSubjectToGravity? _currentGravity->y: 0, &this->position.y, &this->velocity.y, &this->acceleration.y, this->appliedForce.y, this->movementType.y, frictionForce.y);
        }

        if(this->velocity.z || this->acceleration.z || this->appliedForce.z || ((__ACCELERATED_MOVEMENT & this->movementType.z) && _currentGravity->z && this->acceleration.z))
        {
            Body_updateMovement(__SAFE_CAST(Body, this), __ZAXIS & this->axisSubjectToGravity? _currentGravity->z: 0, &this->position.z, &this->velocity.z, &this->acceleration.z, this->appliedForce.z, this->movementType.z, frictionForce.z);
        }

        // clear any force so the next update does not get influenced
        Body_clearForce(__SAFE_CAST(Body, this));
	}
}

Force ParticleBody_calculateFrictionForce(ParticleBody this __attribute__ ((unused)))
{
	ASSERT(this, "ParticleBody::calculateFrictionForce: null this");

	return (Force){0, 0, 0};
}
