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

class SpatialObject;

/**
 * Movement result
 *
 * @memberof Body
 */
typedef struct MovementResult
{
	uint16 axisStoppedMovement;
	uint16 axisOfAcceleratedBouncing;

} MovementResult;

// defines a body
typedef struct PhysicalProperties
{
	/// mass
	fixed_t mass;
	/// friction coefficient
	fixed_t frictionCoefficient;
	/// bounciness
	fixed_t bounciness;
	/// maximum velocity
	Vector3D maximumVelocity;
	/// maximum speed
	fixed_t maximumSpeed;

} PhysicalProperties;

typedef const PhysicalProperties PhysicalPropertiesROMSpec;

typedef struct Vector3DPlus
{
	fix7_9_ext x;
	fix7_9_ext y;
	fix7_9_ext z;

} Vector3DPlus;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------


/// @ingroup physics
class Body : ListenerObject
{
	// owner
	SpatialObject owner;
	// gravity
	Vector3D gravity;
	// applied external force
	Vector3D externalForce;
	// friction surrounding object
	Vector3D friction;
	// total normal forces applied to the body
	Vector3D totalNormal;
	// List of normal forces affecting the body
	VirtualList normals;
	// spatial position
	Vector3D position;
	Vector3DPlus internalPosition;
	// velocity on each instance
	Vector3D velocity;
	Vector3DPlus internalVelocity;
	// direction
	Vector3D direction;
	// speed
	fixed_t speed;
	// maximum velocity on each instance
	Vector3D maximumVelocity;
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
	int8 skipCycles;
	int8 skipedCycles;
	// raise flag to make the body active
	bool active;
	// raise flag to update body's physics
	bool awake;
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
	static void setCurrentWorldFrictionCoefficient(fixed_t currentWorldFriction);
	static void setCurrentGravity(const Vector3D* currentGravity);
	static const Vector3D* getCurrentGravity();
	static fixed_t computeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed);

	void constructor(SpatialObject owner, const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity);
	void applySustainedForce(const Vector3D* force);
	uint8 applyForce(const Vector3D* force);
	uint8 applyGravity(uint16 axis);
	void bounce(ListenerObject bounceReferent, Vector3D bouncingPlaneNormal, fixed_t frictionCoefficient, fixed_t bounciness);
	void clearAcceleration(uint16 axis);
	void clearExternalForce();
	Vector3DFlag getAccelerationState();
	Vector3D getAppliedForce();
	uint16 getAxisSubjectToGravity();
	fixed_t getBounciness();
	Vector3D getLastDisplacement();
	Vector3D getGravity();
	Vector3D getFriction();
	fixed_t getMass();
	MovementType getMovementType();
	SpatialObject getOwner();
	const Vector3D* getPosition();
	const Vector3D* getVelocity();
	void setVelocity(Vector3D* velocity);
	const Vector3D* getDirection();
	void setDirection3D(const Vector3D* direction);
	fixed_t getSpeed();
	fixed_ext_t getSpeedSquare();
	void modifyVelocity(const Vector3D* multiplier);
	bool isActive();
	bool isAwake();
	bool reachedMaximumSpeed();
	uint16 getMovementOnAllAxis();
	void setMovementType(int32 movementType, uint16 axis);
	void moveAccelerated(uint16 axis);
	void moveUniformly(Vector3D velocity);
	void setActive(bool active);
	void setAxisSubjectToGravity(uint16 axisSubjectToGravity);
	void setBounciness(fixed_t bounciness);
	void setSkipCycles(int8 skipCycles);
	Vector3D getNormal();
	Vector3D getLastNormalDirection();
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
	void setMaximumVelocity(Vector3D maximumVelocity);
	Vector3D getMaximumVelocity();
	void setMaximumSpeed(fixed_t maximumSpeed);
	fixed_t getMaximumSpeed();
	void print(int32 x, int32 y);
	void sendMessages(bool value);
	void update(uint16 cycle);
}


#endif
