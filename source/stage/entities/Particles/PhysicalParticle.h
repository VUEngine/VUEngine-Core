/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PHYSICAL_PARTICLE_H_
#define PHYSICAL_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Body;

typedef struct PhysicalParticleSpec
{
	ParticleSpec particleSpec;

	/// particle's minimum mass
	fixed_t minimumMass;

	/// particle's mass delta
	fixed_t massDelta;

	/// axis subject to gravity (bitwise or of __X_AXIS, __Y_AXIS, __Z_AXIS, or false to disable)
	uint16 axisSubjectToGravity;

} PhysicalParticleSpec;

/**
 * A Particle that is stored in ROM
 *
 * @memberof	Particle
 */
typedef const PhysicalParticleSpec PhysicalParticleROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class PhysicalParticle : Particle
{
	// Particle's spec
	const PhysicalParticleSpec* physicalParticleSpec;
	// Particle's physical body
	Body body;

	/// @publicsection
	void constructor(const PhysicalParticleSpec* physicalParticleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, ParticleSystem creator);
	override bool isSubjectToGravity(Vector3D gravity);
	override void setPosition(const Vector3D* position);
	override void applySustainedForce(const Vector3D* force, uint32 movementType);
	override bool update(uint32 elapsedTime, void (* behavior)(Particle particle));
	override void reset();
	override void changeMass();
}


#endif
