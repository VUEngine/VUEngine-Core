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


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Collider.h>
#include <PhysicalParticle.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A SolidParticle Spec
/// @memberof SolidParticle
typedef struct SolidParticleSpec
{
	PhysicalParticleSpec physicalParticleSpec;

	/// Particles's in-game type
	uint32 inGameType;

	/// Animation to play upon collision
	const char* onCollisionAnimation;

} SolidParticleSpec;

/// A SolidParticle spec that is stored in ROM
/// @memberof SolidParticle
typedef const SolidParticleSpec SolidParticleROMSpec;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class SolidParticle
///
/// Inherits from PhysicalParticle
///
/// Implements a physical particle that collides with other objects in the game stage.
class SolidParticle : PhysicalParticle
{
	/// @protectedsection

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

	/// Check if the particle is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the particle; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	override bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Process when a previously detected collision by one of the component colliders stops.
	/// @param collisionInformation: Information struct about the collision to resolve
	override void collisionEnds(const CollisionInformation* collisionInformation);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();
}


#endif
