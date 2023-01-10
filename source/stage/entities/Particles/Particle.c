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
#include <Clock.h>
#include <Optics.h>


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
void Particle::constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan)
{
	// construct base Container
	Base::constructor();

	this->lifeSpan = lifeSpan;
	this->sprite = NULL;
	this->wireframe = NULL;
	this->position = Vector3D::zero();
	this->previousZ = 0;
	this->expired = false;
	this->transform = Particle::overrides(this, transform);

	Particle::addSprite(this, spriteSpec, particleSpec->animationFunctions, particleSpec->initialAnimation);
	Particle::addWireframe(this, wireframeSpec, particleSpec->animationFunctions, particleSpec->initialAnimation);
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

	if(!isDeleted(this->wireframe))
	{
		delete this->wireframe;
		this->wireframe = NULL;
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
void Particle::addSprite(const SpriteSpec* spriteSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	if(NULL != spriteSpec)
	{
		// call the appropriate allocator to support inheritance
		this->sprite = SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)spriteSpec, ListenerObject::safeCast(this));

		if(animationName && animationFunctions)
		{
			Sprite::play(this->sprite, animationFunctions, (char*)animationName, ListenerObject::safeCast(this));
		}
		
		ASSERT(this->sprite, "Particle::addSprite: sprite not created");
	}
}


/**
 * Add wireframe
 *
 * @private
 */
void Particle::addWireframe(const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions __attribute__((unused)), const char* animationName __attribute__((unused)))
{
	if(NULL != wireframeSpec)
	{
		// call the appropriate allocator to support inheritance
		this->wireframe = ((Wireframe (*)(WireframeSpec*)) wireframeSpec->allocator)((WireframeSpec*)wireframeSpec);
		Wireframe::setup(this->wireframe, &this->position, NULL, NULL, this->expired);

		NM_ASSERT(this->wireframe, "Particle::addWireframe: wireframe not created");
	}
}

/**
 * Change the animation
 *
 * @param animationName		Char*
 */
void Particle::changeAnimation(const AnimationFunction** animationFunctions, const char* animationName, bool force)
{
	if(!isDeleted(this->sprite) && NULL != animationName)
	{
		if(force || !Sprite::replay(this->sprite, animationFunctions))
		{
			Sprite::play(this->sprite, animationFunctions, (char*)animationName, ListenerObject::safeCast(this));
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
			Particle::expire(this);
			return true;
		}

		if(behavior)
		{
			behavior(this);
		}

		if(!isDeleted(this->sprite))
		{
			Sprite::updateAnimation(this->sprite);
		}
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
	if(!isDeleted(this->sprite))
	{
		if(this->position.z != this->previousZ)
		{
			// calculate sprite's parallax
			Sprite::calculateParallax(this->sprite, this->position.z);

			this->previousZ = this->position.z;
		}

		PixelVector position = Vector3D::transformToPixelVector(this->position);

		// update sprite's 2D position
		Sprite::setPosition(this->sprite, &position);
	}
}

/**
 * Add force
 *
 * @param force
 * @param movementType
 */
void Particle::applySustainedForce(const Vector3D* force __attribute__ ((unused)), uint32 movementType __attribute__ ((unused)))
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
void Particle::setMass(fixed_t mass __attribute__ ((unused)))
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
	if(!isDeleted(this->sprite))
	{
		Sprite::show(this->sprite);
	}

	if(!isDeleted(this->wireframe))
	{
		Wireframe::show(this->wireframe);
	}
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
	if(!isDeleted(this->sprite))
	{
		Sprite::hide(this->sprite);
	}

	if(!isDeleted(this->wireframe))
	{
		Wireframe::hide(this->wireframe);
	}
}

/**
 * Can move over axis?
 *
 * @param acceleration
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
bool Particle::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
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
void Particle::resume(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	Particle::addSprite(this, spriteSpec, animationFunctions, animationName);
	Particle::addWireframe(this, wireframeSpec, animationFunctions, animationName);

	// Force parallax computation
	this->previousZ = 0;
}

/**
 * Pause
 */
void Particle::suspend()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::disposeSprite(SpriteManager::getInstance(), this->sprite);

		this->sprite = NULL;
	}

	if(!isDeleted(this->wireframe))
	{
		delete this->wireframe;
		this->wireframe = NULL;
	}
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
void Particle::setup(int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName, bool forceAnimation)
{
	Particle::reset(this);
	Particle::changeAnimation(this, animationFunctions, animationName, forceAnimation);
	Particle::setLifeSpan(this, lifeSpan);
	Particle::changeMass(this);
	Particle::setPosition(this, position);

	if(!isDeleted(this->wireframe))
	{
		Wireframe::setup(this->wireframe, &this->position, NULL, NULL, false);
	}

	if(force->x | force->y | force->z)
	{
		Particle::applySustainedForce(this, force, movementType);
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
	PixelVector pixelVector;

	int16 halfWidth = __PARTICLE_VISIBILITY_PADDING;
	int16 halfHeight = __PARTICLE_VISIBILITY_PADDING;

	if(!isDeleted(this->sprite))
	{
		pixelVector = Sprite::getDisplacedPosition(this->sprite);

		Texture texture = Sprite::getTexture(this->sprite);

		if(!isDeleted(texture))
		{
			halfWidth = Texture::getCols(texture) << 2;
			halfHeight = Texture::getRows(texture) << 2;
		}
	}
	else
	{
		Vector3D relativeGlobalPosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->position), *_cameraInvertedRotation);
		pixelVector = Vector3D::projectToPixelVector(relativeGlobalPosition, Optics::calculateParallax(relativeGlobalPosition.z));
	}

	extern const CameraFrustum* _cameraFrustum;

	// check x visibility
	if(pixelVector.x + halfWidth < _cameraFrustum->x0 || pixelVector.x - halfWidth > _cameraFrustum->x1)
	{
		return false;
	}

	// check y visibility
	if(pixelVector.y + halfHeight < _cameraFrustum->y0 || pixelVector.y - halfHeight > _cameraFrustum->y1)
	{
		return false;
	}

	// check z visibility
	if(pixelVector.z > __SCREEN_DEPTH || pixelVector.z < -(__SCREEN_DEPTH >> 1))
	{
		return false;
	}

	return true;
}
