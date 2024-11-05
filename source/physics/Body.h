/*
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

	/// Axises on which the body is subject to gravity
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

	/// Compute the instantaneous speed caused by the provided physical properties.
	/// @param forceMagnitude: Magnitude of the applied force
	/// @param gravity: Gravity acceleration vector that affects the resulting speed
	/// @param mass: The mass that will aquire the computed speed
	/// @param friction: Friction affecting the mass that will aquire the computed speed
	/// @param maximumSpeed: Maximum value that the computated speed can reach
	/// @return The instantaneous speed caused by the provided physical properties
	static fixed_t computeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed);

	/// Class' constructor
	/// @param owner: SpatialObject to which the body attaches to
	/// @param physicalProperties: Struct that specifies the physical properties of bodies
	/// @param axisSubjectToGravity: Axis on which the body is subject to gravity
	void constructor(SpatialObject owner, const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity);
	
	/// Clear the body's state.
	void reset();

	/// Remove any normal vector affecting the body belonging to the provided referent.
	/// @param referent: Normal vector affecting the body's owner
	void clearNormal(ListenerObject referent);

	/// Update the physics simulation on the body.
	/// @param cycle: Cycle number during the current second
	/// @param elapsedTime: Elapsed time since the last call to this method
	void update(uint16 cycle, fix7_9_ext elapsedTime);

	/// Apply a force to the body.
	/// @param force: Force to be applied
	uint8 applyForce(const Vector3D* force);

	/// Make the body to bounce on the profixed plane according to the provided friction and bounciness.
	/// @param bounceReferent: Referent of the normal to the plane on which the body has to bounce
	/// @param bouncingPlaneNormal: Normal of the plane on which the body has to bounce
	/// @param frictionCoefficient: Friction coefficient of the bounce referent
	/// @param bounciness: Bounciness coefficient of the bounce referent
	void bounce(ListenerObject bounceReferent, Vector3D bouncingPlaneNormal, fixed_t frictionCoefficient, fixed_t bounciness);

	/// Stop the body's movement on the speficied axis.
	/// @param axis: Flag indicating the axises on which the movement has to stop
	/// @return Flag indicatiing the actual axises on which the body's movement stopped
	uint16 stopMovement(uint16 axis);

	/// Set a constant velocity at which the body must move.
	/// @param velocity: Pointer to a velocity vector
	void setVelocity(const Vector3D* velocity);

	/// Retrieve the current velocity at which the body move.
	/// @return Pointer to the body's velocity vector
	const Vector3D* getVelocity();
	
	/// Set the direction towards which the body must move.
	/// @param velocity: Pointer to a direction vector
	void setDirection(const Vector3D* direction);

	/// Retrieve the direction towards which the body is moving.
	/// @param velocity: Pointer to the direction towards which the body is moving
	const Vector3D* getDirection();
	
	/// Set the axises on which the body is subject to gravity.
	/// @param axisSubjectToGravity: Flag containing the axises on which the body is subject to gravity
	void setAxisSubjectToGravity(uint16 axisSubjectToGravity);

	/// Rretrieve the axises on which the body is subject to gravity.
	/// @return Flag containing the axises on which the body is subject to gravity
	uint16 getAxisSubjectToGravity();

	/// Set the body's bounciness factor.
	/// @param bounciness: Value to set as the body's bounciness factor (between 0 and 1)
	void setBounciness(fixed_t bounciness);

	/// Retrieve the body's bounciness factor.
	/// @return Body's bounciness factor
	fixed_t getBounciness();

	/// Set the body's friction coefficient.
	/// @param frictionCoefficient: Value to set as the body's friction coefficient (between 0 and __MAXIMUM_FRICTION_COEFFICIENT)
	void setFrictionCoefficient(fixed_t frictionCoefficient);

	/// Retrieve the body's friction coefficient.
	/// @return Body's friction coefficient
	fixed_t getFrictionCoefficient();

	/// Set the body's mass.
	/// @param frictionCoefficient: Value to set as the body's mass (between 0.01f and 1)
	void setMass(fixed_t mass);

	/// Retrieve the body's mass.
	/// @return Body's mass
	fixed_t getMass();

	/// Set the body's position.
	/// @param position: 3D vector defining the body's new position
	/// @param caller: Must be the body's owner; otherwise the call to this method doesn't have any effect
	void setPosition(const Vector3D* position, SpatialObject caller);

	/// Retrieve the body's position.
	/// @return Pointer to the body's 3D vector defining its position
	const Vector3D* getPosition();

	/// Set the body's maximum velocity.
	/// @param maximumVelocity: 3D vector defining the body's maximum speed on each axis
	/// (only applicable when the body's movement is independent on each axis)
	void setMaximumVelocity(Vector3D maximumVelocity);

	/// Retrieve the body's maximum velocity.
	/// @return 3D vector defining the body's maximum speed on each axis
	/// (only applicable when the body's movement is independent on each axis)
	Vector3D getMaximumVelocity();

	/// Set the body's maximum speed.
	/// @param maximumSpeed: Maximum magnitude of the body's velocity
	void setMaximumSpeed(fixed_t maximumSpeed);

	/// Retrieve the body's maximum speed.
	/// @return Maximum magnitude of the body's velocity
	fixed_t getMaximumSpeed();
	
	/// Set the flag that enables or prevents the sending of messages to the body's owner about its state changes.
	/// @param value: If true, the body sends messages to its owner when its movement state changes
	void sendMessages(bool value);

	/// Set the number of cycles to wait before updating the physics simulations on the body.
	/// @param skipCycles: Number of cycles to skip physical simulations to slow down physics
	void setSkipCycles(int8 skipCycles);
	
	/// Set the body's friction coefficient of the its surroundings.
	/// @param surroundingFrictionCoefficient: Body's friction coefficient of the its surroundings
	void setSurroundingFrictionCoefficient(fixed_t surroundingFrictionCoefficient);

	/// Retrieve the body's current speed (velocity vector's magnitude).
	/// @return Body's current speed (velocity vector's magnitude)
	fixed_t getSpeed();

	/// Retrieve the body's current movement state
	/// @return Flag containing the body's current movement state on each axis
	uint16 getMovementOnAllAxis();
	
	/// Print the body's properties.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}


#endif
