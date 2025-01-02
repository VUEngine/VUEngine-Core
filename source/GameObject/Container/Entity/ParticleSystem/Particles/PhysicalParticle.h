/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PHYSICAL_PARTICLE_H_
#define PHYSICAL_PARTICLE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <Particle.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

class Body;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A PhysicalParticle Spec
/// @memberof PhysicalParticle
typedef struct PhysicalParticleSpec
{
	ParticleSpec particleSpec;

} PhysicalParticleSpec;

/// A PhysicalParticle spec that is stored in ROM
/// @memberof PhysicalParticle
typedef const PhysicalParticleSpec PhysicalParticleROMSpec;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class PhysicalParticle
///
/// Inherits from Particle
///
/// Implements a particle that physically moves through a game stage.
class PhysicalParticle : Particle
{
	/// @publicsection

	/// Class' constructor
	/// @param physicalParticleSpec: Specification that determines how to configure the particle
	void constructor(const PhysicalParticleSpec* physicalParticleSpec);
	
	/// Set the particle's position.
	/// @param position: 3D vector defining the particle's new position
	override void setPosition(const Vector3D* position);

	/// Check if the particle is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the particle; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Update the particle's state.
	/// @param elapsedTime: Elapsed time since the last call
	/// @param behavior: Function pointer to control particle's behavior
	override bool update(uint32 elapsedTime, void (* behavior)(Particle particle));

	/// Apply a force to the particle.
	/// @param force: Force to be applied
	/// @param movementType: Movement type on each axis
	override void applyForce(const Vector3D* force, uint32 movementType);
}


#endif
