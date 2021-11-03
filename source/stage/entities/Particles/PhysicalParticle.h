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

typedef struct PhysicalParticleSpec
{
	ParticleSpec particleSpec;

	/// particle's minimum mass
	fix10_6 minimumMass;

	/// particle's mass delta
	fix10_6 massDelta;

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
	void constructor(const PhysicalParticleSpec* physicalParticleSpec, const SpriteSpec* spriteSpec, int16 lifeSpan);
	override bool isSubjectToGravity(Acceleration gravity);
	override void setPosition(const Vector3D* position);
	override void addForce(const Force* force, uint32 movementType);
	override bool update(uint32 elapsedTime, void (* behavior)(Particle particle));
	override void transform();
	override void setMass(fix10_6 mass);
	override void hide();
	override void reset();
	override void changeMass();
}


#endif
