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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleSystem.h>
#include <Game.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ParticleSystem
__CLASS_DEFINITION(ParticleSystem, Entity);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
const extern VBVec3D* _screenDisplacement;
extern const Optical* _optical;

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
__CLASS_NEW_DEFINITION(ParticleSystem, ParticleSystemDefinition* particleSystemDefinition, s16 id, const char* const name)
__CLASS_NEW_END(ParticleSystem, particleSystemDefinition, id, name);

// class's constructor
void ParticleSystem_constructor(ParticleSystem this, ParticleSystemDefinition* particleSystemDefinition, s16 id, const char* const name)
{
	ASSERT(this, "ParticleSystem::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(Entity, &particleSystemDefinition->entityDefinition, id, name);

	// save definition
	this->particleSystemDefinition = particleSystemDefinition;

	this->particles = __NEW(VirtualList);
	this->recyclableParticles = this->particleSystemDefinition->recycleParticles? __NEW(VirtualList) : NULL;
	this->expiredParticles = __NEW(VirtualList);

	this->particleCount = 0;
	this->paused = !this->particleSystemDefinition->autoStart;

	// set size from definition if there are not no sprites to be added
	this->size.x += FIX19_13TOI(abs(this->particleSystemDefinition->maximumRelativeSpanPosition.x - this->particleSystemDefinition->minimumRelativeSpanPosition.x));
	this->size.y += FIX19_13TOI(abs(this->particleSystemDefinition->maximumRelativeSpanPosition.y - this->particleSystemDefinition->minimumRelativeSpanPosition.y));
	this->size.z += FIX19_13TOI(abs(this->particleSystemDefinition->maximumRelativeSpanPosition.z - this->particleSystemDefinition->minimumRelativeSpanPosition.z));

	// retrieve clock
	this->clock = Game_getInGameClock(Game_getInstance());
	this->previousTime = 0;

	this->nextSpawnTime = this->paused ? 0 : ParticleSystem_computeNextSpawnTime(this);

	// calculate the numbe of sprite definitions
	for(this->numberOfSpriteDefinitions = 0; 0 <= (int)this->numberOfSpriteDefinitions && this->particleSystemDefinition->objectSpriteDefinitions[this->numberOfSpriteDefinitions]; this->numberOfSpriteDefinitions++);

	ASSERT(0 < this->numberOfSpriteDefinitions, "ParticleSystem::constructor: 0 sprite definitions");
}

// class's destructor
void ParticleSystem_destructor(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::destructor: null this");

	ParticleSystem_hide(this);

	ParticleSystem_processExpiredParticles(this);

	if(this->particles)
	{
		VirtualNode node = this->particles->head;

		for(; node; node = node->next)
		{
			__DELETE(node->data);
		}

		__DELETE(this->particles);
		this->particles = NULL;
	}

	if(this->recyclableParticles)
	{
		VirtualNode node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			__DELETE(node->data);
		}

		__DELETE(this->recyclableParticles);
		this->recyclableParticles = NULL;
	}

	if(this->expiredParticles)
	{
		ASSERT(!VirtualList_getSize(this->expiredParticles), "ParticleSystem::destructor: expiredParticles not clean");

		__DELETE(this->expiredParticles);
		this->expiredParticles = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}


static void ParticleSystem_processExpiredParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::processExpiredParticles: null this");

	VirtualNode node = this->expiredParticles->head;

	if(this->particleSystemDefinition->recycleParticles)
	{
		for(; node; node = node->next)
		{
			Particle particle = __SAFE_CAST(Particle, node->data);
			VirtualList_pushBack(this->recyclableParticles, particle);
			VirtualList_removeElement(this->particles, particle);
			this->particleCount--;
		}

		VirtualList_clear(this->expiredParticles);
	}
	else
	{
		for(; node; node = node->next)
		{
			Particle particle = __SAFE_CAST(Particle, node->data);
			VirtualList_removeElement(this->particles, particle);

			__DELETE(particle);
			this->particleCount--;
		}

		VirtualList_clear(this->expiredParticles);
	}
}

