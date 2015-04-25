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

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleSystem.h>
#include <Game.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ParticleSystem
__CLASS_DEFINITION(ParticleSystem, Entity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
const extern VBVec3D* _screenDisplacement;
extern const Optical* _optical;

static void ParticleSystem_spawnParticle(ParticleSystem this);
static void ParticleSystem_processExpiredParticles(ParticleSystem this);
static void ParticleSystem_onParticleExipired(ParticleSystem this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ParticleSystem, const ParticleSystemDefinition* particleSystemDefinition, s16 id)
__CLASS_NEW_END(ParticleSystem, particleSystemDefinition, id);

// class's constructor
void ParticleSystem_constructor(ParticleSystem this, const ParticleSystemDefinition* particleSystemDefinition, s16 id)
{
	ASSERT(this, "ParticleSystem::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(particleSystemDefinition->entityDefinition, id);

	// save definition
	this->particleSystemDefinition = particleSystemDefinition;
	
	this->particles = __NEW(VirtualList);
	this->expiredParticles = __NEW(VirtualList);
	
	this->paused = !this->particleSystemDefinition->autoStart;
	
	// set size from definition if there are not no sprites to be added
	this->size.x += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.x - this->particleSystemDefinition->minimumRelativeSpanPosition.x);
	this->size.y += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.y - this->particleSystemDefinition->minimumRelativeSpanPosition.y);
	this->size.z += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.z - this->particleSystemDefinition->minimumRelativeSpanPosition.z);
	
	// retrieve clock
	this->clock = Game_getInGameClock(Game_getInstance());
	
	this->lastUpdateTime = this->paused? 0: Clock_getTime(this->clock);
	this->nextSpawnTime = this->paused? 0: this->lastUpdateTime + this->particleSystemDefinition->minimumSpawnDelay + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumSpawnDelay - this->particleSystemDefinition->minimumSpawnDelay));

	// calculate the numbe of sprite definitions
	for(this->numberOfSpriteDefinitions = 0; 0 <= (int)this->numberOfSpriteDefinitions && this->particleSystemDefinition->objectSpriteDefinitions[this->numberOfSpriteDefinitions]; this->numberOfSpriteDefinitions++);
	
	ASSERT(0 < this->numberOfSpriteDefinitions, "ParticleSystem::constructor: 0 sprite definitions");
}

// class's destructor
void ParticleSystem_destructor(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::destructor: null this");

	if(this->particles)
	{
		VirtualNode node = VirtualList_begin(this->particles);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			__DELETE(VirtualNode_getData(node));
		}
		
		__DELETE(this->particles);
		this->particles = NULL;
	}
	
	if(this->expiredParticles)
	{
		__DELETE(this->expiredParticles);
	}

	// destroy the super Container
	__DESTROY_BASE;
}

static void ParticleSystem_processExpiredParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::update: null this");

	VirtualNode node = VirtualList_begin(this->expiredParticles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		VirtualList_removeElement(this->particles, VirtualNode_getData(node));
		__DELETE(VirtualNode_getData(node));
	}
	
	VirtualList_clear(this->expiredParticles);
}

void ParticleSystem_update(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::update: null this");
	
	Container_update(__UPCAST(Container, this));
	
	ParticleSystem_processExpiredParticles(this);
	
	if(!this->paused)
	{
		u32 timeElapsed = Clock_getTime(this->clock) - this->lastUpdateTime;
		
		// update each particle
		VirtualNode node = VirtualList_begin(this->particles);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			__VIRTUAL_CALL(void, Particle, update, VirtualNode_getData(node), timeElapsed, this->particleSystemDefinition->particleDefinition->behavior);
		}
		
		// check if it is time to spwn new particles
		this->lastUpdateTime = Clock_getTime(this->clock);
		
		if(this->lastUpdateTime > this->nextSpawnTime)
		{
			if(VirtualList_getSize(this->particles) < this->particleSystemDefinition->maximumNumberOfAliveParticles)
			{
				ParticleSystem_spawnParticle(this);
				this->nextSpawnTime = this->lastUpdateTime + this->particleSystemDefinition->minimumSpawnDelay + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumSpawnDelay - this->particleSystemDefinition->minimumSpawnDelay));
			}
		}
	}
}

