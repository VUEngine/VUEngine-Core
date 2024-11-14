/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Collider.h>
#include <PhysicalParticle.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A SolidParticle Spec
/// @memberof SolidParticle
typedef struct SolidParticleSpec
{
	PhysicalParticleSpec physicalParticleSpec;

	/// Collider's radius
	fixed_t radius;

	/// Particle's friction
	fixed_t frictionCoefficient;

	/// Particle's bounciness
	fixed_t bounciness;

	/// Particles's in-game type
	uint32 inGameType;

	/// Layers in which the collider lives
	uint32 layers;

	/// Layers to ignore when checking for collisions
	uint32 layersToIgnore;

	/// If true, collisions detectionis disabled whenn the particle stops
	bool disableCollisionOnStop;

	/// Animation to play upon collision
	const char* onCollisionAnimation;

} SolidParticleSpec;

/// A SolidParticle spec that is stored in ROM
/// @memberof SolidParticle
typedef const SolidParticleSpec SolidParticleROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class SolidParticle
///
/// Inherits from PhysicalParticle
///
/// Implements a physical particle that collides with other objects in the game stage.
/// @ingroup stage-entities-particles
class SolidParticle : PhysicalParticle
{
	/// @protectedsection

	/// Collider for collision detection
	Collider collider;

	/// Specification for the collider
	ColliderSpec* colliderSpec;

	/// Specification that determines how to configure the particle
	const SolidParticleSpec* solidParticleSpec;

	/// @publicsection

	/// Class' constructor
	/// @param solidParticleSpec: Specification that determines how to configure the particle
	void constructor(const SolidParticleSpec* solidParticleSpec);

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Retrieve the particle's radius.
	/// @return Radius
	override fixed_t getRadius();

	/// Check if the particle is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the particle; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	override bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Process when a previously detected collision by one of the component colliders stops.
	/// @param collisionInformation: Information struct about the collision to resolve
	override void collisionEnds(const CollisionInformation* collisionInformation);

	/// Reset the particle's state.
	override void reset();
}


#endif
