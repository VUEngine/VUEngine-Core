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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct ParticleSystemDefinition
{
	/// it has an Entity at the beginning
	EntityDefinition entityDefinition;

	/// whether to delete or reuse expired particles
	u8 recycleParticles;

	/// minimum generation delay in milliseconds
	u16 minimumSpawnDelay;

	/// generation delay delta in milliseconds
	u16 spawnDelayDelta;

	/// maximum total particles
	u8 maximumNumberOfAliveParticles;

	/// array of sprites to select randomly
	const ObjectSpriteDefinition** objectSpriteDefinitions;

	/// auto start
	bool autoStart;

	/// particle's definition
	ParticleDefinition* particleDefinition;

	/// minimum relative spawn position
	Vector3D minimumRelativeSpawnPosition;

	/// maximum relative spawn position
	Vector3D maximumRelativeSpawnPosition;

	/// minimum force to apply (use int values in the definition to avoid overflow)
	Vector3D minimumForce;

	/// maximum force to apply (use int values in the definition to avoid overflow)
	Vector3D maximumForce;

	/// type of movement for the particles
	u32 movementType;

} ParticleSystemDefinition;

/**
 * A ParticleSystem that is stored in ROM
 *
 * @memberof	ParticleSystem
 */
typedef const ParticleSystemDefinition ParticleSystemROMDef;


class ParticleSystem : Entity
{
	/*
	* @var ParticleSystemDefinition*	particleSystemDefinition
	* @brief							system's definition
	* @memberof						ParticleSystem
	*/
	const ParticleSystemDefinition* particleSystemDefinition;
	/*
	* @var VirtualList				 	particles
	* @brief							particle list
	* @memberof						ParticleSystem
	*/
	VirtualList particles;
	/*
	* @var VirtualList				 	recyclableParticles
	* @brief							particle list
	* @memberof						ParticleSystem
	*/
	VirtualList recyclableParticles;
	/*
	* @var VirtualList				 	expiredParticles
	* @brief							particle list
	* @memberof						ParticleSystem
	*/
	VirtualList expiredParticles;
	/*
	* @var int						 	nextSpawnTime
	* @brief							next spawn time
	* @memberof						ParticleSystem
	*/
	int nextSpawnTime;
	/*
	* @var int						 	particleCount
	* @brief							particle count
	* @memberof						ParticleSystem
	*/
	int particleCount;
	/*
	* @var s16						 	numberOfSpriteDefinitions
	* @brief							number of sprite definitions
	* @memberof						ParticleSystem
	*/
	s16 numberOfSpriteDefinitions;
	/*
	* @var bool						paused
	* @brief							pause flag
	* @memberof						ParticleSystem
	*/
	bool paused;

	void constructor(ParticleSystem this, ParticleSystemDefinition* particleSystemDefinition,  s16 id, s16 internalId, const char* const name);
	bool handleMessage(ParticleSystem this, Telegram telegram);
	void pause(ParticleSystem this);
	void show(ParticleSystem this);
	void spawnAllParticles(ParticleSystem this);
	void start(ParticleSystem this);
	override void update(ParticleSystem this, u32 elapsedTime);
	override void transform(ParticleSystem this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void synchronizeGraphics(ParticleSystem this);
	override void resume(ParticleSystem this);
	override void suspend(ParticleSystem this);
	override void hide(ParticleSystem this);
}


#endif
