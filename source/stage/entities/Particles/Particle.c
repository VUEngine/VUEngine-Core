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
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Particle
 * @extends SpatialObject
 * @ingroup stage-entities-particles
 */



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
 * @memberof					Particle
 * @public
 *
 * @param this					Function scope
 * @param particleDefinition	Definition of the Particle
 * @param spriteDefinition
 * @param lifeSpan
 * @param mass
 */
void Particle::constructor(const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass)
{
	// construct base Container
	Base::constructor();

	this->particleDefinition = particleDefinition;
	this->spriteDefinition = spriteDefinition;
	this->lifeSpan = lifeSpan;
	PhysicalSpecification physicalSpecification = {mass, 0, 0, (Vector3D){0, 0, 0}};
	this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(ParticleBody), __SAFE_CAST(SpatialObject, this), &physicalSpecification, particleDefinition->axesSubjectToGravity);
	this->objectSprite = NULL;
	Particle::addSprite(this);
}

/**
 * Class destructor
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
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
 * @memberof	Particle
 * @private
 *
 * @param this	Function scope
 */
void Particle::addSprite()
{
	ASSERT(this->spriteDefinition->allocator, "Particle::load: no sprite allocator");

	// call the appropriate allocator to support inheritance
	this->objectSprite = __SAFE_CAST(ObjectSprite, ((Sprite (*)(const SpriteDefinition*, Object)) this->spriteDefinition->allocator)((SpriteDefinition*)this->spriteDefinition, __SAFE_CAST(Object, this)));

	if(this->particleDefinition->initialAnimation && this->particleDefinition->animationDescription && __SAFE_CAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite::play(__SAFE_CAST(Sprite, this->objectSprite), this->particleDefinition->animationDescription, this->particleDefinition->initialAnimation);
	}

	ASSERT(this->objectSprite, "Particle::addSprite: sprite not created");
}

/**
 * Update
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 * @param behavior
 *
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
			Body::stopMovement(this->body, __ALL_AXES);
			return true;
		}

		Sprite::updateAnimation(__SAFE_CAST(Sprite, this->objectSprite));
	}

	return false;
}

/**
 * Update Visual Representation
 *
 * @memberof					Particle
 * @public
 *
 * @param this					Function scope
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

	if(__Z_AXIS & Body::getMovementOnAllAxes(this->body))
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
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
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
 * @memberof		Particle
 * @public
 *
 * @param this		Function scope
 * @param lifeSpan
 */
void Particle::setLifeSpan(int lifeSpan)
{
	this->lifeSpan = lifeSpan;
}

/**
 * Set mass
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 * @param mass
 */
void Particle::setMass(fix10_6 mass)
{
	Body::setMass(this->body, mass);
}

/**
 * Set position
 *
 * @memberof		Particle
 * @public
 *
 * @param this		Function scope
 * @param position
 */
void Particle::setPosition(const Vector3D* position)
{
	ASSERT(this->body, "Particle::setPosition: null body");

	Body::setPosition(this->body, position, __SAFE_CAST(SpatialObject, this));

	// sync sprite
	 Sprite::position(this->objectSprite, position);

	// calculate sprite's parallax
	 Sprite::calculateParallax(this->objectSprite, position->z);
}

/**
 * Retrieve position
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
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
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::show()
{
	ASSERT(this->objectSprite, "Particle::show: null objectSprite");

	Sprite::show(__SAFE_CAST(Sprite, this->objectSprite));

	if(this->particleDefinition->initialAnimation && this->particleDefinition->animationDescription && __SAFE_CAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite::play(__SAFE_CAST(Sprite, this->objectSprite), this->particleDefinition->animationDescription, this->particleDefinition->initialAnimation);
	}
}

/**
 * Make Particle invisible
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::hide()
{
	ASSERT(this->objectSprite, "Particle::hide: null objectSprite");

	Sprite::hide(__SAFE_CAST(Sprite, this->objectSprite));

	Body::stopMovement(this->body, __ALL_AXES);
}

/**
 * Can move over axis?
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param acceleration
 *
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool Particle::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return (bool)Body::getAxesSubjectToGravity(this->body);
}

/**
 * Transform
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::transform()
{}

/**
 * Resume
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::resume()
{
	Particle::addSprite(this);

	NM_ASSERT(this->objectSprite, "Particle::resume: null objectSprite");
}

/**
 * Pause
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::suspend()
{
	delete this->objectSprite;

	this->objectSprite = NULL;
}

/**
 * Reset
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle::reset()
{
	Body::reset(this->body);
}


/**
 * Is visible
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 *
 * @return		True if within camera's reach
 */
bool Particle::isVisible()
{
	PixelVector spritePosition = Sprite::getDisplacedPosition(__SAFE_CAST(Sprite, this->objectSprite));

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