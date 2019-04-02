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

#include <Particle.h>
#include <PhysicalWorld.h>
#include <ObjectAnimatedSprite.h>
#include <Game.h>
#include <Clock.h>
#include <ParticleBody.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __PARTICLE_VISIBILITY_PADDING	__I_TO_FIX10_6(30)


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param particleSpec	Spec of the Particle
 * @param spriteSpec
 * @param lifeSpan
 * @param mass
 */
void Particle::constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, int lifeSpan, fix10_6 mass)
{
	// construct base Container
	Base::constructor();

	this->particleSpec = particleSpec;
	this->spriteSpec = spriteSpec;
	this->lifeSpan = lifeSpan;
	PhysicalSpecification physicalSpecification = {mass, 0, 0, Vector3D::zero(), 0};
	this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(ParticleBody), SpatialObject::safeCast(this), &physicalSpecification, particleSpec->axisSubjectToGravity);
	this->objectSprite = NULL;
	Particle::addSprite(this);
}

/**
 * Class destructor
 */
void Particle::destructor()
{
	// remove a body
	if(this->body)
	{
		// remove a body
		PhysicalWorld::destroyBody(Game::getPhysicalWorld(Game::getInstance()), this->body);
		this->body = NULL;
	}

	if(this->objectSprite)
	{
		delete this->objectSprite;
		this->objectSprite = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add a sprite
 *
 * @private
 */
void Particle::addSprite()
{
	ASSERT(this->spriteSpec->allocator, "Particle::load: no sprite allocator");

	// call the appropriate allocator to support inheritance
	this->objectSprite = ObjectSprite::safeCast(((Sprite (*)(const SpriteSpec*, Object)) this->spriteSpec->allocator)((SpriteSpec*)this->spriteSpec, Object::safeCast(this)));

	if(this->particleSpec->initialAnimation && this->particleSpec->animationDescription && ObjectAnimatedSprite::safeCast(this->objectSprite))
	{
		Sprite::play(this->objectSprite, this->particleSpec->animationDescription, this->particleSpec->initialAnimation);
	}

	ASSERT(this->objectSprite, "Particle::addSprite: sprite not created");
}

/**
 * Update
 *
 * @param elapsedTime
 * @param behavior
 * @return				Boolean that tells whether a body was set active(?)
 */
u32 Particle::update(u32 elapsedTime, void (* behavior)(Particle particle))
{
	if(0 <= this->lifeSpan)
	{
		this->lifeSpan -= elapsedTime;

		if(behavior)
		{
			behavior(this);
		}

		if(0 > this->lifeSpan)
		{
			Body::stopMovement(this->body, __ALL_AXIS);
			return true;
		}

		Sprite::updateAnimation(this->objectSprite);
	}

	return false;
}

/**
 * Update Visual Representation
 *
 * @param updateSpritePosition
 */
void Particle::synchronizeGraphics(bool updateSpritePosition)
{
	if(!(updateSpritePosition | Body::isAwake(this->body)))
	{
		return;
	}

	const Vector3D* position = Body::getPosition(this->body);

	ASSERT(this->objectSprite, "Particle::synchronizeGraphics: null objectSprite");

	if(__Z_AXIS & Body::getMovementOnAllAxis(this->body))
	{
		// calculate sprite's parallax
		Sprite::calculateParallax(this->objectSprite, position->z);
	}

	// update sprite's 2D position
	Sprite::position(this->objectSprite, position);
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void Particle::addForce(const Force* force, u32 movementType)
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
 * Set lifespan
 *
 * @param lifeSpan
 */
void Particle::setLifeSpan(int lifeSpan)
{
	this->lifeSpan = lifeSpan;
}

/**
 * Set mass
 *
 * @param mass
 */
void Particle::setMass(fix10_6 mass)
{
	Body::setMass(this->body, mass);
}

/**
 * Set position
 *
 * @param position
 */
void Particle::setPosition(const Vector3D* position)
{
	ASSERT(this->body, "Particle::setPosition: null body");

	Body::setPosition(this->body, position, SpatialObject::safeCast(this));

	// sync sprite
	Sprite::position(this->objectSprite, position);

	// calculate sprite's parallax
	Sprite::calculateParallax(this->objectSprite, position->z);
}

/**
 * Retrieve position
 *
 * @return		Position of particle's body
 */
const Vector3D* Particle::getPosition()
{
	ASSERT(this->body, "Particle::getPosition: null body");

	return Body::getPosition(this->body);
}

/**
 * Make Particle visible
 */
void Particle::show()
{
	ASSERT(this->objectSprite, "Particle::show: null objectSprite");

	Sprite::show(this->objectSprite);

	if(this->particleSpec->initialAnimation && this->particleSpec->animationDescription && ObjectAnimatedSprite::safeCast(this->objectSprite))
	{
		Sprite::play(this->objectSprite, this->particleSpec->animationDescription, this->particleSpec->initialAnimation);
	}
}

/**
 * Make Particle invisible
 */
void Particle::hide()
{
	ASSERT(this->objectSprite, "Particle::hide: null objectSprite");

	Sprite::hide(this->objectSprite);

	Body::stopMovement(this->body, __ALL_AXIS);
}

/**
 * Can move over axis?
 *
 * @param acceleration
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool Particle::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return (bool)Body::getaxisSubjectToGravity(this->body);
}

/**
 * Transform
 */
void Particle::transform()
{}

/**
 * Resume
 */
void Particle::resume()
{
	Particle::addSprite(this);

	NM_ASSERT(this->objectSprite, "Particle::resume: null objectSprite");
}

/**
 * Pause
 */
void Particle::suspend()
{
	delete this->objectSprite;

	this->objectSprite = NULL;
}

/**
 * Reset
 */
void Particle::reset()
{
	Body::reset(this->body);
}

/**
 * Is visible
 *
 * @return		True if within camera's reach
 */
bool Particle::isVisible()
{
	PixelVector spritePosition = Sprite::getDisplacedPosition(this->objectSprite);

	// check x visibility
	if((unsigned)(spritePosition.x + __PARTICLE_VISIBILITY_PADDING) >= (unsigned)(__I_TO_FIX10_6(__SCREEN_WIDTH) + __PARTICLE_VISIBILITY_PADDING))
	{
		return false;
	}

	// check y visibility
	if((unsigned)(spritePosition.y + __PARTICLE_VISIBILITY_PADDING) >= (unsigned)(__I_TO_FIX10_6(__SCREEN_HEIGHT) + __PARTICLE_VISIBILITY_PADDING))
	{
		return false;
	}

	// check z visibility
	if((unsigned)(spritePosition.z + __PARTICLE_VISIBILITY_PADDING) >= (unsigned)(__I_TO_FIX10_6(__SCREEN_HEIGHT) + __PARTICLE_VISIBILITY_PADDING))
	{
		return false;
	}

	return true;
}
