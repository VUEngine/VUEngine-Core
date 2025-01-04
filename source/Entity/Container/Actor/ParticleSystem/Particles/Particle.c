/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <ParticleSystem.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Telegram.h>
#include <Wireframe.h>
#include <WireframeManager.h>

#include "Particle.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __PARTICLE_VISIBILITY_PADDING	8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::constructor(const ParticleSpec* particleSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->particleSpec = particleSpec;
	this->lifeSpan = 0;
	this->expired = false;
	this->body = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::destructor()
{
	this->body = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Particle::getInGameType()
{
	return this->particleSpec->inGameType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Particle::collisionStarts(const CollisionInformation* collisionInformation)
{
	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(NULL != this->particleSpec->onCollisionAnimation)
		{
			Particle::playAnimation
			(
				this, ((ParticleSpec*)this->particleSpec)->animationFunctions, this->particleSpec->onCollisionAnimation
			);
		}
	}

	return Base::collisionStarts(this, collisionInformation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::setup
(
	const ComponentSpec* visualComponentSpec, const ComponentSpec* physicsComponentSpec, const ComponentSpec* colliderComponentSpec, 
	int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, 
	const char* animationName
)
{
	this->expired = false;
	this->lifeSpan = lifeSpan;

	// Is this needed? It can be quite heavy.
	// Particle::resetComponents(this);

	if(NULL != visualComponentSpec)
	{
		Particle::removeComponents(this, kSpriteComponent);
		Particle::removeComponents(this, kWireframeComponent);
		Particle::addComponent(this, visualComponentSpec);
	}

	if(NULL != physicsComponentSpec && NULL == this->body)
	{
		Particle::removeComponents(this, kPhysicsComponent);
		this->body = Body::safeCast(Particle::addComponent(this, physicsComponentSpec));
	}

	if(NULL != colliderComponentSpec)
	{
		Particle::removeComponents(this, kColliderComponent);
		Particle::addComponent(this, colliderComponentSpec);
		Particle::registerCollisions(this, false);
	}

	Particle::playAnimation(this, animationFunctions, animationName);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, position, GameObject::safeCast(this));
	}

	this->transformation.position = *position;

	if(NULL != force)
	{
		if(0 != force->x || 0 != force->y || 0 != force->z)
		{
			Particle::applyForce(this, force, movementType);
		}
	}

	Particle::show(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::resume(const VisualComponentSpec* visualComponentSpec, const AnimationFunction** animationFunctions, const char* animationName)
{
	Particle::addComponent(this, (ComponentSpec*)visualComponentSpec);
	Particle::playAnimation(this, animationFunctions, animationName);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::suspend()
{
	Particle::removeComponents(this, kSpriteComponent);
	Particle::removeComponents(this, kWireframeComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::expire()
{
	this->expired = true;

	Particle::hide(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Particle::isVisible()
{
	PixelVector pixelVector;

	int16 halfWidth = __PARTICLE_VISIBILITY_PADDING;
	int16 halfHeight = __PARTICLE_VISIBILITY_PADDING;

	Vector3D relativeGlobalPosition = 
		Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation.position), *_cameraInvertedRotation);
	
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::playAnimation(const AnimationFunction** animationFunctions, const char* animationName)
{
	SpriteManager::propagateCommand(
		SpriteManager::getInstance(), 
		cVisualComponentCommandPlay, 
		GameObject::safeCast(this), 
		animationFunctions, 
		animationName, 
		NULL, 
		NULL
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Particle::update(uint32 elapsedTime, void (* behavior)(Particle particle))
{
	if(0 <= this->lifeSpan)
	{
		this->lifeSpan -= elapsedTime;

		if(0 > this->lifeSpan)
		{
			Particle::expire(this);

			if(!isDeleted(this->body))
			{
				Body::stopMovement(this->body, __ALL_AXIS);
			}

			return true;
		}

		if(NULL != behavior)
		{
			behavior(this);
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Particle::applyForce(const Vector3D* force __attribute__ ((unused)), uint32 movementType __attribute__ ((unused)))
{
	if(isDeleted(this->body))
	{
		return;
	}

	if(__UNIFORM_MOVEMENT == movementType)
	{
		fixed_t mass = Body::getMass(this->body);

		Vector3D acceleration =
		{
			force->x,
			force->y,
			force->z
		};

		if(mass && __1I_FIXED != mass)
		{
			acceleration.x = __FIXED_DIV(acceleration.x, mass);
			acceleration.y = __FIXED_DIV(acceleration.y, mass);
			acceleration.z = __FIXED_DIV(acceleration.z, mass);
		}

		Vector3D velocity =
		{
			acceleration.x,
			acceleration.y,
			acceleration.z
		};

		Body::setVelocity(this->body, &velocity);
	}
	else
	{
		Body::applyForce(this->body, force);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
