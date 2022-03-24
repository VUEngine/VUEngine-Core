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
#include <Game.h>
#include <Clock.h>
#include <ParticleBody.h>
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
void PhysicalParticle::constructor(const PhysicalParticleSpec* physicalParticleSpec, const SpriteSpec* spriteSpec, int16 lifeSpan)
{
	// construct base Container
	Base::constructor(&physicalParticleSpec->particleSpec, spriteSpec, lifeSpan);

	this->physicalParticleSpec = physicalParticleSpec;
	fix10_6 mass = this->physicalParticleSpec->minimumMass + (this->physicalParticleSpec->massDelta ? Utilities::random(_gameRandomSeed, this->physicalParticleSpec->massDelta) : 0);
	PhysicalSpecification physicalSpecification = {mass, 0, 0, Vector3D::zero(), 0};
	this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(ParticleBody), SpatialObject::safeCast(this), &physicalSpecification, physicalParticleSpec->axisSubjectToGravity);
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
		PhysicalWorld::destroyBody(Game::getPhysicalWorld(Game::getInstance()), this->body);
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
void PhysicalParticle::applySustainedForce(const Force* force, uint32 movementType)
{
	if(__UNIFORM_MOVEMENT == movementType)
	{
		fix10_6 mass = Body::getMass(this->body);

		Acceleration acceleration =
		{
			force->x,
			force->y,
			force->z
		};

		if(mass && __1I_FIX10_6 != mass)
		{
			acceleration.x = __FIX10_6_DIV(acceleration.x, mass);
			acceleration.y = __FIX10_6_DIV(acceleration.y, mass);
			acceleration.z = __FIX10_6_DIV(acceleration.z, mass);
		}

		Velocity velocity =
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
void PhysicalParticle::setMass(fix10_6 mass)
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
bool PhysicalParticle::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return (bool)Body::getaxisSubjectToGravity(this->body);
}

/**
 * Reset
 */
void PhysicalParticle::reset()
{
	Base::reset(this);
	Body::reset(this->body);
}
