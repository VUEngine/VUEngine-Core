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

#include <SpatialObject.h>
#include <Shape.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void SpatialObject::constructor()
{
	// construct base object
	Base::constructor();
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
	static Vector3D position =
	{
		0, 0, 0
	};

	return &position;
}

void SpatialObject::setPosition(const Vector3D* position __attribute__ ((unused)))
{}

const Rotation* SpatialObject::getRotation()
{
	static Rotation rotation =
	{
		0, 0, 0
	};

	return &rotation;
}

void SpatialObject::setRotation(const Rotation* rotation __attribute__ ((unused)))
{}

const Scale* SpatialObject::getScale()
{
	static Scale scale =
	{
		__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9
	};

	return &scale;
}
void SpatialObject::setScale(const Scale* scale __attribute__ ((unused)))
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

void SpatialObject::exitCollision(Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore __attribute__ ((unused)), bool isShapeImpenetrable __attribute__ ((unused)))
{}

void SpatialObject::collidingShapeOwnerDestroyed(Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore __attribute__ ((unused)), bool isShapeImpenetrable __attribute__ ((unused)))
{}

/**
 * Retrieve shapes list
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
