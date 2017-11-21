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

#include <ParticleBody.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ParticleBody
 * @extends Body
 * @ingroup stage-entities-particles
 */
__CLASS_DEFINITION(ParticleBody, Body);

typedef struct MovementResult
{
	u16 axesStoppedMovement;
	u16 axesOfAcceleratedBouncing;
	u16 axesOfChangeOfMovement;

} MovementResult;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern fix19_13 _currentWorldFriction;
extern fix19_13 _currentElapsedTime;
extern const Acceleration* _currentGravity;

MovementResult Body_updateMovement(Body this, Acceleration gravity);
Acceleration Body_getGravity(Body this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ParticleBody, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
__CLASS_NEW_END(ParticleBody, owner, physicalSpecification, axesSubjectToGravity);

/**
 * Class constructor
 *
 * @memberof	ParticleBody
 * @public
 *
 * @param this	Function scope
 * @param owner
 * @param mass
 */
void ParticleBody_constructor(ParticleBody this, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
{
	ASSERT(this, "ParticleBody::constructor: null this");

	__CONSTRUCT_BASE(Body, owner, physicalSpecification, axesSubjectToGravity);
}

/**
 * Class destructor
 *
 * @memberof	ParticleBody
 * @public
 *
 * @param this	Function scope
 */
void ParticleBody_destructor(ParticleBody this)
{
	ASSERT(this, "ParticleBody::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Update movement
 *
 * @memberof	ParticleBody
 * @public
 *
 * @param this	Function scope
 */
void ParticleBody_update(ParticleBody this)
{
	ASSERT(this, "ParticleBody::update: null this");

	if(this->active)
	{
		if(this->awake)
		{
			Acceleration gravity = Body_getGravity(__SAFE_CAST(Body, this));
			this->weight = Vector3D_scalarProduct(gravity, this->mass);
			this->friction = Vector3D_scalarProduct(Vector3D_normalize(this->velocity), -this->frictionForceMagnitude);

			MovementResult movementResult = Body_updateMovement(__SAFE_CAST(Body, this), gravity);

			// if stopped on any axis
			if(movementResult.axesStoppedMovement)
			{
				Body_stopMovement(__SAFE_CAST(Body, this), movementResult.axesStoppedMovement);
			}
		}

		// clear any force so the next update does not get influenced
		Body_clearExternalForce(__SAFE_CAST(Body, this));
	}
}