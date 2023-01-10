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

#include <PhysicalParticle.h>
#include <PhysicalWorld.h>
#include <ObjectAnimatedSprite.h>
#include <VUEngine.h>
#include <Clock.h>
#include <Utilities.h>


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
 * @param spriteSpec
 * @param lifeSpan
 * @param mass
 */
void PhysicalParticle::constructor(const PhysicalParticleSpec* physicalParticleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan)
{
	// construct base Container
	Base::constructor(&physicalParticleSpec->particleSpec, spriteSpec, wireframeSpec, lifeSpan);

	this->physicalParticleSpec = physicalParticleSpec;
	fixed_t mass = this->physicalParticleSpec->minimumMass + (this->physicalParticleSpec->massDelta ? Utilities::random(_gameRandomSeed, this->physicalParticleSpec->massDelta) : 0);
	PhysicalSpecification physicalSpecification = {mass, 0, 0, Vector3D::zero(), 0};
	this->body = PhysicalWorld::createBody(VUEngine::getPhysicalWorld(_vuEngine), SpatialObject::safeCast(this), &physicalSpecification, physicalParticleSpec->axisSubjectToGravity);
}

/**
 * Class destructor
 */
void PhysicalParticle::destructor()
{
	// remove a body
	if(this->body)
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
 * @param elapsedTime
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
 * Transform
 */
void PhysicalParticle::transform()
{
	this->position = *Body::getPosition(this->body);
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

		Body::moveUniformly(this->body, velocity);
	}
	else
	{
		Body::applySustainedForce(this->body, force);
	}
}

/**
 * Set mass
 *
 * @param mass
 */
void PhysicalParticle::setMass(fixed_t mass)
{
	Body::setMass(this->body, mass);
}

/**
 * Change mass
 *
 */
void PhysicalParticle::changeMass()
{
	Body::setMass(this->body, this->physicalParticleSpec->minimumMass + (this->physicalParticleSpec->massDelta ? Utilities::random(_gameRandomSeed, this->physicalParticleSpec->massDelta) : 0));
}

/**
 * Set position
 *
 * @param position
 */
void PhysicalParticle::setPosition(const Vector3D* position)
{
	ASSERT(this->body, "Particle::setPosition: null body");

	Body::setPosition(this->body, position, SpatialObject::safeCast(this));

	Base::setPosition(this, position);
}

/**
 * Make PhysicalParticle invisible
 */
void PhysicalParticle::hide()
{
	Base::hide(this);

	Body::stopMovement(this->body, __ALL_AXIS);
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
