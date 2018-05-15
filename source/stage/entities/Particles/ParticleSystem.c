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

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static Particle ParticleSystem::recycleParticle(ParticleSystem this);
static Particle ParticleSystem::spawnParticle(ParticleSystem this);
static void ParticleSystem::processExpiredParticles(ParticleSystem this);
static void ParticleSystem::particleExpired(ParticleSystem this, Particle particle);
static int ParticleSystem::computeNextSpawnTime(ParticleSystem this);
static const Vector3D* ParticleSystem::getParticleSpawnPosition(ParticleSystem this, long seed);
static const Force* ParticleSystem::getParticleSpawnForce(ParticleSystem this, long seed);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

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
void ParticleSystem::constructor(ParticleSystem this, ParticleSystemDefinition* particleSystemDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ParticleSystem::constructor: null this");

	// construct base
	Base::constructor(&particleSystemDefinition->entityDefinition, id, internalId, name);

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

	this->nextSpawnTime = this->paused ? 0 : ParticleSystem::computeNextSpawnTime(this);

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
void ParticleSystem::destructor(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::destructor: null this");

	ParticleSystem::processExpiredParticles(this);

	ParticleRemover particleRemover = Stage::getParticleRemover(Game::getStage(Game::getInstance()));

	if(this->particles)
	{
		// the remover handles all the cleaning
		if(particleRemover)
		{
			ParticleRemover::deleteParticles(particleRemover, this->particles);
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
			ParticleRemover::deleteParticles(particleRemover, this->recyclableParticles);
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
		ASSERT(!VirtualList::getSize(this->expiredParticles), "ParticleSystem::destructor: expiredParticles not clean");

		__DELETE(this->expiredParticles);
		this->expiredParticles = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 */
static void ParticleSystem::processExpiredParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::processExpiredParticles: null this");

	VirtualNode node = this->expiredParticles->head;

	if(this->particleSystemDefinition->recycleParticles)
	{
		for(; node; node = node->next)
		{
			Particle particle = __SAFE_CAST(Particle, node->data);
			VirtualList::pushBack(this->recyclableParticles, particle);
			VirtualList::removeElement(this->particles, particle);
			this->particleCount--;
		}

		VirtualList::clear(this->expiredParticles);
	}
	else
	{
		for(; node; node = node->next)
		{
			Particle particle = __SAFE_CAST(Particle, node->data);
			VirtualList::removeElement(this->particles, particle);

			__DELETE(particle);
			this->particleCount--;
		}

		VirtualList::clear(this->expiredParticles);
	}
}

/**
 * @memberof			ParticleSystem
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 */
void ParticleSystem::update(ParticleSystem this, u32 elapsedTime)
{
	ASSERT(this, "ParticleSystem::update: null this");

	Base::update(this, elapsedTime);

	ParticleSystem::processExpiredParticles(this);

	// update each particle
	VirtualNode node = this->particles->head;

	void (* behavior)(Particle particle) = this->particleSystemDefinition->particleDefinition->behavior;

	for(; node; node = node->next)
	{
		if( Particle::update(node->data, elapsedTime, behavior) || !Particle::isVisible(__SAFE_CAST(Particle, node->data)))
		{
			ParticleSystem::particleExpired(this, __SAFE_CAST(Particle, node->data));
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
				VirtualList::pushBack(this->particles, ParticleSystem::recycleParticle(this));
				this->particleCount++;
			}
			else
			{
				VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
				this->particleCount++;
			}

			this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
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
static Particle ParticleSystem::recycleParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::recycleParticle: null this");

	if(this->recyclableParticles->head && (VirtualList::getSize(this->particles) + VirtualList::getSize(this->recyclableParticles) >= this->particleSystemDefinition->maximumNumberOfAliveParticles))
	{
		long seed = Utilities::randomSeed();

		int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
		fix10_6 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

		// call the appropriate allocator to support inheritance
		Particle particle = __SAFE_CAST(Particle, VirtualList::front(this->recyclableParticles));

		 Particle::reset(particle);
		Particle::setLifeSpan(particle, lifeSpan);
		Particle::setMass(particle, mass);
		 Particle::setPosition(particle, ParticleSystem::getParticleSpawnPosition(this, seed));
		Particle::addForce(particle, ParticleSystem::getParticleSpawnForce(this, seed), this->particleSystemDefinition->movementType);
		Particle::show(particle);

		VirtualList::popFront(this->recyclableParticles);

		return particle;
	}

	return ParticleSystem::spawnParticle(this);
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
static const Vector3D* ParticleSystem::getParticleSpawnPosition(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnPosition: null this");

	static Vector3D position =
	{
		0, 0, 0
	};

	position.x = this->transformation.globalPosition.x + this->particleSystemDefinition->minimumRelativeSpawnPosition.x + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.x - this->particleSystemDefinition->minimumRelativeSpawnPosition.x));
	position.y = this->transformation.globalPosition.y + this->particleSystemDefinition->minimumRelativeSpawnPosition.y + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.y - this->particleSystemDefinition->minimumRelativeSpawnPosition.y));
	position.z = this->transformation.globalPosition.z + this->particleSystemDefinition->minimumRelativeSpawnPosition.z + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumRelativeSpawnPosition.z - this->particleSystemDefinition->minimumRelativeSpawnPosition.z));

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
static const Force* ParticleSystem::getParticleSpawnForce(ParticleSystem this, long seed)
{
	ASSERT(this, "ParticleSystem::getParticleSpawnForce: null this");

	static Force force =
	{
		0, 0, 0
	};

	force.x = this->particleSystemDefinition->minimumForce.x + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumForce.x - this->particleSystemDefinition->minimumForce.x));
	force.y = this->particleSystemDefinition->minimumForce.y + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumForce.y - this->particleSystemDefinition->minimumForce.y));
	force.z = this->particleSystemDefinition->minimumForce.z + Utilities::random(seed, __ABS(this->particleSystemDefinition->maximumForce.z - this->particleSystemDefinition->minimumForce.z));

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
void ParticleSystem::spawnAllParticles(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnAllParticles: null this");

	while(this->particleCount < this->particleSystemDefinition->maximumNumberOfAliveParticles)
	{
		if(this->particleSystemDefinition->recycleParticles)
		{
			VirtualList::pushBack(this->particles, ParticleSystem::recycleParticle(this));
			this->particleCount++;
		}
		else
		{
			VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
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
static Particle ParticleSystem::spawnParticle(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::spawnParticle: null this");

	long seed = Utilities::randomSeed();

	int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
	fix10_6 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

	int spriteDefinitionIndex = 0;

	if(1 < this->numberOfSpriteDefinitions)
	{
		spriteDefinitionIndex = Utilities::random(seed, this->numberOfSpriteDefinitions);
	}

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleDefinition*, const SpriteDefinition*, int, fix10_6)) this->particleSystemDefinition->particleDefinition->allocator)(this->particleSystemDefinition->particleDefinition, (const SpriteDefinition*)this->particleSystemDefinition->objectSpriteDefinitions[spriteDefinitionIndex], lifeSpan, mass);
	 Particle::setPosition(particle, ParticleSystem::getParticleSpawnPosition(this, seed));
	Particle::addForce(particle, ParticleSystem::getParticleSpawnForce(this, seed), this->particleSystemDefinition->movementType);

	return particle;
}

