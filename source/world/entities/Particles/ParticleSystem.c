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

static void ParticleSystem_spawnAllParticles(ParticleSystem this);
static Particle ParticleSystem_recycleParticle(ParticleSystem this);
static Particle ParticleSystem_spawnParticle(ParticleSystem this);
static void ParticleSystem_processExpiredParticles(ParticleSystem this);
static void ParticleSystem_onParticleExipired(ParticleSystem this, Object eventFirer);
static int ParticleSystem_computeNextSpawnTime(ParticleSystem this);
static const VBVec3D* ParticleSystem_getParticleSpawnPosition(ParticleSystem this, long seed);
static const Force* ParticleSystem_getParticleSpawnForce(ParticleSystem this, long seed);


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
	__CONSTRUCT_BASE(&particleSystemDefinition->entityDefinition, id);

	// save definition
	this->particleSystemDefinition = particleSystemDefinition;
	
	this->particles = __NEW(VirtualList);
	this->expiredParticles = __NEW(VirtualList);
	
	this->particleCount = 0;
	this->paused = !this->particleSystemDefinition->autoStart;
	
	// set size from definition if there are not no sprites to be added
	this->size.x += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.x - this->particleSystemDefinition->minimumRelativeSpanPosition.x);
	this->size.y += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.y - this->particleSystemDefinition->minimumRelativeSpanPosition.y);
	this->size.z += abs(this->particleSystemDefinition->maximumRelativeSpanPosition.z - this->particleSystemDefinition->minimumRelativeSpanPosition.z);
	
	// retrieve clock
	this->clock = Game_getInGameClock(Game_getInstance());
	
	this->lastUpdateTime = this->paused ? 0 : Clock_getTime(this->clock);
	this->nextSpawnTime = this->paused ? 0 : ParticleSystem_computeNextSpawnTime(this);

	// calculate the numbe of sprite definitions
	for(this->numberOfSpriteDefinitions = 0; 0 <= (int)this->numberOfSpriteDefinitions && this->particleSystemDefinition->objectSpriteDefinitions[this->numberOfSpriteDefinitions]; this->numberOfSpriteDefinitions++);
	
	if(this->particleSystemDefinition->recycleParticles)
	{
		ParticleSystem_spawnAllParticles(this);
	}
	
	ASSERT(0 < this->numberOfSpriteDefinitions, "ParticleSystem::constructor: 0 sprite definitions");
}

// class's destructor
void ParticleSystem_destructor(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::destructor: null this");

	ParticleSystem_processExpiredParticles(this);
	
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
		VirtualNode node = VirtualList_begin(this->expiredParticles);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			__DELETE(VirtualNode_getData(node));
		}
		
		__DELETE(this->expiredParticles);
		this->expiredParticles = NULL;
	}

	// destroy the super Container
	__DESTROY_BASE;
}

static void ParticleSystem_spawnAllParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnAllParticles: null this");
	
	int i = 0;
	for(; i < this->particleSystemDefinition->maximumNumberOfAliveParticles; i++)
	{
		Particle particle = ParticleSystem_spawnParticle(this);
		Particle_hide(particle);
		VirtualList_pushBack(this->expiredParticles, particle);
	}
}

static void ParticleSystem_processExpiredParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::update: null this");

	VirtualNode node = VirtualList_begin(this->expiredParticles);

	if(this->particleSystemDefinition->recycleParticles)
	{
		for(; node; node = VirtualNode_getNext(node))
		{
			VirtualList_removeElement(this->particles, VirtualNode_getData(node));
		}
	}
	else
	{
		for(; node; node = VirtualNode_getNext(node))
		{
			VirtualList_removeElement(this->particles, VirtualNode_getData(node));
			
			__DELETE(VirtualNode_getData(node));
		}

		VirtualList_clear(this->expiredParticles);
	}
}

void ParticleSystem_update(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::update: null this");
	
	Container_update(__UPCAST(Container, this));
	
	ParticleSystem_processExpiredParticles(this);

    u32 timeElapsed = Clock_getTime(this->clock) - this->lastUpdateTime;

    // update each particle
    VirtualNode node = VirtualList_begin(this->particles);

    for(; node; node = VirtualNode_getNext(node))
    {
        __VIRTUAL_CALL(void, Particle, update, VirtualNode_getData(node), timeElapsed, this->particleSystemDefinition->particleDefinition->behavior);
    }

	if(!this->paused)
	{
		// check if it is time to spawn new particles
		this->lastUpdateTime = Clock_getTime(this->clock);
		
		if(this->lastUpdateTime > this->nextSpawnTime)
		{
			if(this->particleCount < this->particleSystemDefinition->maximumNumberOfAliveParticles)
			{
				if(this->particleSystemDefinition->recycleParticles)
				{
					VirtualList_pushBack(this->particles, ParticleSystem_recycleParticle(this));
					this->particleCount++;
				}
				else
				{
					VirtualList_pushBack(this->particles, ParticleSystem_spawnParticle(this));
					this->particleCount++;
				}
				
				this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);
			}
		}
	}
}


static Particle ParticleSystem_recycleParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::recycleParticle: null this");

	if(VirtualList_begin(this->expiredParticles))
	{
		long seed = Utilities_randomSeed();
	
		int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
		fix19_13 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->massDelta);
		
		// call the appropiate allocator to support inheritance!
		Particle particle = __UPCAST(Particle, VirtualList_front(this->expiredParticles));

		Particle_setLifeSpan(particle, lifeSpan);
		Particle_setMass(particle, mass);
		Particle_setPosition(particle, ParticleSystem_getParticleSpawnPosition(this, seed));
		Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed));

		Particle_show(particle);
	
		VirtualList_popFront(this->expiredParticles);
		
		return particle;
	}
	
	return ParticleSystem_spawnParticle(this);
}

