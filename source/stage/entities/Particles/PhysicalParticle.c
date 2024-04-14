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

#include <Body.h>
#include <PhysicalWorld.h>
#include <Sprite.h>
#include <Utilities.h>
#include <VUEngine.h>

#include "PhysicalParticle.h"


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param particleSpec	Spec of the PhysicalParticle
 * @param creator		Owner Particle System
 */
void PhysicalParticle::constructor(const PhysicalParticleSpec* physicalParticleSpec, ParticleSystem creator)
{
	// construct base Container
	Base::constructor(&physicalParticleSpec->particleSpec, creator);

	this->physicalParticleSpec = physicalParticleSpec;
	fixed_t mass = this->physicalParticleSpec->minimumMass + (this->physicalParticleSpec->massDelta ? Math::random(_gameRandomSeed, this->physicalParticleSpec->massDelta) : 0);
	PhysicalProperties physicalProperties = {mass, 0, 0, Vector3D::zero(), 0};
	this->body = PhysicalWorld::createBody(VUEngine::getPhysicalWorld(_vuEngine), SpatialObject::safeCast(this), &physicalProperties, physicalParticleSpec->axisSubjectToGravity);
}

/**
 * Class destructor
 */
void PhysicalParticle::destructor()
{
	// remove a body
	if(!isDeleted(this->body))
	{
		// remove a body
		PhysicalWorld::destroyBody(VUEngine::getPhysicalWorld(_vuEngine), this->body);
		this->body = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Update
 *
 * @param behavior
 * @return				Boolean that tells whether a body was set active(?)
 */
bool PhysicalParticle::update(uint32 elapsedTime, void (* behavior)(Particle particle))
{
	if(Base::update(this, elapsedTime, behavior))
	{
		Body::stopMovement(this->body, __ALL_AXIS);
		return true;
	}

	return false;
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void PhysicalParticle::applySustainedForce(const Vector3D* force, uint32 movementType)
{
	if(__UNIFORM_MOVEMENT == movementType)
	{
		fixed_t mass = Body::getMass(this->body);

		Vector3D acceleration =
		{
			force->x,
			force->y,
			force->z
		};

		if(mass && __1I_FIXED != mass)
		{
			acceleration.x = __FIXED_DIV(acceleration.x, mass);
			acceleration.y = __FIXED_DIV(acceleration.y, mass);
			acceleration.z = __FIXED_DIV(acceleration.z, mass);
		}

		Vector3D velocity =
		{
			acceleration.x,
			acceleration.y,
			acceleration.z
		};

		Body::moveUniformly(this->body, &velocity);
	}
	else
	{
		Body::applySustainedForce(this->body, force);
	}
}

/**
 * Change mass
 *
 */
void PhysicalParticle::changeMass()
{
	Body::setMass(this->body, this->physicalParticleSpec->minimumMass + (this->physicalParticleSpec->massDelta ? Math::random(_gameRandomSeed, this->physicalParticleSpec->massDelta) : 0));
}

/**
 * Set position
 *
 * @param position
 */
void PhysicalParticle::setPosition(const Vector3D* position)
{
	ASSERT(this->body, "Particle::setPosition: null body");

	if(Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, position, SpatialObject::safeCast(this));
	}

	Base::setPosition(this, position);
}

/**
 * Can move over axis?
 *
 * @param acceleration
 * @return				Boolean that tells whether the PhysicalParticle's body can move over axis (defaults to true)
 */
bool PhysicalParticle::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return (bool)Body::getAxisSubjectToGravity(this->body);
}

/**
 * Reset
 */
void PhysicalParticle::reset()
{
	Base::reset(this);
	Body::reset(this->body);
}
