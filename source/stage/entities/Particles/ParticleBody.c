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
implements ParticleBody : Body;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern fix10_6 _currentWorldFriction;
extern fix10_6 _currentElapsedTime;
extern const Acceleration* _currentGravity;

MovementResult Body::updateMovement(Body this);
Acceleration Body::getGravity(Body this);


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
void ParticleBody::constructor(ParticleBody this, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
{
	ASSERT(this, "ParticleBody::constructor: null this");

	Base::constructor(owner, physicalSpecification, axesSubjectToGravity);
}

/**
 * Class destructor
 *
 * @memberof	ParticleBody
 * @public
 *
 * @param this	Function scope
 */
void ParticleBody::destructor(ParticleBody this)
{
	ASSERT(this, "ParticleBody::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Update movement
 *
 * @memberof	ParticleBody
 * @public
 *
 * @param this	Function scope
 */
void ParticleBody::update(ParticleBody this)
{
	ASSERT(this, "ParticleBody::update: null this");

	if(this->active)
	{
		if(this->awake)
		{
			MovementResult movementResult = Body::updateMovement(__SAFE_CAST(Body, this));

			// if stopped on any axis
			if(movementResult.axesStoppedMovement)
			{
				Body::stopMovement(__SAFE_CAST(Body, this), movementResult.axesStoppedMovement);
			}
/*			else if(!Body::getMovementOnAllAxes(__SAFE_CAST(Body, this)))
			{
				Body::sleep(__SAFE_CAST(Body, this));
			}
			*/
		}

		// clear any force so the next update does not get influenced
		Body::clearExternalForce(__SAFE_CAST(Body, this));
	}
}