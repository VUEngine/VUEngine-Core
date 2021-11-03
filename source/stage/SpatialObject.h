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

#include <Object.h>
#include <VirtualList.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class SpatialObject : Object
{
	/// @publicsection
	void constructor();
	void destructor();
	virtual bool isSubjectToGravity(Acceleration gravity);
	virtual fix10_6 getRadius();
	virtual fix10_6 getWidth();
	virtual fix10_6 getHeight();
	virtual fix10_6 getDepth();
	virtual const Vector3D* getPosition();
	virtual void setPosition(const Vector3D* position);
	virtual const Rotation* getRotation();
	virtual void setRotation(const Rotation* rotation);
	virtual const Scale* getScale();
	virtual void setScale(const Scale* scale);
	virtual fix10_6 getBounciness();
	virtual fix10_6 getFrictionCoefficient();
	virtual Velocity getVelocity();
	virtual fix10_6 getSpeed();
	virtual bool enterCollision(const CollisionInformation* collisionInformation);
	virtual bool updateCollision(const CollisionInformation* collisionInformation);
	virtual void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual void collidingShapeOwnerDestroyed(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual uint32 getInGameType();
}


#endif
