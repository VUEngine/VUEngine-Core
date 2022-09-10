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

#include <ListenerObject.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// movement type
#define	__NO_MOVEMENT				0x00
#define	__UNIFORM_MOVEMENT			0x01
#define	__ACCELERATED_MOVEMENT		0x20

#define __BODY_MIN_MASS			__F_TO_FIXED(0.01f)
#define __BODY_MAX_MASS			__I_TO_FIXED(1)


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
	fixed_t mass;
	/// friction coefficient
	fixed_t frictionCoefficient;
	/// bounciness
	fixed_t bounciness;
	/// maximum velocity
	Velocity maximumVelocity;
	/// maximum speed
	fixed_t maximumSpeed;

} PhysicalSpecification;

typedef const PhysicalSpecification PhysicalSpecificationROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct Vector3DPlus
{
	fix7_9_ext x;
	fix7_9_ext y;
	fix7_9_ext z;

} Vector3DPlus;


/// @ingroup physics
class Body : ListenerObject
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
	Vector3DPlus internalPosition;
	// velocity on each instance
	Velocity velocity;
	Vector3DPlus internalVelocity;
	// direction
	Direction3D direction;
	// speed
	fixed_t speed;
	// maximum velocity on each instance
	Velocity maximumVelocity;
	// maximum speed
	fixed_t maximumSpeed;
	// acceleration structure
	Vector3DFlag accelerating;
	// bounciness
	fixed_t bounciness;
	// friction coefficient
	fixed_t frictionCoefficient;
	// friction coefficient of the surroundings
	fixed_t surroundingFrictionCoefficient;
	// friction force magnitude
	fixed_t totalFrictionCoefficient;
	// total friction force magnitude
	fixed_t frictionForceMagnitude;
	// mass
	fixed_t mass;
	// movement type on each axis
	MovementType movementType;
	// axis that are subject to gravity
	uint16 axisSubjectToGravity;
	// shift elapsed time
	int8 elapsedTimeModifier;
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
	// flag that determines the logic for stoping the body
	bool movesIndependentlyOnEachAxis;

	/// @publicsection
	static void setCurrentElapsedTime(fix7_9_ext currentElapsedTime);
	static fix7_9_ext getCurrentElapsedTime();
	static void setCurrentWorldFrictionCoefficient(fixed_t _currentWorldFriction);
	static void setCurrentGravity(const Acceleration* currentGravity);
	static const Acceleration* getCurrentGravity();
	static fixed_t computeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed);

	void constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity);
	void applySustainedForce(const Force* force);
	uint8 applyForce(const Force* force);
	uint8 applyGravity(uint16 axis);
	void bounce(ListenerObject bounceReferent, Vector3D bouncingPlaneNormal, fixed_t frictionCoefficient, fixed_t bounciness);
	void clearAcceleration(uint16 axis);
	void clearExternalForce();
	Vector3DFlag getAccelerationState();
	Force getAppliedForce();
	uint16 getaxisSubjectToGravity();
	fixed_t getBounciness();
	Vector3D getLastDisplacement();
	Acceleration getGravity();
	Force getFriction();
	fixed_t getMass();
	MovementType getMovementType();
	SpatialObject getOwner();
	const Vector3D* getPosition();
	Velocity getVelocity();
	void setVelocity(Velocity* velocity);
	const Direction3D* getDirection3D();
	fixed_t getSpeed();
	fixed_ext_t getSpeedSquare();
	void modifyVelocity(const Velocity* multiplier);
	bool isActive();
	bool isAwake();
	uint16 getMovementOnAllAxis();
	void setMovementType(int32 movementType, uint16 axis);
	void moveAccelerated(uint16 axis);
	void moveUniformly(Velocity velocity);
	void setActive(bool active);
	void setAxisSubjectToGravity(uint16 axisSubjectToGravity);
	void setBounciness(fixed_t bounciness);
	void setElapsedTimeModifier(int8 elapsedTimeModifier);
	Force getNormal();
	Force getLastNormalDirection();
	void reset();
	void clearNormal(ListenerObject referent);
	fixed_t getFrictionForceMagnitude();
	fixed_t getFrictionCoefficient();
	void setFrictionCoefficient(fixed_t frictionCoefficient);
	void setSurroundingFrictionCoefficient(fixed_t surroundingFrictionCoefficient);
	void setMass(fixed_t mass);
	void setOwner(SpatialObject owner);
	void setPosition(const Vector3D* position, SpatialObject caller);
	uint16 stopMovement(uint16 axis);
	void takeHitFrom(Body other);
	void setMaximumVelocity(Velocity maximumVelocity);
	Velocity getMaximumVelocity();
	void setMaximumSpeed(fixed_t maximumSpeed);
	fixed_t getMaximumSpeed();
	void print(int32 x, int32 y);
	MovementResult updateMovement();
	bool changedDirection();
	void sendMessages(bool value);

	virtual void update();
}


#endif
