/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

	/// whether to delete or reuse expired particles
	u8 recycleParticles;

	/// minimum generation delay in milliseconds
	u16 minimumSpawnDelay;

	/// generation delay delta in milliseconds
	u16 spawnDelayDelta;

	/// maximum total particles
	u8 maximumNumberOfAliveParticles;

	/// array of sprites to select randomly
	const SpriteSpec** spriteSpecs;

	/// auto start
	bool autoStart;

	/// particle's spec
	ParticleSpec* particleSpec;

	/// minimum relative spawn position
	Vector3D minimumRelativeSpawnPosition;

	/// maximum relative spawn position
	Vector3D maximumRelativeSpawnPosition;

	/// minimum force to apply (use int values in the spec to avoid overflow)
	Vector3D minimumForce;

	/// maximum force to apply (use int values in the spec to avoid overflow)
	Vector3D maximumForce;

	/// type of movement for the particles
	u32 movementType;

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
	// system's spec
	const ParticleSystemSpec* particleSystemSpec;
	// particle list
	VirtualList particles;
	// next spawn time
	int nextSpawnTime;
	// particle count
	int particleCount;
	// particles' life span increment
	s16 particleLifeSpanIncrement;
	// number of sprite specs
	s16 numberOfSpriteSpecs;
	// pause flag
	bool paused;
	// Flag to keep spawning particles
	bool loop;
	// Counter of total spawned particles
	u16 totalSpawnedParticles;
	// Flags to speed up particle spawning
	Vector3DFlag spawnPositionDisplacement;
	Vector3DFlag spawnForceDelta;
	// Particles' animation name
	const char* animationName;
	u8 maximumNumberOfAliveParticles;

	/// @publicsection
	void constructor(ParticleSystemSpec* particleSystemSpec,  s16 id, const char* const name);
	void setParticleSystemSpec(ParticleSystemSpec* particleSystemSpec, bool reset);
	bool handleMessage(Telegram telegram);
	bool isPaused();
	void pause();
	void unpause();
	void show();
	void spawnAllParticles();
	void start();
	bool getLoop();
	void setLoop(bool value);
	void setMaximumNumberOfAliveParticles(u8 maximumNumberOfAliveParticles);
	override void update(u32 elapsedTime);
	override void transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void synchronizeGraphics();
	override void resume();
	override void suspend();
	override void hide();
}


#endif