static void ParticleSystem_spawnParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnParticle: null this");

	int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->particleDefinition->maximumLifeSpan - this->particleSystemDefinition->particleDefinition->minimumLifeSpan));
	fix19_13 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->particleDefinition->maximumMass - this->particleSystemDefinition->particleDefinition->minimumMass));

	int spriteDefinitionIndex = Utilities_random(Utilities_randomSeed(), abs(this->numberOfSpriteDefinitions));
	
	// call the appropiate allocator to support inheritance!
	Particle particle = ((Particle (*)(ParticleDefinition*, ...)) this->particleSystemDefinition->particleDefinition->allocator)(this->particleSystemDefinition->particleDefinition, this->particleSystemDefinition->objectSpriteDefinitions[spriteDefinitionIndex], lifeSpan, mass);
	
	fix19_13 x = this->particleSystemDefinition->minimumRelativeSpanPosition.x + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumRelativeSpanPosition.x - this->particleSystemDefinition->minimumRelativeSpanPosition.x));
	fix19_13 y = this->particleSystemDefinition->minimumRelativeSpanPosition.y + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumRelativeSpanPosition.y - this->particleSystemDefinition->minimumRelativeSpanPosition.y));
	fix19_13 z = this->particleSystemDefinition->minimumRelativeSpanPosition.z + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumRelativeSpanPosition.z - this->particleSystemDefinition->minimumRelativeSpanPosition.z));

	VBVec3D position = 
	{
		x + this->transform.globalPosition.x + (0 < x? this->particleSystemDefinition->minimumSpanDistance.x: -this->particleSystemDefinition->minimumSpanDistance.x),
		y + this->transform.globalPosition.y + (0 < x? this->particleSystemDefinition->minimumSpanDistance.y: -this->particleSystemDefinition->minimumSpanDistance.y),
		z + this->transform.globalPosition.z + (0 < x? this->particleSystemDefinition->minimumSpanDistance.z: -this->particleSystemDefinition->minimumSpanDistance.z)
	};
	
	Particle_setPosition(particle, &position);

	x = this->particleSystemDefinition->minimumForce.x + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumForce.x - this->particleSystemDefinition->minimumForce.x));
	y = this->particleSystemDefinition->minimumForce.y + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumForce.y - this->particleSystemDefinition->minimumForce.y));
	z = this->particleSystemDefinition->minimumForce.z + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumForce.z - this->particleSystemDefinition->minimumForce.z));

	Force force =
    {
    	ITOFIX19_13(x),
        ITOFIX19_13(y),
        ITOFIX19_13(z)
    };

	Particle_addForce(particle, &force);

	Object_addEventListener(__UPCAST(Object, particle), __UPCAST(Object, this), (void (*)(Object, Object))ParticleSystem_onParticleExipired, __EVENT_PARTICLE_EXPIRED);

	VirtualList_pushBack(this->particles, particle);
}

void ParticleSystem_initialize(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::initialize: null this");
	
	Entity_initialize(__UPCAST(Entity, this));
}

void ParticleSystem_transform(ParticleSystem this, Transformation* environmentTransform)
{
	ASSERT(this, "ParticleSystem::transform: null this");
	
	Entity_transform(__UPCAST(Entity, this), environmentTransform);
	
	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, transform, VirtualNode_getData(node));
	}
}

bool ParticleSystem_handleMessage(ParticleSystem this, Telegram telegram)
{
	ASSERT(this, "ParticleSystem::handleMessage: null this");
	
	return false;
}

void ParticleSystem_show(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::show: null this");

	Entity_show(__UPCAST(Entity, this));
	
	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		Particle_show(__UPCAST(Particle, VirtualNode_getData(node)));
	}
}

void ParticleSystem_hide(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::hide: null this");
	
	Entity_hide(__UPCAST(Entity, this));
	
	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		Particle_hide(__UPCAST(Particle, VirtualNode_getData(node)));
	}
}

void ParticleSystem_suspend(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::suspend: null this");

	Entity_suspend(__UPCAST(Entity, this));

	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__DELETE(VirtualNode_getData(node));
	}
}

static void ParticleSystem_onParticleExipired(ParticleSystem this, Object eventFirer)
{
	ASSERT(this, "ParticleSystem::onParticleExipired: null this");
	ASSERT(__UPCAST(Particle, eventFirer), "ParticleSystem::onParticleExipired: null this");

	VirtualList_pushBack(this->expiredParticles, eventFirer);
}

void ParticleSystem_resume(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::resume: null this");

	Entity_resume(__UPCAST(Entity, this));
	
	this->lastUpdateTime = this->paused? 0: Clock_getTime(this->clock);
	this->nextSpawnTime = this->paused? 0: this->lastUpdateTime + this->particleSystemDefinition->minimumSpawnDelay + Utilities_random(Utilities_randomSeed(), abs(this->particleSystemDefinition->maximumSpawnDelay - this->particleSystemDefinition->minimumSpawnDelay));
}
