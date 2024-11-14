/**
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
/// @ingroup stage-entities-particles
class ParticleSystem : Entity
{
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
	
	/// Flag to trigger animations in the particles
	bool animationChanged;

	/// If true, the particle system auto destroys when the total number 
	/// of spawned particles equals the maximum number of alive particles
	bool selfDestroyWhenDone;
	
	/// Flag to prevent computing force when not necessary
	bool applyForceToParticles;
	
	// Raise flag when transformed to reset particles' positions
	bool transformed;

	/// @publicsection
	void constructor(const ParticleSystemSpec* particleSystemSpec,  int16 internalId, const char* const name);
	void setParticleSystemSpec(ParticleSystemSpec* particleSystemSpec, bool reset);
	bool handleMessage(Telegram telegram);
	bool isPaused();
	void pause();
	void unpause();
	void spawnAllParticles();
	void start();
	bool getLoop();
	void setLoop(bool value);
	void deleteAllParticles();
	void expireAllParticles();
	void setMaximumNumberOfAliveParticles(uint8 maximumNumberOfAliveParticles);
	void setSelfDestroyWhenDone(bool selfDestroyWhenDone);
	void setElapsedTime(uint32 elapsedTime);
	const AnimationFunction** getAnimationFunctions();
	void print(int16 x, int16 y);

	override void update();
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	override void resume();
	override void suspend();
	override void show();
	override void hide();
	override void setTransparent(uint8 transparent);
	virtual void particleSpawned(Particle particle);
	virtual void particleRecycled(Particle particle);
}


#endif
