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

#include <string.h>

#include <Collider.h>

#include "SpatialObject.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void SpatialObject::constructor()
{
	// construct base object
	Base::constructor();

	// set position
	this->transformation.position = Vector3D::zero();

	// set rotation
	this->transformation.rotation = Rotation::zero();

	// set scale
	this->transformation.scale = Scale::unit();
}

// class's destructor
void SpatialObject::destructor()
{
	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	Base::destructor();
}

// defaults to true
bool SpatialObject::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}

fixed_t SpatialObject::getRadius()
{
	fixed_t width = SpatialObject::getWidth(this);
	fixed_t height = SpatialObject::getHeight(this);
	fixed_t depth = SpatialObject::getDepth(this);

	if(width > height)
	{
		if(width > depth)
		{
			return width >> 1;
		}
		else
		{
			return depth >> 1;
		}

	}
	else if(height > depth)
	{
		return height >> 1;
	}
	else
	{
		return depth >> 1;
	}

	return 0;
}

/**
 * Retrieve transformation
 *
 * @return		Pointer to Transformation
 */
const Transformation* SpatialObject::getTransformation()
{
	return &this->transformation;
}

fixed_t SpatialObject::getWidth()
{
	return 0;
}

fixed_t SpatialObject::getHeight()
{
	return 0;
}

fixed_t SpatialObject::getDepth()
{
	return 0;
}

const Vector3D* SpatialObject::getPosition()
{
	return &this->transformation.position;
}

void SpatialObject::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;
}

const Rotation* SpatialObject::getRotation()
{
	return &this->transformation.rotation;

}

void SpatialObject::setRotation(const Rotation* rotation)
{
	this->transformation.rotation = *rotation;
}

const Scale* SpatialObject::getScale()
{
	return &this->transformation.scale;
}

void SpatialObject::setScale(const Scale* scale)
{
	this->transformation.scale = *scale;
}

const Size* SpatialObject::getSize()
{
	static Size size =
	{
		0, 0, 0
	};

	return &size;
}

void SpatialObject::setDirection(const Vector3D* direction __attribute__ ((unused)))
{}

// get bounciness
fixed_t SpatialObject::getBounciness()
{
	return 0;
}

// get friction
fixed_t SpatialObject::getFrictionCoefficient()
{
	return 0;
}

// get velocity
const Vector3D* SpatialObject::getVelocity()
{
	return NULL;
}

const Vector3D* SpatialObject::getDirection()
{
	return NULL;
}

fixed_t SpatialObject::getSpeed()
{
	return 0;
}

fixed_t SpatialObject::getMaximumSpeed()
{
	return 0;
}

bool SpatialObject::enterCollision(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	return false;
}

bool SpatialObject::updateCollision(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	return false;
}

void SpatialObject::exitCollision(Collider collider __attribute__ ((unused)), Collider colliderNotCollidingAnymore __attribute__ ((unused)), bool isColliderImpenetrable __attribute__ ((unused)))
{}

void SpatialObject::otherColliderOwnerDestroyed(Collider collider __attribute__ ((unused)), Collider colliderNotCollidingAnymore __attribute__ ((unused)), bool isColliderImpenetrable __attribute__ ((unused)))
{}

/**
 * Retrieve colliders list
 *
 * @memberof	SpatialObject
 * @public
 *
 * @return		no type
 */
uint32 SpatialObject::getInGameType()
{
	return kTypeNone;
}
