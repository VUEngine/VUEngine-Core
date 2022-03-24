/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BODY_H_
#define BODY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// movement type
#define	__NO_MOVEMENT				0x00
#define	__UNIFORM_MOVEMENT			0x01
#define	__ACCELERATED_MOVEMENT		0x20

#define __MAXIMUM_FRICTION_COEFFICIENT			__I_TO_FIX10_6(1)


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Movement result
 *
 * @memberof Body
 */
typedef struct MovementResult
{
	uint16 axisStoppedMovement;
	uint16 axisOfAcceleratedBouncing;
	uint16 axisOfChangeOfMovement;
	uint16 axisOfChangeOfDirection;

} MovementResult;

// defines a body
typedef struct PhysicalSpecification
{
	/// mass
	fix10_6 mass;
	/// friction coefficient
	fix10_6 frictionCoefficient;
	/// bounciness
	fix10_6 bounciness;
	/// maximum velocity
	Velocity maximumVelocity;
	/// maximum speed
	fix10_6 maximumSpeed;

} PhysicalSpecification;

typedef const PhysicalSpecification PhysicalSpecificationROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class Body : Object
{
	// owner
	SpatialObject owner;
	// direction
	Force weight;
	// direction
	Force externalForce;
	// friction surrounding object
	Force friction;
	// total normal forces applied to the body
	Force totalNormal;
	// List of normal forces affecting the body
	VirtualList normals;
	// spatial position
	Vector3D position;
	// velocity on each instance
	Velocity velocity;
	// direction
	Direction3D direction;
	// speed
	fix10_6 speed;
	// maximum velocity on each instance
	Velocity maximumVelocity;
	// maximum speed
	fix10_6 maximumSpeed;
	// acceleration structure
	Acceleration acceleration;
	// bounciness
	fix10_6 bounciness;
	// friction coefficient
	fix10_6 frictionCoefficient;
	// friction coefficient of the surroundings
	fix10_6 surroundingFrictionCoefficient;
	// friction force magnitude
	fix10_6 totalFrictionCoefficient;
	// total friction force magnitude
	fix10_6 frictionForceMagnitude;
	// mass
	fix10_6 mass;
	// movement type on each axis
	MovementType movementType;
	// axis that are subject to gravity
	uint16 axisSubjectToGravity;
	// raise flag to make the body active
	bool active;
	// raise flag to update body's physics
	bool awake;
	// Flag to indicate if the body changed direction during the last frame
	bool changedDirection;
	// Flag to enable messages
	bool sendMessages;
	// Delete flag
	bool destroy;
	// flag to clear the external force after each update
	bool clearExternalForce;

	/// @publicsection
	static void setCurrentElapsedTime(fix10_6 currentElapsedTime);
	static void setCurrentWorldFrictionCoefficient(fix10_6 _currentWorldFriction);
	static void setCurrentGravity(const Acceleration* currentGravity);
	void constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity);
	void applySustainedForcerce(const Force* force);
	uint8 applyForce(const Force* force);
	uint8 applyGravity(uint16 axis);
	void bounce(Object bounceReferent, Vector3D bouncingPlaneNormal, fix10_6 frictionCoefficient, fix10_6 bounciness);
	void clearAcceleration(uint16 axis);
	void clearExternalForce();
	Acceleration getAcceleration();
	Force getAppliedForce();
	uint16 getaxisSubjectToGravity();
	fix10_6 getBounciness();
	Vector3D getLastDisplacement();
	fix10_6 getMass();
	MovementType getMovementType();
	SpatialObject getOwner();
	const Vector3D* getPosition();
	Velocity getVelocity();
	void setVelocity(Velocity* velocity);
	const Direction3D* getDirection3D();
	fix10_6 getSpeed();
	fix10_6_ext getSpeedSquare();
	void modifyVelocity(const Velocity* multiplier);
	bool isActive();
	bool isAwake();
	uint16 getMovementOnAllAxis();
	void setMovementType(int32 movementType, uint16 axis);
	void moveAccelerated(uint16 axis);
	void moveUniformly(Velocity velocity);
	void setActive(bool active);
	void setAxisSubjectToGravity(uint16 axisSubjectToGravity);
	void setBounciness(fix10_6 bounciness);
	Force getNormal();
	Force getLastNormalDirection();
	void reset();
	void clearNormal(Object referent);
	fix10_6 getFrictionForceMagnitude();
	fix10_6 getFrictionCoefficient();
	void setFrictionCoefficient(fix10_6 frictionCoefficient);
	void setSurroundingFrictionCoefficient(fix10_6 surroundingFrictionCoefficient);
	void setMass(fix10_6 mass);
	void setOwner(SpatialObject owner);
	void setPosition(const Vector3D* position, SpatialObject caller);
	uint16 stopMovement(uint16 axis);
	void takeHitFrom(Body other);
	void setMaximumVelocity(Velocity maximumVelocity);
	Velocity getMaximumVelocity();
	void setMaximumSpeed(fix10_6 maximumSpeed);
	fix10_6 getMaximumSpeed();
	void print(int32 x, int32 y);
	MovementResult updateMovement();
	bool changedDirection();
	void sendMessages(bool value);

	virtual void update();
}


#endif
