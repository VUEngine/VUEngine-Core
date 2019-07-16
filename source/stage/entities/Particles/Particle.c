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
#include <SpriteManager.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __PARTICLE_VISIBILITY_PADDING	8


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
void Particle::constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, int lifeSpan)
{
	// construct base Container
	Base::constructor();

	this->particleSpec = particleSpec;
	this->spriteSpec = spriteSpec;
	this->lifeSpan = lifeSpan;
	this->sprite = NULL;
	this->position = Vector3D::zero();

	Particle::addSprite(this);
}

/**
 * Class destructor
 */
void Particle::destructor()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::disposeSprite(SpriteManager::getInstance(), this->sprite);
		this->sprite = NULL;
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
	this->sprite = SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)this->spriteSpec, Object::safeCast(this));

	if(this->particleSpec->initialAnimation && this->particleSpec->animationDescription)
	{
		Sprite::play(this->sprite, this->particleSpec->animationDescription, this->particleSpec->initialAnimation);
	}

	ASSERT(this->sprite, "Particle::addSprite: sprite not created");
}

/**
 * Update
 *
 * @param elapsedTime
 * @param behavior
 * @return				Boolean that tells whether a body was set active(?)
 */
bool Particle::update(u32 elapsedTime, void (* behavior)(Particle particle))
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
			return true;
		}

		Sprite::updateAnimation(this->sprite);
	}

	return false;
}

/**
 * Update Visual Representation
 *
 * @param updateSpritePosition
 */
void Particle::synchronizeGraphics(bool updateSpritePosition __attribute__ ((unused)))
{
	NM_ASSERT(this->sprite, "Particle::synchronizeGraphics: null sprite");

	// calculate sprite's parallax
	Sprite::calculateParallax(this->sprite, this->position.z);

	// update sprite's 2D position
	Sprite::position(this->sprite, &this->position);
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void Particle::addForce(const Force* force __attribute__ ((unused)), u32 movementType __attribute__ ((unused)))
{
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
void Particle::setMass(fix10_6 mass __attribute__ ((unused)))
{
}

/**
 * Change mass
 * 
 */
void Particle::changeMass()
{
}

/**
 * Set position
 *
 * @param position
 */
void Particle::setPosition(const Vector3D* position)
{
	this->position = *position;
}

/**
 * Retrieve position
 *
 * @return		Position of particle's body
 */
const Vector3D* Particle::getPosition()
{
	return &this->position;
}

/**
 * Make Particle visible
 */
void Particle::show()
{
	ASSERT(this->sprite, "Particle::show: null sprite");

	Sprite::show(this->sprite);

	if(this->particleSpec->initialAnimation && this->particleSpec->animationDescription)
	{
		Sprite::play(this->sprite, this->particleSpec->animationDescription, this->particleSpec->initialAnimation);
	}
}

/**
 * Make Particle invisible
 */
void Particle::hide(const Vector3D* position)
{
	NM_ASSERT(this->sprite, "Particle::hide: null sprite");

	Particle::setPosition(this, position);

	Sprite::hide(this->sprite);
}

/**
 * Can move over axis?
 *
 * @param acceleration
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool Particle::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return false;
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

	NM_ASSERT(this->sprite, "Particle::resume: null sprite");
}

/**
 * Pause
 */
void Particle::suspend()
{
	SpriteManager::disposeSprite(SpriteManager::getInstance(), this->sprite);
	this->sprite = NULL;
}

/**
 * Reset
 */
void Particle::reset()
{
}

/**
 * Is visible
 *
 * @return		True if within camera's reach
 */
bool Particle::isVisible()
{
	PixelVector spritePosition = Sprite::getDisplacedPosition(this->sprite);

	Texture texture = Sprite::getTexture(this->sprite);

	s16 halfWidth = __PARTICLE_VISIBILITY_PADDING;
	s16 halfHeight = __PARTICLE_VISIBILITY_PADDING;

	if(!isDeleted(texture))
	{
		halfWidth = Texture::getCols(texture) << 2;
		halfHeight = Texture::getRows(texture) << 2;
	}

	extern const CameraFrustum* _cameraFrustum;

	// check x visibility
	if(spritePosition.x + halfWidth < _cameraFrustum->x0 || spritePosition.x - halfWidth > _cameraFrustum->x1)
	{
		return false;
	}

	// check y visibility
	if(spritePosition.y + halfHeight < _cameraFrustum->y0 || spritePosition.y - halfHeight > _cameraFrustum->y1)
	{
		return false;
	}

	// check z visibility
	if(spritePosition.z > __SCREEN_DEPTH || spritePosition.z < -(__SCREEN_DEPTH >> 1))
	{
		return false;
	}

	return true;
}
