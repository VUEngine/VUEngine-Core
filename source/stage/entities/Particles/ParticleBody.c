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

#include <ParticleBody.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

extern fixed_t _currentWorldFriction;
extern fixed_t _currentPhysicsElapsedTime;
extern const Acceleration* _currentGravity;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param owner
 * @param mass
 */
void ParticleBody::constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity)
{
	Base::constructor(owner, physicalSpecification, axisSubjectToGravity);
}

/**
 * Class destructor
 */
void ParticleBody::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Update movement
 */
void ParticleBody::update()
{
	if(this->active)
	{
		if(this->awake)
		{
			MovementResult movementResult = Body::updateMovement(this);

			// if stopped on any axis
			if(movementResult.axisStoppedMovement)
			{
				Body::stopMovement(this, movementResult.axisStoppedMovement);
			}
/*			else if(!Body::getMovementOnAllAxis(this))
			{
				Body::sleep(this);
			}
			*/
		}

		// clear any force so the next update does not get influenced
		Body::clearExternalForce(this);
	}
}
