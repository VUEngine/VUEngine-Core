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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class SpatialObject;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define	__NO_MOVEMENT					0x00
#define	__UNIFORM_MOVEMENT				0x01
#define	__ACCELERATED_MOVEMENT			0x20

#define __BODY_MIN_MASS					__F_TO_FIXED(0.01f)
#define __BODY_MAX_MASS					__I_TO_FIXED(1)


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Struct that specifies the physical properties of bodies
/// @memberof Body
typedef struct PhysicalProperties
{
	/// Mass
	fixed_t mass;
	
	/// Friction coefficient
	fixed_t frictionCoefficient;

	/// Bounciness
	fixed_t bounciness;
	
	/// Maximum velocity
	Vector3D maximumVelocity;
	
	/// Maximum speed
	fixed_t maximumSpeed;

} PhysicalProperties;

typedef const PhysicalProperties PhysicalPropertiesROMSpec;

/// 3D Vector struct for higher decimal precision
/// @memberof Body
typedef struct Vector3DPlus
{
	fix7_9_ext x;
	fix7_9_ext y;
	fix7_9_ext z;

} Vector3DPlus;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Body
///
/// Inherits from ListenerObject
///
/// Implements newtonian physics.
/// @ingroup physics
class Body : ListenerObject
{
	/// @protectedsection

	/// Object to which this body attaches to
	SpatialObject owner;

	/// Spatial position
	Vector3D position;

	/// Spatial position at higher decimal precision
	Vector3DPlus internalPosition;

	/// Velocity vector
	Vector3D velocity;

	/// Velocity vector at higher decimal precision
	Vector3DPlus internalVelocity;

	// Direction vector
	Vector3D direction;

	/// Maximum velocity
	Vector3D maximumVelocity;

	/// Speed magnitude
	fixed_t speed;

	/// Maximum speed
	fixed_t maximumSpeed;

	/// Gravity affecting the body
	Vector3D gravity;

	/// External applied force
	Vector3D externalForce;

	/// Friction surrounding object
	Vector3D friction;
	
	/// Total normal forces applied to the body
	Vector3D totalNormal;
	
	/// List of normal for the forces affecting the body
	VirtualList normals;

	/// Flags for the acceleration state of the body on each axis
	Vector3DFlag accelerating;
	
	/// Body's bounciness factor
	fixed_t bounciness;

	/// Friction coefficient of the body
	fixed_t frictionCoefficient;

	/// Friction coefficient of the body's surroundings
	fixed_t surroundingFrictionCoefficient;

	/// Friction force's magnitude
	fixed_t totalFrictionCoefficient;

	/// Total friction force magnitude affecting the body
	fixed_t frictionForceMagnitude;

	/// Body's mass
	fixed_t mass;

	/// Body's movement type on each axis
	MovementType movementType;

	/// Axis on which the body is subject to gravity
	uint16 axisSubjectToGravity;

	/// Number of cycles to skip physical simulations to slow down physics
	int8 skipCycles;

	/// Number of skiped cycles
	int8 skipedCycles;
	
	/// If not true, the body skips physical simulations
	bool awake;

	/// If true, the body informs its owner of its change in movement state
	bool sendMessages;

	/// Flag to destroy the body
	bool deleteMe;

	/// If true, the movement of the body is independent on each axis
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
	void setVelocity(const Vector3D* velocity);
	const Vector3D* getDirection();
	void setDirection3D(const Vector3D* direction);
	fixed_t getSpeed();
	fixed_ext_t getSpeedSquare();
	void modifyVelocity(const Vector3D* multiplier);
	bool isAwake();
	bool reachedMaximumSpeed();
	uint16 getMovementOnAllAxis();
	void setMovementType(int32 movementType, uint16 axis);
	void moveAccelerated(uint16 axis);
	void moveUniformly(const Vector3D* velocity);
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
