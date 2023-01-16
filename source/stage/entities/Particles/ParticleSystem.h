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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Clock.h>
#include <Particle.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef struct ParticleSystemSpec
{
	/// it has an Entity at the beginning
	EntitySpec entitySpec;

	/// reuse expired particles?
	uint8 recycleParticles;

	/// minimum generation delay in milliseconds
	uint16 minimumSpawnDelay;

	/// generation delay delta in milliseconds
	uint16 spawnDelayDelta;

	/// maximum number of alive particles
	uint8 maximumNumberOfAliveParticles;

	/// maximum number of particles to spawn in each cycle
	uint8 maximumNumberOfParticlesToSpawnPerCycle;

	/// array of sprites to select randomly
	const SpriteSpec** spriteSpecs;

	/// array of wireframes to select randomly
	const WireframeSpec** wireframeSpecs;

	/// auto start
	bool autoStart;

	/// particle's spec
	ParticleSpec* particleSpec;

	/// minimum relative spawn position
	Vector3D minimumRelativeSpawnPosition;

	/// maximum relative spawn position
	Vector3D maximumRelativeSpawnPosition;

	/// minimum force to apply (use int32 values in the spec to avoid overflow)
	Vector3D minimumForce;

	/// maximum force to apply (use int32 values in the spec to avoid overflow)
	Vector3D maximumForce;

	/// type of movement for the particles
	uint32 movementType;

	/// use particle system movement vector for the force to apply to the particles
	bool useMovementVector;

} ParticleSystemSpec;

/**
 * A ParticleSystem that is stored in ROM
 *
 * @memberof	ParticleSystem
 */
typedef const ParticleSystemSpec ParticleSystemROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class ParticleSystem : Entity
{
	// Vector for particles' movement
	Vector3D previousGlobalPosition;
	// particle list
	VirtualList particles;
	// Flags to speed up particle spawning
	Vector3DFlag spawnPositionDisplacement;
	Vector3DFlag spawnForceDelta;
	// elapsed time per tick
	uint32 elapsedTime;
	// next spawn time
	int32 nextSpawnTime;
	// particles' life span increment
	int16 particleLifeSpanIncrement;
	// number of specs
	int8 numberOfSpriteSpecs;
	int8 numberOfWireframeSpecs;
	// particle count
	int8 particleCount;
	// pause flag
	bool paused;
	// Flag to keep spawning particles
	bool loop;
	// Counter of total spawned particles
	int8 totalSpawnedParticles;
	// Particles' animation name
	uint8 maximumNumberOfAliveParticles;
	// Flag to trigger animations in the particles
	bool animationChanged;
	// Flag to auto destroy when done
	bool selfDestroyWhenDone;
	// Flag to prevent computing force when not necessary
	bool applyForceToParticles;

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
	override void update();
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	override void synchronizeGraphics();
	override void resume();
	override void suspend();
	override void show();
	override void hide();
	virtual void transformParticles();
	virtual void particleSpawned(Particle particle);
	virtual void particleRecycled(Particle particle);
}


#endif