void ParticleSystem_update(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::update: null this");

	Container_update(__SAFE_CAST(Container, this));

	ParticleSystem_processExpiredParticles(this);

	if(!Clock_isPaused(this->clock))
	{
		u32 currentTime = Clock_getTime(this->clock);
	    u32 elapsedTime = currentTime - this->previousTime;
	    this->previousTime = currentTime;

	    // update each particle
	    VirtualNode node = this->particles->head;

	    void (* behavior)(Particle particle) = this->particleSystemDefinition->particleDefinition->behavior;

	    for(; node; node = node->next)
	    {
	        __VIRTUAL_CALL(void, Particle, update, node->data, elapsedTime, behavior);
	    }

		if(!this->paused)
		{
			// check if it is time to spawn new particles
			this->nextSpawnTime -= abs(elapsedTime);

			if(0 > this->nextSpawnTime)
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
}


static Particle ParticleSystem_recycleParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::recycleParticle: null this");

	if(this->recyclableParticles->head && (VirtualList_getSize(this->particles) + VirtualList_getSize(this->recyclableParticles) >= this->particleSystemDefinition->maximumNumberOfAliveParticles))
	{
		long seed = Utilities_randomSeed();

		int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
		fix19_13 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

		// call the appropriate allocator to support inheritance
		Particle particle = __SAFE_CAST(Particle, VirtualList_front(this->recyclableParticles));

		Particle_setLifeSpan(particle, lifeSpan);
		Particle_setMass(particle, mass);
		__VIRTUAL_CALL(void, Particle, setPosition, particle, ParticleSystem_getParticleSpawnPosition(this, seed));
		Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed));
		Particle_show(particle);

		VirtualList_popFront(this->recyclableParticles);

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

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(ParticleDefinition*, ...)) this->particleSystemDefinition->particleDefinition->allocator)(this->particleSystemDefinition->particleDefinition, this->particleSystemDefinition->objectSpriteDefinitions[spriteDefinitionIndex], lifeSpan, mass);
	__VIRTUAL_CALL(void, Particle, setPosition, particle, ParticleSystem_getParticleSpawnPosition(this, seed));
	Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed));

	Object_addEventListener(__SAFE_CAST(Object, particle), __SAFE_CAST(Object, this), (void (*)(Object, Object))ParticleSystem_onParticleExipired, __EVENT_PARTICLE_EXPIRED);

	return particle;
}

void ParticleSystem_transform(ParticleSystem this, const Transformation* environmentTransform)
{
	ASSERT(this, "ParticleSystem::transform: null this");

	Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);

	ParticleSystem_processExpiredParticles(this);

	this->updateSprites |= __VIRTUAL_CALL(bool, Entity, updateSpritePosition, this)? __UPDATE_SPRITE_POSITION : 0;
}

void ParticleSystem_updateVisualRepresentation(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::updateVisualRepresentation: null this");

	VirtualNode node = this->particles->head;

	bool updateSprites = this->updateSprites? true : false;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(void, Particle, updateVisualRepresentation, node->data, updateSprites);
	}

	this->updateSprites = 0;
}

bool ParticleSystem_handleMessage(ParticleSystem this, Telegram telegram)
{
	ASSERT(this, "ParticleSystem::handleMessage: null this");

	return false;
}

void ParticleSystem_show(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::show: null this");

	Entity_show(__SAFE_CAST(Entity, this));

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle_show(__SAFE_CAST(Particle, node->data));
	}
}

void ParticleSystem_hide(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::hide: null this");

	Entity_hide(__SAFE_CAST(Entity, this));

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle_hide(__SAFE_CAST(Particle, node->data));
	}
}

void ParticleSystem_resume(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::resume: null this");

	Entity_resume(__SAFE_CAST(Entity, this));

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(void, Particle, resume, node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Particle, resume, node->data);
		}
	}

	node = this->expiredParticles->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(void, Particle, resume, node->data);
		Particle_hide(__SAFE_CAST(Particle, node->data));
	}

	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);
}

void ParticleSystem_suspend(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::suspend: null this");

	Entity_suspend(__SAFE_CAST(Entity, this));

	ParticleSystem_processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(void, Particle, suspend, node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Particle, suspend, node->data);
		}
	}
}

static void ParticleSystem_onParticleExipired(ParticleSystem this, Object eventFirer)
{
	ASSERT(this, "ParticleSystem::onParticleExipired: null this");
	ASSERT(__SAFE_CAST(Particle, eventFirer), "ParticleSystem::onParticleExipired: null this");

	VirtualList_pushBack(this->expiredParticles, eventFirer);
	Particle_hide(__SAFE_CAST(Particle, eventFirer));
}

static int ParticleSystem_computeNextSpawnTime(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::computeNextSpawnTime: null this");

	return this->particleSystemDefinition->minimumSpawnDelay +
			Utilities_random(Utilities_randomSeed(), this->particleSystemDefinition->spawnDelayDelta);
}

void ParticleSystem_start(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::start: null this");

	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);

	this->paused = false;
}

void ParticleSystem_pause(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::pause: null this");

	this->paused = true;
}
