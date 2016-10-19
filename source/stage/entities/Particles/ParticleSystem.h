/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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

#define ParticleSystem_METHODS(ClassName)																\
		Entity_METHODS(ClassName)																		\

#define ParticleSystem_SET_VTABLE(ClassName)															\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ParticleSystem, update);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, transform);											\
		__VIRTUAL_SET(ClassName, ParticleSystem, updateVisualRepresentation);							\
		__VIRTUAL_SET(ClassName, ParticleSystem, resume);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, suspend);												\
		__VIRTUAL_SET(ClassName, ParticleSystem, hide);												    \

#define ParticleSystem_ATTRIBUTES																		\
        Entity_ATTRIBUTES																				\
        const ParticleSystemDefinition* particleSystemDefinition;										\
        VirtualList particles;																			\
        VirtualList recyclableParticles;																\
        VirtualList expiredParticles;																	\
        int nextSpawnTime;																				\
        int particleCount;																				\
        s16 numberOfSpriteDefinitions;																	\
        bool paused;																					\

__CLASS(ParticleSystem);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Defines a ParticleSystem
 *
 * @memberof    ParticleSystem
 */
typedef struct ParticleSystemDefinition
{
    /** it has an Entity at the beginning */
    EntityDefinition entityDefinition;

    /** whether to delete or reuse expired particles */
	u8 recycleParticles;

    /** minimun generation delay in milliseconds */
	u16 minimumSpawnDelay;

    /** generation delay delta in milliseconds */
	u16 spawnDelayDelta;

    /** maximum total particles */
	u8 maximumNumberOfAliveParticles;

    /** array of sprites to select randomly */
	const ObjectSpriteDefinition** objectSpriteDefinitions;

    /** auto start */
	bool autoStart;

    /** particle's definition */
	ParticleDefinition* particleDefinition;

    /** minimum random distance from the center of the system for spanw */
	VBVec3D minimumSpanDistance;

    /** minimum relative spawn position */
	VBVec3D minimumRelativeSpanPosition;

    /** maximum relative spawn position */
	VBVec3D maximumRelativeSpanPosition;

    /** minimum force to apply (use int values in the definition to avoid overflow) */
	VBVec3D minimumForce;

    /** maximum force to apply (use int values in the definition to avoid overflow) */
	VBVec3D maximumForce;

    /** type of movement for the particles */
	u32 movementType;

} ParticleSystemDefinition;

/**
 * Defines a ParticleSystem in ROM memory
 *
 * @memberof    ParticleSystem
 * @extends     ParticleSystemDefinition
 */
typedef const ParticleSystemDefinition ParticleSystemROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
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
