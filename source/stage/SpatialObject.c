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
#include <SpatialObject.h>
#include <Shape.h>
#include <VirtualList.h>


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
	if(this->events)
	{
		SpatialObject::fireEvent(this, kEventSpatialObjectDeleted);
		NM_ASSERT(!isDeleted(this), "SpatialObject::destructor: deleted this during kEventSpatialObjectDeleted");
	}

	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	Base::destructor();
}

// defaults to true
bool SpatialObject::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return false;
}

fix10_6 SpatialObject::getRadius()
{
	fix10_6 width = SpatialObject::getWidth(this);
	fix10_6 height = SpatialObject::getHeight(this);
	fix10_6 depth = SpatialObject::getDepth(this);

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

fix10_6 SpatialObject::getWidth()
{
	return 0;
}

fix10_6 SpatialObject::getHeight()
{
	return 0;
}

fix10_6 SpatialObject::getDepth()
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
fix10_6 SpatialObject::getBounciness()
{
	return 0;
}

// get friction
fix10_6 SpatialObject::getFrictionCoefficient()
{
	return 0;
}

// get velocity
Velocity SpatialObject::getVelocity()
{
	return Vector3D::zero();
}

fix10_6 SpatialObject::getSpeed()
{
	return 0;
}

fix10_6 SpatialObject::getMaximumSpeed()
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
