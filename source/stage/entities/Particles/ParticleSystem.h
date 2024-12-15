/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Entity.h>
#include <Particle.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A ParticleSystem Spec
/// @memberof ParticleSystem
typedef struct ParticleSystemSpec
{
	EntitySpec entitySpec;

	/// Reuse expired particles?
	uint8 recycleParticles;

	/// Minimum generation delay in milliseconds
	uint16 minimumSpawnDelay;

	/// gGneration delay delta in milliseconds
	uint16 spawnDelayDelta;

	/// Maximum number of alive particles
	uint8 maximumNumberOfAliveParticles;

	/// Maximum number of particles to spawn in each cycle
	uint8 maximumNumberOfParticlesToSpawnPerCycle;

	/// Array of sprites to select randomly
	const SpriteSpec** spriteSpecs;

	/// Array of wireframes to select randomly
	const WireframeSpec** wireframeSpecs;

	/// Auto start
	bool autoStart;

	/// Particle's spec
	ParticleSpec* particleSpec;

	/// Minimum relative spawn position
	Vector3D minimumRelativeSpawnPosition;

	/// Maximum relative spawn position
	Vector3D maximumRelativeSpawnPosition;

	/// Minimum force to apply (use int32 values in the spec to avoid overflow)
	Vector3D minimumForce;

	/// Maximum force to apply (use int32 values in the spec to avoid overflow)
	Vector3D maximumForce;

	/// Type of movement for the particles
	uint32 movementType;

} ParticleSystemSpec;

/// A ParticleSystem spec that is stored in ROM
/// @memberof ParticleSystem
typedef const ParticleSystemSpec ParticleSystemROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class ParticleSystem
///
/// Inherits from Entity
///
/// Implements an entity that generates particles.
class ParticleSystem : Entity
{
	/// @protectedsection

	/// Linked list of particles
	VirtualList particles;

	/// Range for random displacement upon particle spawning
	Vector3D spawnPositionDisplacement;

	/// Range for random force delta to be adde to the force applied 
	/// to newly spawned particles
	Vector3D spawnForceDelta;
	
	/// Elapsed time per tick
	uint32 elapsedTime;

	/// Time when the next particle has to be spawned
	int32 nextSpawnTime;

	/// Number of available sprite specs for particles
	int8 numberOfSpriteSpecs;

	/// Number of available wireframes specs for particles
	int8 numberOfWireframeSpecs;

	/// Number of alive particles
	int8 aliveParticlesCount;

	/// Counter of total spawned particles
	int8 totalSpawnedParticles;

	/// Maximum number of alive particles at any given moment
	uint8 maximumNumberOfAliveParticles;

	/// Flag to pause the generation of particles
	bool paused;

	/// If false, the spawning or particles stops once the total number 
	/// of spawned particles equals the maximum number of alive particles
	bool loop;
	
	/// If true, the particle system auto destroys when the total number 
	/// of spawned particles equals the maximum number of alive particles
	bool selfDestroyWhenDone;
	
	/// Flag to prevent computing force when not necessary
	bool applyForceToParticles;

	/// @publicsection

	/// Class' constructor
	/// @param particleSystemSpec: Specification that determines how to configure the particle system
	/// @param internalId: ID to internally identify this instance
	/// @param name: Instance's name
	void constructor(const ParticleSystemSpec* particleSystemSpec,  int16 internalId, const char* const name);

	/// Make this instance visible.
	override void show();

	/// Make this instance invisible.
	override void hide();

	/// Update this instance's logic.
	override void update();

	/// Prepare to suspend this instance's logic.
	override void suspend();

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Set this instance's transparency effects.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	override void setTransparency(uint8 transparency);

	/// Set the particle systems's spec.
	/// @param particleSystemSpec: Specification that determines how to configure the particle system
	override void setSpec(void* particleSystemSpec);

	/// Start spawning particles.
	void start();

	/// Pause the spawning of particles.
	void pause();

	/// Resume the spawning of particles. 
	void unpause();

	/// Check if the spawning of particles is paused.
	/// @return True if the spawning of particles is paused; false otherwise
	bool isPaused();

	/// Delete all spawned particles.
	void deleteAllParticles();

	/// Set the looping behavior of the particle system.
	/// @param loop: If false, the spawning or particles stops once the total number 
	/// of spawned particles equals the maximum number of alive particles
	void setLoop(bool loop);

	/// Check the looping behavior of the particle system.
	/// @return False if the spawning or particles stops once the total number 
	/// of spawned particles equals the maximum number of alive particles
	bool getLoop();

	/// Set the particle system to auto destroy or not when the total number 
	/// of spawned particles equals the maximum number of alive particles.
	/// @param selfDestroyWhenDone: If true, the particle system auto destroys when the
	/// total number of spawned particles equals the maximum number of alive particles.
	void setSelfDestroyWhenDone(bool selfDestroyWhenDone);

	/// Set the elapsed time between calls to the update method.
	/// @param elapsedTime: Elapsed time between calls to the update method
	void setElapsedTime(uint32 elapsedTime);

	/// Print the particle system's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int16 x, int16 y);

	/// Informs itself when a particle is spawned.
	/// @param particle: The newly spawned particle 
	virtual void particleSpawned(Particle particle);

	/// Informs itself when a particle is recycled.
	/// @param particle: The recycled particle 
	virtual void particleRecycled(Particle particle);
}


#endif
