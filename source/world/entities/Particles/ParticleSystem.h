/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Clock.h>
#include <Particle.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ParticleSystem_METHODS													\
		Entity_METHODS															\

#define ParticleSystem_SET_VTABLE(ClassName)									\
		Entity_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, ParticleSystem, update);						\
		__VIRTUAL_SET(ClassName, ParticleSystem, transform);					\
		__VIRTUAL_SET(ClassName, ParticleSystem, resume);						\
		__VIRTUAL_SET(ClassName, ParticleSystem, suspend);						\

#define ParticleSystem_ATTRIBUTES												\
																				\
	/* it is derivated from */													\
	Entity_ATTRIBUTES															\
																				\
	/* system's definition */													\
	const ParticleSystemDefinition* particleSystemDefinition;					\
																				\
	/* particle list */															\
	VirtualList particles;														\
																				\
	/* particle list */															\
	VirtualList expiredParticles;												\
																				\
	/* clock  */																\
	Clock clock;																\
																				\
	/* last udpate time */														\
	u32 lastUpdateTime;															\
																				\
	/* next spawn time */														\
	u32 nextSpawnTime;															\
																				\
	/* status flag */															\
	bool paused;																\
																				\
	/* status flag */															\
	u8 numberOfSpriteDefinitions;												\

__CLASS(ParticleSystem);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a ParticleSystem in ROM memory
typedef struct ParticleSystemDefinition
{
	// It has an Entity at the beggining
	EntityDefinition entityDefinition;

	// minimun generation delay in miliseconds
	u16 minimumSpawnDelay;

	// maximum generation delay in miliseconds
	u16 maximumSpawnDelay;

	// maximum total particles
	u8 maximumNumberOfAliveParticles;
	
	// array of sprites to select randomly
	const ObjectSpriteDefinition** objectSpriteDefinitions;

	// auto start
	bool autoStart;

	// particle's definition
	ParticleDefinition* particleDefinition;
	
	// minimum random distance from the center of the system for spanw
	VBVec3D minimumSpanDistance;

	// minimum relative spawn position
	VBVec3D minimumRelativeSpanPosition;

	// maximum relative spawn position
	VBVec3D maximumRelativeSpanPosition;
	
	// minimum force to apply (use int values in the definition to avoid overflow)
	VBVec3D minimumForce;

	// maximum force to apply (use int values in the definition to avoid overflow)
	VBVec3D maximumForce;

} ParticleSystemDefinition;

typedef const ParticleSystemDefinition ParticleSystemROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ParticleSystem, const ParticleSystemDefinition* particleSystemDefinition, s16 ID);

void ParticleSystem_constructor(ParticleSystem this, const ParticleSystemDefinition* particleSystemDefinition, s16 id);
void ParticleSystem_destructor(ParticleSystem this);
void ParticleSystem_update(ParticleSystem this);
void ParticleSystem_transform(ParticleSystem this, Transformation* environmentTransform);
bool ParticleSystem_handleMessage(ParticleSystem this, Telegram telegram);
void ParticleSystem_show(ParticleSystem this);
void ParticleSystem_hide(ParticleSystem this);
void ParticleSystem_suspend(ParticleSystem this);
void ParticleSystem_resume(ParticleSystem this);
void ParticleSystem_start(ParticleSystem this);
void ParticleSystem_pause(ParticleSystem this);


#endif