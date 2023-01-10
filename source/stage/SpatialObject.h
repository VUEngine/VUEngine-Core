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
#include <VirtualList.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class SpatialObject : ListenerObject
{
	/// @publicsection
	void constructor();
	void destructor();
	virtual bool isSubjectToGravity(Vector3D gravity);
	virtual fixed_t getRadius();
	virtual fixed_t getWidth();
	virtual fixed_t getHeight();
	virtual fixed_t getDepth();
	virtual const Vector3D* getPosition();
	virtual void setPosition(const Vector3D* position);
	virtual const Rotation* getRotation();
	virtual void setRotation(const Rotation* rotation);
	virtual const Scale* getScale();
	virtual void setScale(const Scale* scale);
	virtual fixed_t getBounciness();
	virtual fixed_t getFrictionCoefficient();
	virtual const Vector3D* getVelocity();
	virtual fixed_t getSpeed();
	virtual bool enterCollision(const CollisionInformation* collisionInformation);
	virtual bool updateCollision(const CollisionInformation* collisionInformation);
	virtual void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual void collidingShapeOwnerDestroyed(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual uint32 getInGameType();
}


#endif
