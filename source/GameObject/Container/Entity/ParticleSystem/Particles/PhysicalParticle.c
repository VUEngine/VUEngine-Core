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
#include <BodyManager.h>
#include <Sprite.h>
#include <Utilities.h>
#include <VUEngine.h>

#include "PhysicalParticle.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PhysicalParticle::constructor(const PhysicalParticleSpec* physicalParticleSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(&physicalParticleSpec->particleSpec);
	
	this->body = Body::safeCast(PhysicalParticle::addComponent(this, (ComponentSpec*)physicalParticleSpec->bodySpec));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PhysicalParticle::destructor()
{
	this->body = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* PhysicalParticle::getVelocity()
{
	if(isDeleted(this->body))
	{
		return NULL;
	}

	return Body::getVelocity(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PhysicalParticle::setPosition(const Vector3D* position)
{
	if(isDeleted(this->body))
	{
		return;
	}

	if(Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, position, GameObject::safeCast(this));
	}

	Base::setPosition(this, position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool PhysicalParticle::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	if(isDeleted(this->body))
	{
		return false;
	}

	return (bool)Body::getAxisSubjectToGravity(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool PhysicalParticle::update(uint32 elapsedTime, void (* behavior)(Particle particle))
{
	if(Base::update(this, elapsedTime, behavior))
	{
		if(!isDeleted(this->body))
		{
			Body::stopMovement(this->body, __ALL_AXIS);
		}

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PhysicalParticle::applyForce(const Vector3D* force, uint32 movementType)
{
	if(isDeleted(this->body))
	{
		return;
	}

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

		Body::setVelocity(this->body, &velocity);
	}
	else
	{
		Body::applyForce(this->body, force);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

