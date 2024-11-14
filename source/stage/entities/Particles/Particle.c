/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ParticleSystem.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Wireframe.h>
#include <WireframeManager.h>

#include "Particle.h"


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __PARTICLE_VISIBILITY_PADDING	8


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Particle::constructor(const ParticleSpec* particleSpec __attribute__((unused)))
{
	// construct base Container
	Base::constructor();

	this->lifeSpan = 0;
	this->sprite = NULL;
	this->wireframe = NULL;
	this->expired = false;
}
//---------------------------------------------------------------------------------------------------------
void Particle::destructor()
{
	Particle::destroyGraphics(this);

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Particle::setup(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName)
{
	if(Particle::overrides(this, reset))
	{
		Particle::reset(this);
	}
	else
	{
		this->expired = false;
	}

	if(Particle::overrides(this, configureMass))
	{
		Particle::configureMass(this);
	}

	// TOOD: the preprocessor does't catch properly this override check with Particle 	
	if(SpatialObject::overrides(this, setPosition))
	{
		Particle::setPosition(this, position);
	}
	else
	{
		this->transformation.position = *position;
	}

	Particle::addSprite(this, spriteSpec);
	Particle::addWireframe(this, wireframeSpec);
	Particle::changeAnimation(this, animationFunctions, animationName);
	Particle::setLifeSpan(this, lifeSpan);

	if(NULL != force)
	{
		if(0 != force->x || 0 != force->y || 0 != force->z)
		{
			Particle::applyForce(this, force, movementType);
		}
	}

	Particle::show(this);
}
//---------------------------------------------------------------------------------------------------------
void Particle::resume(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	Particle::addSprite(this, spriteSpec);
	Particle::addWireframe(this, wireframeSpec);
	Particle::changeAnimation(this, animationFunctions, animationName);
}
//---------------------------------------------------------------------------------------------------------
void Particle::suspend()
{
	Particle::destroyGraphics(this);
}
//---------------------------------------------------------------------------------------------------------
void Particle::expire()
{
	this->expired = true;

	Particle::hide(this);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void Particle::setTransparent(uint8 transparent)
{
	if(!isDeleted(this->sprite))
	{
		Sprite::setTransparent(this->sprite, transparent);
	}

	if(!isDeleted(this->wireframe))
	{
		Wireframe::setTransparent(this->wireframe, transparent);
	}
}
//---------------------------------------------------------------------------------------------------------
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
		pixelVector = PixelVector::projectVector3D(relativeGlobalPosition, Optics::calculateParallax(relativeGlobalPosition.z));
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
//---------------------------------------------------------------------------------------------------------
void Particle::reset()
{
	this->expired = false;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void Particle::applyForce(const Vector3D* force __attribute__ ((unused)), uint32 movementType __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void Particle::configureMass()
{}
//---------------------------------------------------------------------------------------------------------
bool Particle::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Particle::addSprite(const SpriteSpec* spriteSpec)
{
	if(NULL != spriteSpec && NULL == this->sprite)
	{
		// call the appropriate allocator to support inheritance
		this->sprite = SpriteManager::createSprite(SpriteManager::getInstance(), SpatialObject::safeCast(this), (SpriteSpec*)spriteSpec);

		ASSERT(this->sprite, "Particle::addSprite: sprite not created");
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::addWireframe(const WireframeSpec* wireframeSpec)
{
	if(NULL != wireframeSpec && NULL == this->wireframe)
	{
		this->wireframe = WireframeManager::createWireframe(WireframeManager::getInstance(), wireframeSpec, SpatialObject::safeCast(this));

		NM_ASSERT(this->wireframe, "Particle::addWireframe: wireframe not created");
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::destroyGraphics()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::destroySprite(SpriteManager::getInstance(), this->sprite);
	}

	this->sprite = NULL;

	if(!isDeleted(this->wireframe))
	{
		WireframeManager::destroyWireframe(WireframeManager::getInstance(), this->wireframe);
	}

	this->wireframe = NULL;
}
//---------------------------------------------------------------------------------------------------------
void Particle::changeAnimation(const AnimationFunction** animationFunctions, const char* animationName)
{
	if(!isDeleted(this->sprite) && NULL != animationName)
	{
		if(!Sprite::replay(this->sprite, animationFunctions))
		{
			Sprite::play(this->sprite, animationFunctions, (char*)animationName, ListenerObject::safeCast(this));
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::setLifeSpan(int16 lifeSpan)
{
	this->lifeSpan = lifeSpan;
}
//---------------------------------------------------------------------------------------------------------
void Particle::setMass(fixed_t mass __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