static const VBVec3D* ParticleSystem_getParticleSpawnPosition(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnPosition: null this");

	static VBVec3D position = 
	{
		0, 0, 0
	};

	position.x = this->transform.globalPosition.x + this->particleSystemDefinition->minimumRelativeSpanPosition.x + Utilities_random(seed, abs(this->particleSystemDefinition->maximumRelativeSpanPosition.x - this->particleSystemDefinition->minimumRelativeSpanPosition.x));
	position.y = this->transform.globalPosition.y + this->particleSystemDefinition->minimumRelativeSpanPosition.y + Utilities_random(seed, abs(this->particleSystemDefinition->maximumRelativeSpanPosition.y - this->particleSystemDefinition->minimumRelativeSpanPosition.y));
	position.z = this->transform.globalPosition.z + this->particleSystemDefinition->minimumRelativeSpanPosition.z + Utilities_random(seed, abs(this->particleSystemDefinition->maximumRelativeSpanPosition.z - this->particleSystemDefinition->minimumRelativeSpanPosition.z));

	return &position;
}

static const Force* ParticleSystem_getParticleSpawnForce(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnForce: null this");

	static Force force =
    {
    	0, 0, 0
    };

	force.x = ITOFIX19_13(this->particleSystemDefinition->minimumForce.x + Utilities_random(seed, abs(this->particleSystemDefinition->maximumForce.x - this->particleSystemDefinition->minimumForce.x)));
	force.y = ITOFIX19_13(this->particleSystemDefinition->minimumForce.y + Utilities_random(seed, abs(this->particleSystemDefinition->maximumForce.y - this->particleSystemDefinition->minimumForce.y)));
	force.z = ITOFIX19_13(this->particleSystemDefinition->minimumForce.z + Utilities_random(seed, abs(this->particleSystemDefinition->maximumForce.z - this->particleSystemDefinition->minimumForce.z)));

	return &force;
}

static Particle ParticleSystem_spawnParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnParticle: null this");
	
	long seed = Utilities_randomSeed();

	int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
	fix19_13 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

	int spriteDefinitionIndex = Utilities_random(seed, abs(this->numberOfSpriteDefinitions));

	// call the appropiate allocator to support inheritance!
	Particle particle = ((Particle (*)(ParticleDefinition*, ...)) this->particleSystemDefinition->particleDefinition->allocator)(this->particleSystemDefinition->particleDefinition, this->particleSystemDefinition->objectSpriteDefinitions[spriteDefinitionIndex], lifeSpan, mass);

	Particle_setPosition(particle, ParticleSystem_getParticleSpawnPosition(this, seed));

	Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed));

	Object_addEventListener(__UPCAST(Object, particle), __UPCAST(Object, this), (void (*)(Object, Object))ParticleSystem_onParticleExipired, __EVENT_PARTICLE_EXPIRED);
	
	return particle;
}

void ParticleSystem_transform(ParticleSystem this, const Transformation* environmentTransform)
{
	ASSERT(this, "ParticleSystem::transform: null this");
	
	Entity_transform(__UPCAST(Entity, this), environmentTransform);

	ParticleSystem_processExpiredParticles(this);

	bool updateSpritePosition = __VIRTUAL_CALL(bool, Entity, updateSpritePosition, this);

	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, transform, VirtualNode_getData(node), updateSpritePosition);
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

void ParticleSystem_resume(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::resume: null this");

	Entity_resume(__UPCAST(Entity, this));

	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, resume, VirtualNode_getData(node));
	}

	node = VirtualList_begin(this->expiredParticles);
		
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, resume, VirtualNode_getData(node));
		Particle_hide(__UPCAST(Particle, VirtualNode_getData(node)));
	}
	
	this->lastUpdateTime = Clock_getTime(this->clock);
	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);
}

void ParticleSystem_suspend(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::suspend: null this");

	Entity_suspend(__UPCAST(Entity, this));

	ParticleSystem_processExpiredParticles(this);
	
	VirtualNode node = VirtualList_begin(this->particles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, suspend, VirtualNode_getData(node));
	}
	
	node = VirtualList_begin(this->expiredParticles);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Particle, suspend, VirtualNode_getData(node));
	}
}

static void ParticleSystem_onParticleExipired(ParticleSystem this, Object eventFirer)
{
	ASSERT(this, "ParticleSystem::onParticleExipired: null this");
	ASSERT(__UPCAST(Particle, eventFirer), "ParticleSystem::onParticleExipired: null this");

	VirtualList_pushBack(this->expiredParticles, eventFirer);
	Particle_hide(__UPCAST(Particle, eventFirer));
	this->particleCount--;
}

static int ParticleSystem_computeNextSpawnTime(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::computeNextSpawnTime: null this");

	return this->lastUpdateTime +
			this->particleSystemDefinition->minimumSpawnDelay +
			Utilities_random(Utilities_randomSeed(), this->particleSystemDefinition->spawnDelayDelta);
}

void ParticleSystem_start(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::start: null this");

	this->lastUpdateTime = Clock_getTime(this->clock);
	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);

	this->paused = false;
}

void ParticleSystem_pause(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::pause: null this");

	this->paused = true;
}