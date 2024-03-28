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

#include <ParticleSystem.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Wireframe.h>

#include "Particle.h"


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
 */

void Particle::constructor(const ParticleSpec* particleSpec __attribute__((unused)), ParticleSystem creator __attribute__((unused)))
{
	// construct base Container
	Base::constructor();

	this->lifeSpan = 0;
	this->sprite = NULL;
	this->wireframe = NULL;
	this->expired = false;
}

/**
 * Class destructor
 */
void Particle::destructor()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::destroySprite(SpriteManager::getInstance(), this->sprite);
	}

	this->sprite = NULL;

	if(!isDeleted(this->wireframe))
	{
		delete this->wireframe;
	}

	this->wireframe = NULL;

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add a sprite
 *
 * @private
 */
void Particle::addSprite(const SpriteSpec* spriteSpec)
{
	if(NULL != spriteSpec && NULL == this->sprite)
	{
		// call the appropriate allocator to support inheritance
		this->sprite = SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)spriteSpec, SpatialObject::safeCast(this));

		ASSERT(this->sprite, "Particle::addSprite: sprite not created");
	}
}

/**
 * Add wireframe
 *
 * @private
 */
void Particle::addWireframe(const WireframeSpec* wireframeSpec)
{
	if(NULL != wireframeSpec && NULL == this->wireframe)
	{
		// call the appropriate allocator to support inheritance
		this->wireframe = ((Wireframe (*)(WireframeSpec*, SpatialObject)) wireframeSpec->allocator)((WireframeSpec*)wireframeSpec, SpatialObject::safeCast(this));

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

		if(NULL != behavior)
		{
			behavior(this);
		}
	}

	return false;
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
 * Set sprite's transparency
 */
void Particle::setTransparent(uint8 transparent)
{
	if(!isDeleted(this->sprite))
	{
		Sprite::setTransparent(this->sprite, transparent);
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
 * Resume
 */
void Particle::resume(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	Particle::addSprite(this, spriteSpec);
	Particle::addWireframe(this, wireframeSpec);
	Particle::changeAnimation(this, animationFunctions, animationName, true);
}

/**
 * Pause
 */
void Particle::suspend()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::destroySprite(SpriteManager::getInstance(), this->sprite);
	}

	this->sprite = NULL;

	if(!isDeleted(this->wireframe))
	{
		delete this->wireframe;
	}

	this->wireframe = NULL;
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
void Particle::setup(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName, bool forceAnimation)
{
	if(Particle::overrides(this, reset))
	{
		Particle::reset(this);
	}
	else
	{
		this->expired = false;
	}

	if(Particle::overrides(this, changeMass))
	{
		Particle::changeMass(this);
	}

	// TOOD: the preprocessor does't catch properly this override check with Particle 	
	if(SpatialObject::overrides(this, setPosition))
	{
		SpatialObject::setPosition(this, position);
	}
	else
	{
		this->transformation.position = *position;
	}

	Particle::addSprite(this, spriteSpec);
	Particle::addWireframe(this, wireframeSpec);
	Particle::changeAnimation(this, animationFunctions, animationName, forceAnimation);
	Particle::setLifeSpan(this, lifeSpan);

	if(NULL != force)
	{
		if(0 != force->x || 0 != force->y || 0 != force->z)
		{
			Particle::applySustainedForce(this, force, movementType);
		}
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
		Vector3D relativeGlobalPosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation.position), *_cameraInvertedRotation);
		pixelVector = Vector3D::projectToPixelVector(relativeGlobalPosition, Optics::calculateParallax(relativeGlobalPosition.z));
	}

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
