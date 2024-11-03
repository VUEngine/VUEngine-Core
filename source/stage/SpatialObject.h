/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPATIAL_OBJECT_H_
#define SPATIAL_OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Collider.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class SpatialObject : ListenerObject
{
	// 3D transformation
	Transformation transformation;
	
	/// @publicsection
	void constructor();
	void destructor();
	const Transformation* getTransformation();
	const Vector3D* getPosition();
	const Rotation* getRotation();
	const Scale* getScale();
	virtual bool isSubjectToGravity(Vector3D gravity);
	virtual fixed_t getRadius();
	virtual fixed_t getWidth();
	virtual fixed_t getHeight();
	virtual fixed_t getDepth();
	virtual void setPosition(const Vector3D* position);
	virtual void setRotation(const Rotation* rotation);
	virtual void setScale(const Scale* scale);
	virtual const Vector3D* getDirection();
	virtual void setDirection(const Vector3D* direction);
	virtual const Size* getSize();
	virtual fixed_t getBounciness();
	virtual fixed_t getFrictionCoefficient();
	virtual const Vector3D* getVelocity();
	virtual fixed_t getSpeed();
	virtual bool collisionStarts(const CollisionInformation* collisionInformation);
	virtual bool collisionPersists(const CollisionInformation* collisionInformation);
	virtual void collisionEnds(const CollisionInformation* collisionInformation);
	virtual uint32 getInGameType();
}


#endif
