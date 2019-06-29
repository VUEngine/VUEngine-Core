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
void PhysicalParticle::constructor(const PhysicalParticleSpec* physicalParticleSpec, const SpriteSpec* spriteSpec, int lifeSpan)
{
	// construct base Container
	Base::constructor(&physicalParticleSpec->particleSpec, spriteSpec, lifeSpan);

	this->physicalParticleSpec = physicalParticleSpec;
	fix10_6 mass = this->physicalParticleSpec->minimumMass + this->physicalParticleSpec->massDelta ? Utilities::random(Game::getRandomSeed(Game::getInstance()), this->physicalParticleSpec->massDelta) : 0;
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
bool PhysicalParticle::update(u32 elapsedTime, void (* behavior)(Particle particle))
{
	if(0 <= this->lifeSpan && Base::update(this, elapsedTime, behavior))
	{
		Body::stopMovement(this->body, __ALL_AXIS);
		return true;
	}

	this->position = *Body::getPosition(this->body);

	return false;
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void PhysicalParticle::addForce(const Force* force, u32 movementType)
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

		if(mass)
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
		Body::addForce(this->body, force);
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
	Body::setMass(this->body, this->physicalParticleSpec->minimumMass + this->physicalParticleSpec->massDelta ? Utilities::random(Game::getRandomSeed(Game::getInstance()), this->physicalParticleSpec->massDelta) : 0);
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
	Body::reset(this->body);
}

/**
 * Update Visual Representation
 *
 * @param updateSpritePosition
 */
void PhysicalParticle::synchronizeGraphics(bool updateSpritePosition)
{
	Base::synchronizeGraphics(this, updateSpritePosition || Body::getMovementOnAllAxis(this->body));
}