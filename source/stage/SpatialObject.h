/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPATIAL_OBJECT_H_
#define SPATIAL_OBJECT_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>
#include <Collider.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class SpatialObject
///
/// Inherits from ListenerObject
///
/// Defines objects that occupy a place in 3D space.
/// @ingroup stage
class SpatialObject : ListenerObject
{
	/// @protectedsection
	
	/// 3D transformation
	Transformation transformation;
	
	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Retrieve the object's transformation.
	/// @return Pointer to the object's 3D transformation
	const Transformation* getTransformation();

	/// Retrieve the object's position.
	/// @return Pointer to the object's 3D vector defining its position
	const Vector3D* getPosition();

	/// Retrieve the object's rotation.
	/// @return Pointer to the object's 3D rotation
	const Rotation* getRotation();

	/// Retrieve the object's scale.
	/// @return Pointer to the object's 3D
	const Scale* getScale();

	/// Retrieve the object's radius.
	/// @return Radius
	virtual fixed_t getRadius();

	/// Retrieve the object's velocity vector.
	/// @return Pointer to the direction towards which the object is moving
	virtual const Vector3D* getVelocity();

	/// Retrieve the object's current speed (velocity vector's magnitude).
	/// @return Object's current speed (velocity vector's magnitude)
	virtual fixed_t getSpeed();

	/// Retrieve the object's bounciness factor.
	/// @return Object's bounciness factor
	virtual fixed_t getBounciness();

	/// Retrieve the object's friction coefficient.
	/// @return Object's friction coefficient
	virtual fixed_t getFrictionCoefficient();

	/// Set the object's position.
	/// @param position: 3D vector defining the object's new position
	virtual void setPosition(const Vector3D* position);

	/// Set the object's rotation.
	/// @param rotation: Rotation
	virtual void setRotation(const Rotation* rotation);

	/// Set the object's scale.
	/// @param scale: Scale
	virtual void setScale(const Scale* scale);

	/// Set the direction towards which the object must move.
	/// @param direction: Pointer to a direction vector
	virtual void setDirection(const Vector3D* direction);

	/// Retrieve the direction towards which the object is moving.
	/// @return Pointer to the direction towards which the object is moving
	virtual const Vector3D* getDirection();

	/// Check if the object is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the object; false otherwise
	virtual bool isSubjectToGravity(Vector3D gravity);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	virtual uint32 getInGameType();

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	/// @return True if the collider must keep track of the collision to detect if it persists and when it ends; false otherwise
	virtual bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Process a going on collision detected by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	virtual void collisionPersists(const CollisionInformation* collisionInformation);

	/// Process when a previously detected collision by one of the component colliders stops.
	/// @param collisionInformation: Information struct about the collision to resolve
	virtual void collisionEnds(const CollisionInformation* collisionInformation);
}


#endif
