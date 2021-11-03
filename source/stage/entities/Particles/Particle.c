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
void Particle::constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, int16 lifeSpan)
{
	// construct base Container
	Base::constructor();

	this->lifeSpan = lifeSpan;
	this->sprite = NULL;
	this->position = Vector3D::zero();
	this->previousZ = 0;
	this->expired = false;

	Particle::addSprite(this, spriteSpec, particleSpec->animationDescription, particleSpec->initialAnimation);
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
void Particle::addSprite(const SpriteSpec* spriteSpec, const AnimationDescription* animationDescription, const char* animationName)
{
	ASSERT(spriteSpec->allocator, "Particle::load: no sprite allocator");

	if(spriteSpec)
	{
		// call the appropriate allocator to support inheritance
		this->sprite = SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)spriteSpec, Object::safeCast(this));

		if(animationName && animationDescription)
		{
			Sprite::play(this->sprite, animationDescription, (char*)animationName, Object::safeCast(this));
		}
		
		ASSERT(this->sprite, "Particle::addSprite: sprite not created");
	}
}

/**
 * Change the animation
 *
 * @param animationName		Char*
 */
void Particle::changeAnimation(const AnimationDescription* animationDescription, const char* animationName, bool force)
{
	if(!isDeleted(this->sprite) && animationName)
	{
		if(force || !Sprite::replay(this->sprite, animationDescription))
		{
			Sprite::play(this->sprite, animationDescription, (char*)animationName, Object::safeCast(this));
		}
	}
}

/**
 * Update
 *
 * @param elapsedTime
 * @param behavior
 * @return				Returns true if the particle's life span has elapsed
 */
bool Particle::update(uint32 elapsedTime, void (* behavior)(Particle particle))
{
	if(0 <= this->lifeSpan)
	{
		this->lifeSpan -= elapsedTime;

		if(0 > this->lifeSpan)
		{
			return true;
		}

		if(behavior)
		{
			behavior(this);
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
void Particle::synchronizeGraphics()
{
	NM_ASSERT(this->sprite, "Particle::synchronizeGraphics: null sprite");

	if(this->position.z != this->previousZ)
	{
		// calculate sprite's parallax
		Sprite::calculateParallax(this->sprite, this->position.z);

		this->previousZ = this->position.z;
	}

	// update sprite's 2D position
	Sprite::position(this->sprite, &this->position);
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void Particle::addForce(const Force* force __attribute__ ((unused)), uint32 movementType __attribute__ ((unused)))
{
}

/**
 * Set lifespan
 *
 * @param lifeSpan
 */
void Particle::setLifeSpan(int16 lifeSpan)
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
}

/**
 * Make Particle expire
 */
void Particle::expire()
{
	this->expired = true;

	Particle::hide(this);
}

/**
 * Make Particle invisible
 */
void Particle::hide()
{
	NM_ASSERT(this->sprite, "Particle::hide: null sprite");

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
void Particle::resume(const SpriteSpec* spriteSpec, const AnimationDescription* animationDescription, const char* animationName)
{
	Particle::addSprite(this, spriteSpec, animationDescription, animationName);

	// Force parallax computation
	this->previousZ = 0;

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
	this->expired = false;
}

/**
 * Setup
 */
void Particle::setup(int16 lifeSpan, const Vector3D* position, const Force* force, uint32 movementType, const AnimationDescription* animationDescription, const char* animationName, bool forceAnimation)
{
	Particle::reset(this);
	Particle::changeAnimation(this, animationDescription, animationName, forceAnimation);
	Particle::setLifeSpan(this, lifeSpan);
	Particle::changeMass(this);
	Particle::setPosition(this, position);

	if(force->x | force->y | force->z)
	{
		Particle::addForce(this, force, movementType);
	}
	
	Particle::show(this);
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

	int16 halfWidth = __PARTICLE_VISIBILITY_PADDING;
	int16 halfHeight = __PARTICLE_VISIBILITY_PADDING;

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
