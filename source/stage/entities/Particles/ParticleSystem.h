/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#define ParticleSystem_METHODS(ClassName)																\
		Entity_METHODS(ClassName)																		\

#define ParticleSystem_SET_VTABLE(ClassName)															\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ParticleSystem, update);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, transform);											\
		__VIRTUAL_SET(ClassName, ParticleSystem, updateVisualRepresentation);							\
		__VIRTUAL_SET(ClassName, ParticleSystem, resume);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, suspend);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, hide);													\

#define ParticleSystem_ATTRIBUTES																		\
		Entity_ATTRIBUTES																				\
		/*
		 * @var ParticleSystemDefinition*	particleSystemDefinition
		 * @brief							system's definition
		 * @memberof						ParticleSystem
		 */																								\
		const ParticleSystemDefinition* particleSystemDefinition;										\
		/*
		 * @var VirtualList				 	particles
		 * @brief							particle list
		 * @memberof						ParticleSystem
		 */																								\
		VirtualList particles;																			\
		/*
		 * @var VirtualList				 	recyclableParticles
		 * @brief							particle list
		 * @memberof						ParticleSystem
		 */																								\
		VirtualList recyclableParticles;																\
		/*
		 * @var VirtualList				 	expiredParticles
		 * @brief							particle list
		 * @memberof						ParticleSystem
		 */																								\
		VirtualList expiredParticles;																	\
		/*
		 * @var int						 	nextSpawnTime
		 * @brief							next spawn time
		 * @memberof						ParticleSystem
		 */																								\
		int nextSpawnTime;																				\
		/*
		 * @var int						 	particleCount
		 * @brief							particle count
		 * @memberof						ParticleSystem
		 */																								\
		int particleCount;																				\
		/*
		 * @var s16						 	numberOfSpriteDefinitions
		 * @brief							number of sprite definitions
		 * @memberof						ParticleSystem
		 */																								\
		s16 numberOfSpriteDefinitions;																	\
		/*
		 * @var bool						paused
		 * @brief							pause flag
		 * @memberof						ParticleSystem
		 */																								\
		bool paused;																					\

__CLASS(ParticleSystem);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Defines a ParticleSystem
 *
 * @memberof	ParticleSystem
 */
typedef struct ParticleSystemDefinition
{
	/// it has an Entity at the beginning
	EntityDefinition entityDefinition;

	/// whether to delete or reuse expired particles
	u8 recycleParticles;

	/// minimun generation delay in milliseconds
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

	/// minimum random distance from the center of the system for spanw
	VBVec3D minimumSpanDistance;

	/// minimum relative spawn position
	VBVec3D minimumRelativeSpanPosition;

	/// maximum relative spawn position
	VBVec3D maximumRelativeSpanPosition;

	/// minimum force to apply (use int values in the definition to avoid overflow)
	VBVec3D minimumForce;

	/// maximum force to apply (use int values in the definition to avoid overflow)
	VBVec3D maximumForce;

	/// type of movement for the particles
	u32 movementType;

} ParticleSystemDefinition;

/**
 * A ParticleSystem that is stored in ROM
 *
 * @memberof	ParticleSystem
 */
typedef const ParticleSystemDefinition ParticleSystemROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ParticleSystem, ParticleSystemDefinition* particleSystemDefinition, s16 id, s16 internalId, const char* const name);

void ParticleSystem_constructor(ParticleSystem this, ParticleSystemDefinition* particleSystemDefinition,  s16 id, s16 internalId, const char* const name);
void ParticleSystem_destructor(ParticleSystem this);

bool ParticleSystem_handleMessage(ParticleSystem this, Telegram telegram);
void ParticleSystem_hide(ParticleSystem this);
void ParticleSystem_pause(ParticleSystem this);
void ParticleSystem_resume(ParticleSystem this);
void ParticleSystem_show(ParticleSystem this);
void ParticleSystem_spawnAllParticles(ParticleSystem this);
void ParticleSystem_start(ParticleSystem this);
void ParticleSystem_suspend(ParticleSystem this);
void ParticleSystem_transform(ParticleSystem this, const Transformation* environmentTransform);
void ParticleSystem_update(ParticleSystem this, u32 elapsedTime);
void ParticleSystem_updateVisualRepresentation(ParticleSystem this);


#endif
