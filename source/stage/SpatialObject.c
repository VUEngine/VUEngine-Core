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

#include <string.h>

#include "SpatialObject.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void SpatialObject::destructor()
{
	// destroy the super SpatialObject
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
const Transformation* SpatialObject::getTransformation()
{
	return &this->transformation;
}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getPosition()
{
	return &this->transformation.position;
}
//---------------------------------------------------------------------------------------------------------
const Rotation* SpatialObject::getRotation()
{
	return &this->transformation.rotation;
}
//---------------------------------------------------------------------------------------------------------
const Scale* SpatialObject::getScale()
{
	return &this->transformation.scale;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getRadius()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getVelocity()
{
	static Vector3D dummyVelocity = {0, 0, 0};

	return &dummyVelocity;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getSpeed()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getBounciness()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getFrictionCoefficient()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setRotation(const Rotation* rotation)
{
	this->transformation.rotation = *rotation;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setScale(const Scale* scale)
{
	this->transformation.scale = *scale;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setDirection(const Vector3D* direction __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getDirection()
{
	static Vector3D dummyDirection = {0, 0, 0};

	return &dummyDirection;
}
//---------------------------------------------------------------------------------------------------------
bool SpatialObject::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
uint32 SpatialObject::getInGameType()
{
	return kTypeNone;
}
//---------------------------------------------------------------------------------------------------------
bool SpatialObject::collisionStarts(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::collisionPersists(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::collisionEnds(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