/**
 * @memberof					ParticleSystem
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void ParticleSystem::transform(ParticleSystem this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "ParticleSystem::transform: null this");

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	ParticleSystem::processExpiredParticles(this);

	this->invalidateSprites |= invalidateTransformationFlag | Entity::updateSpritePosition(__SAFE_CAST(Entity, this));

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle::transform(node->data);
	}

}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::synchronizeGraphics(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::synchronizeGraphics: null this");

	VirtualNode node = this->particles->head;

	bool updateSprites = this->invalidateSprites ? true : false;

	for(; node; node = node->next)
	{
		 Particle::synchronizeGraphics(node->data, updateSprites);
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
bool ParticleSystem::handleMessage(ParticleSystem this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
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
void ParticleSystem::show(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::show: null this");

	Base::show(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle::show(__SAFE_CAST(Particle, node->data));
	}
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::hide(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::hide: null this");

	Base::hide(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle::hide(__SAFE_CAST(Particle, node->data));
	}
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::resume(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::resume: null this");

	Base::resume(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle::resume(node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			 Particle::resume(node->data);
		}
	}

	node = this->expiredParticles->head;

	for(; node; node = node->next)
	{
		 Particle::resume(node->data);
		Particle::hide(__SAFE_CAST(Particle, node->data));
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::suspend(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::suspend: null this");

	Base::suspend(this);

	ParticleSystem::processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle::suspend(node->data);
	}

	if(this->recyclableParticles)
	{
		node = this->recyclableParticles->head;

		for(; node; node = node->next)
		{
			 Particle::suspend(node->data);
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
static void ParticleSystem::particleExpired(ParticleSystem this, Particle particle)
{
	ASSERT(this, "ParticleSystem::particleExpired: null this");

	VirtualList::pushBack(this->expiredParticles, particle);
	Particle::hide(particle);
}

/**
 * @memberof	ParticleSystem
 * @private
 *
 * @param this	Function scope
 *
 * @return		Time
 */
static int ParticleSystem::computeNextSpawnTime(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::computeNextSpawnTime: null this");

	return this->particleSystemDefinition->minimumSpawnDelay +
			Utilities::random(Utilities::randomSeed(), this->particleSystemDefinition->spawnDelayDelta);
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::start(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::start: null this");

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	this->paused = false;
}

/**
 * @memberof	ParticleSystem
 * @public
 *
 * @param this	Function scope
 */
void ParticleSystem::pause(ParticleSystem this)
{
	ASSERT(this, "ParticleSystem::pause: null this");

	this->paused = true;
}
