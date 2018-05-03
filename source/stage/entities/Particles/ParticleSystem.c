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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleSystem.h>
#include <Game.h>
#include <ParticleRemover.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ParticleSystem
 * @extends Entity
 * @ingroup stage-entities-particles
 */
__CLASS_DEFINITION(ParticleSystem, Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static Particle ParticleSystem_recycleParticle(ParticleSystem this);
static Particle ParticleSystem_spawnParticle(ParticleSystem this);
static void ParticleSystem_processExpiredParticles(ParticleSystem this);
static void ParticleSystem_particleExpired(ParticleSystem this, Particle particle);
static int ParticleSystem_computeNextSpawnTime(ParticleSystem this);
static const Vector3D* ParticleSystem_getParticleSpawnPosition(ParticleSystem this, long seed);
static const Force* ParticleSystem_getParticleSpawnForce(ParticleSystem this, long seed);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ParticleSystem, ParticleSystemDefinition* particleSystemDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ParticleSystem, particleSystemDefinition, id, internalId, name);

/**
 * Class constructor
 *
 * @memberof						ParticleSystem
 * @public
 *
 * @param this						Function scope
 * @param particleSystemDefinition	Definition of the ParticleSystem
 * @param id
 * @param internalId
 * @param name
 */
void ParticleSystem_constructor(ParticleSystem this, ParticleSystemDefinition* particleSystemDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ParticleSystem::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(Entity, &particleSystemDefinition->entityDefinition, id, internalId, name);

	// save definition
	this->particleSystemDefinition = particleSystemDefinition;

	this->particles = __NEW(VirtualList);
	this->recyclableParticles = this->particleSystemDefinition->recycleParticles ? __NEW(VirtualList) : NULL;
	this->expiredParticles = __NEW(VirtualList);

	this->particleCount = 0;
	this->paused = !this->particleSystemDefinition->autoStart;

	// set size from definition
	this->size.x += __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.x - this->particleSystemDefinition->minimumRelativeSpawnPosition.x);
	this->size.y += __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.y - this->particleSystemDefinition->minimumRelativeSpawnPosition.y);
	this->size.z += __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.z - this->particleSystemDefinition->minimumRelativeSpawnPosition.z);

	this->nextSpawnTime = this->paused ? 0 : ParticleSystem_computeNextSpawnTime(this);

	// calculate the number of sprite definitions
	for(this->numberOfSpriteDefinitions = 0; 0 <= this->numberOfSpriteDefinitions && this->particleSystemDefinition->objectSpriteDefinitions[this->numberOfSpriteDefinitions]; this->numberOfSpriteDefinitions++);

	ASSERT(0 < this->numberOfSpriteDefinitions, "ParticleSystem::constructor: 0 sprite definitions");
}

/**
 * Class destructor
 *
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_destructor(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::destructor: null this");

	ParticleSystem_processExpiredParticles(this);

	ParticleRemover particleRemover = Stage_getParticleRemover(Game_getStage(Game_getInstance()));

	if(this->particles)
	{
		// the remover handles all the cleaning
		if(particleRemover)
		{
			ParticleRemover_deleteParticles(particleRemover, this->particles);
		}
		else
		{
			VirtualNode node = this->particles->head;

			for(; node; node = node->next)
			{
				__DELETE(node->data);
			}

			__DELETE(this->particles);
		}

		this->particles = NULL;
	}

	if(this->recyclableParticles)
	{
		// the remover handles all the cleaning
		if(particleRemover)
		{
			ParticleRemover_deleteParticles(particleRemover, this->recyclableParticles);
		}
		else
		{
			VirtualNode node = this->recyclableParticles->head;

			for(; node; node = node->next)
			{
				__DELETE(node->data);
			}

			__DELETE(this->recyclableParticles);
		}

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

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 */
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

/**
 * @memberof			ParticleSystem
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 */
void ParticleSystem_update(ParticleSystem this, u32 elapsedTime)
{
	ASSERT(this, "ParticleSystem::update: null this");

	Base_update(this, elapsedTime);

	ParticleSystem_processExpiredParticles(this);

	// update each particle
	VirtualNode node = this->particles->head;

	void (* behavior)(Particle particle) = this->particleSystemDefinition->particleDefinition->behavior;

	for(; node; node = node->next)
	{
		if( Particle_update(node->data, elapsedTime, behavior) || !Particle_isVisible(__SAFE_CAST(Particle, node->data)))
		{
			ParticleSystem_particleExpired(this, __SAFE_CAST(Particle, node->data));
		}
	}

	if(this->paused)
	{
		return;
	}

	// check if it is time to spawn new particles
	this->nextSpawnTime -= elapsedTime;

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

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 *
 * @return		Particle
 */
static Particle ParticleSystem_recycleParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::recycleParticle: null this");

	if(this->recyclableParticles->head && (VirtualList_getSize(this->particles) + VirtualList_getSize(this->recyclableParticles) >= this->particleSystemDefinition->maximumNumberOfAliveParticles))
	{
		long seed = Utilities_randomSeed();

		int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
		fix10_6 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

		// call the appropriate allocator to support inheritance
		Particle particle = __SAFE_CAST(Particle, VirtualList_front(this->recyclableParticles));

		 Particle_reset(particle);
		Particle_setLifeSpan(particle, lifeSpan);
		Particle_setMass(particle, mass);
		 Particle_setPosition(particle, ParticleSystem_getParticleSpawnPosition(this, seed));
		Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed), this->particleSystemDefinition->movementType);
		Particle_show(particle);

		VirtualList_popFront(this->recyclableParticles);

		return particle;
	}

	return ParticleSystem_spawnParticle(this);
}

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 * @param seed
 *
 * @return		Spawn position
 */
