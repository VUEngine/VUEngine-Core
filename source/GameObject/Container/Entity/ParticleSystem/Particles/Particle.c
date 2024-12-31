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
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->lifeSpan = 0;
	this->visualComponent = NULL;
	this->expired = false;
}
//---------------------------------------------------------------------------------------------------------
void Particle::destructor()
{
	Particle::destroyGraphics(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool Particle::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
void Particle::setup(const VisualComponentSpec* visualComponentSpec, int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName)
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
	if(GameObject::overrides(this, setPosition))
	{
		Particle::setPosition(this, position);
	}
	else
	{
		this->transformation.position = *position;
	}

	Particle::addVisualComponent(this, visualComponentSpec);
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
void Particle::resume(const VisualComponentSpec* visualComponentSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	Particle::addVisualComponent(this, visualComponentSpec);
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
	if(!isDeleted(this->visualComponent))
	{
		VisualComponent::show(this->visualComponent);
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::hide()
{
	if(!isDeleted(this->visualComponent))
	{
		VisualComponent::hide(this->visualComponent);
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::setTransparency(uint8 transparency)
{
	if(!isDeleted(this->visualComponent))
	{
		VisualComponent::setTransparency(this->visualComponent, transparency);
	}
}
//---------------------------------------------------------------------------------------------------------
bool Particle::isVisible()
{
	PixelVector pixelVector;

	int16 halfWidth = __PARTICLE_VISIBILITY_PADDING;
	int16 halfHeight = __PARTICLE_VISIBILITY_PADDING;

	Vector3D relativeGlobalPosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation.position), *_cameraInvertedRotation);
	pixelVector = PixelVector::projectVector3D(relativeGlobalPosition, Optics::calculateParallax(relativeGlobalPosition.z));

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

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Particle::addVisualComponent(const VisualComponentSpec* visualComponentSpec)
{
	if(NULL != visualComponentSpec && NULL == this->visualComponent)
	{
		// call the appropriate allocator to support inheritance
		this->visualComponent = VisualComponent::safeCast(ComponentManager::addComponent(GameObject::safeCast(this), (ComponentSpec*)visualComponentSpec));
	}
}
//---------------------------------------------------------------------------------------------------------
void Particle::destroyGraphics()
{
	if(!isDeleted(this->visualComponent))
	{
		ComponentManager::removeComponent(GameObject::safeCast(this), Component::safeCast(this->visualComponent));
	}

	this->visualComponent = NULL;
}
//---------------------------------------------------------------------------------------------------------
void Particle::changeAnimation(const AnimationFunction** animationFunctions, const char* animationName)
{
	if(!isDeleted(this->visualComponent) && NULL != animationName)
	{
		if(!VisualComponent::replay(this->visualComponent, animationFunctions))
		{
			VisualComponent::play(this->visualComponent, animationFunctions, (char*)animationName, ListenerObject::safeCast(this), NULL);
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