static const Vector3D* ParticleSystem_getParticleSpawnPosition(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnPosition: null this");

	static Vector3D position =
	{
		0, 0, 0
	};

	position.x = this->transformation.globalPosition.x + this->particleSystemDefinition->minimumRelativeSpawnPosition.x + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.x - this->particleSystemDefinition->minimumRelativeSpawnPosition.x));
	position.y = this->transformation.globalPosition.y + this->particleSystemDefinition->minimumRelativeSpawnPosition.y + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.y - this->particleSystemDefinition->minimumRelativeSpawnPosition.y));
	position.z = this->transformation.globalPosition.z + this->particleSystemDefinition->minimumRelativeSpawnPosition.z + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.z - this->particleSystemDefinition->minimumRelativeSpawnPosition.z));

	return &position;
}

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 * @param seed
 *
 * @return		Force
 */
static const Force* ParticleSystem_getParticleSpawnForce(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnForce: null this");

	static Force force =
	{
		0, 0, 0
	};

	force.x = this->particleSystemDefinition->minimumForce.x + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumForce.x - this->particleSystemDefinition->minimumForce.x));
	force.y = this->particleSystemDefinition->minimumForce.y + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumForce.y - this->particleSystemDefinition->minimumForce.y));
	force.z = this->particleSystemDefinition->minimumForce.z + Utilities_random(seed, __ABS(this->particleSystemDefinition->maximumForce.z - this->particleSystemDefinition->minimumForce.z));

	return &force;
}

/**
 * Spawn all particles at once. This function's intended use is for recyclable particles mainly.
 *
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_spawnAllParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnAllParticles: null this");

	while(this->particleCount < this->particleSystemDefinition->maximumNumberOfAliveParticles)
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
	}
}

/**
 * Spawn a particle
 *
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 */
static Particle ParticleSystem_spawnParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnParticle: null this");

	long seed = Utilities_randomSeed();

	int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
	fix10_6 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities_random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

	int spriteDefinitionIndex = 0;

	if(1 < this->numberOfSpriteDefinitions)
	{
		spriteDefinitionIndex = Utilities_random(seed, this->numberOfSpriteDefinitions);
	}

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleDefinition*, const SpriteDefinition*, int, fix10_6)) this->particleSystemDefinition->particleDefinition->allocator)(this->particleSystemDefinition->particleDefinition, (const SpriteDefinition*)this->particleSystemDefinition->objectSpriteDefinitions[spriteDefinitionIndex], lifeSpan, mass);
	 Particle_setPosition(particle, ParticleSystem_getParticleSpawnPosition(this, seed));
	Particle_addForce(particle, ParticleSystem_getParticleSpawnForce(this, seed), this->particleSystemDefinition->movementType);

	return particle;
}

/**
 * @memberof					ParticleSystem
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void ParticleSystem_transform(ParticleSystem this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "ParticleSystem::transform: null this");

	Base_transform(this, environmentTransform, invalidateTransformationFlag);

	ParticleSystem_processExpiredParticles(this);

	this->invalidateSprites |= invalidateTransformationFlag | Entity_updateSpritePosition(__SAFE_CAST(Entity, this));

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle_transform(node->data);
	}

}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_synchronizeGraphics(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::synchronizeGraphics: null this");

	VirtualNode node = this->particles->head;

	bool updateSprites = this->invalidateSprites ? true : false;

	for(; node; node = node->next)
	{
		 Particle_synchronizeGraphics(node->data, updateSprites);
	}

	this->invalidateSprites = 0;
}

/**
 * Handles incoming messages
 *
 * @memberof		ParticleSystem
 * @public
 *
 * @param this		Function scope
 * @param telegram	The received message
 *
 * @return			Always returns false
 */
bool ParticleSystem_handleMessage(ParticleSystem this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	ASSERT(this, "ParticleSystem::handleMessage: null this");

	return false;
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_show(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::show: null this");

	Base_show(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle_show(__SAFE_CAST(Particle, node->data));
	}
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_hide(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::hide: null this");

	Base_hide(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle_hide(__SAFE_CAST(Particle, node->data));
	}
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_resume(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::resume: null this");

	Base_resume(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle_resume(node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			 Particle_resume(node->data);
		}
	}

	node = this->expiredParticles->head;

	for(; node; node = node->next)
	{
		 Particle_resume(node->data);
		Particle_hide(__SAFE_CAST(Particle, node->data));
	}

	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_suspend(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::suspend: null this");

	Base_suspend(this);

	ParticleSystem_processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle_suspend(node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			 Particle_suspend(node->data);
		}
	}
}

/**
 * @memberof		ParticleSystem
 * @private
 *
 * @param this		Function scope
 * @param particle
 */
static void ParticleSystem_particleExpired(ParticleSystem this, Particle particle)
{
	ASSERT(this, "ParticleSystem::particleExpired: null this");

	VirtualList_pushBack(this->expiredParticles, particle);
	Particle_hide(particle);
}

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 *
 * @return		Time
 */
static int ParticleSystem_computeNextSpawnTime(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::computeNextSpawnTime: null this");

	return this->particleSystemDefinition->minimumSpawnDelay +
			Utilities_random(Utilities_randomSeed(), this->particleSystemDefinition->spawnDelayDelta);
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_start(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::start: null this");

	this->nextSpawnTime = ParticleSystem_computeNextSpawnTime(this);

	this->paused = false;
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem_pause(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::pause: null this");

	this->paused = true;
}
