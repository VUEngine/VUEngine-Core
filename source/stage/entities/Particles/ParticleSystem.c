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

friend class VirtualNode;
friend class VirtualList;



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param particleSystemDefinition	Definition of the ParticleSystem
 * @param id
 * @param internalId
 * @param name
 */
void ParticleSystem::constructor(ParticleSystemDefinition* particleSystemDefinition, s16 id, s16 internalId, const char* const name)
{
	// construct base
	Base::constructor(&particleSystemDefinition->entityDefinition, id, internalId, name);

	// save definition
	this->particleSystemDefinition = particleSystemDefinition;

	this->particles = new VirtualList();
	this->recyclableParticles = this->particleSystemDefinition->recycleParticles ? new VirtualList() : NULL;
	this->expiredParticles = new VirtualList();

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
 */
void ParticleSystem::destructor()
{
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
				delete node->data;
			}

			delete this->particles;
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
				delete node->data;
			}

			delete this->recyclableParticles;
		}

		this->recyclableParticles = NULL;
	}

	if(this->expiredParticles)
	{
		ASSERT(!VirtualList::getSize(this->expiredParticles), "ParticleSystem::destructor: expiredParticles not clean");

		delete this->expiredParticles;
		this->expiredParticles = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * @private
 */
void ParticleSystem::processExpiredParticles()
{
	VirtualNode node = this->expiredParticles->head;

	if(this->particleSystemDefinition->recycleParticles)
	{
		for(; node; node = node->next)
		{
			Particle particle = Particle::safeCast(node->data);
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
			Particle particle = Particle::safeCast(node->data);
			VirtualList::removeElement(this->particles, particle);

			delete particle;
			this->particleCount--;
		}

		VirtualList::clear(this->expiredParticles);
	}
}

/**
 * @param elapsedTime
 */
void ParticleSystem::update(u32 elapsedTime)
{
	Base::update(this, elapsedTime);

	ParticleSystem::processExpiredParticles(this);

	// update each particle
	VirtualNode node = this->particles->head;

	void (* behavior)(Particle particle) = this->particleSystemDefinition->particleDefinition->behavior;

	for(; node; node = node->next)
	{
		if( Particle::update(node->data, elapsedTime, behavior) || !Particle::isVisible(node->data))
		{
			ParticleSystem::particleExpired(this, Particle::safeCast(node->data));
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
 * @private
 * @return		Particle
 */
Particle ParticleSystem::recycleParticle()
{
	if(this->recyclableParticles->head && (VirtualList::getSize(this->particles) + VirtualList::getSize(this->recyclableParticles) >= this->particleSystemDefinition->maximumNumberOfAliveParticles))
	{
		long seed = Utilities::randomSeed();

		int lifeSpan = this->particleSystemDefinition->particleDefinition->minimumLifeSpan + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->lifeSpanDelta);
		fix10_6 mass = this->particleSystemDefinition->particleDefinition->minimumMass + Utilities::random(seed, this->particleSystemDefinition->particleDefinition->massDelta);

		// call the appropriate allocator to support inheritance
		Particle particle = Particle::safeCast(VirtualList::front(this->recyclableParticles));

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
 * @private
 * @param seed
 * @return		Spawn position
 */
const Vector3D* ParticleSystem::getParticleSpawnPosition(long seed)
{
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
 * @private
 * @param seed
 * @return		Force
 */
const Force* ParticleSystem::getParticleSpawnForce(long seed)
{
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
 */
void ParticleSystem::spawnAllParticles()
{
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
 * @private
 */
Particle ParticleSystem::spawnParticle()
{
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
 * @param environmentTransform
 */
void ParticleSystem::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	ParticleSystem::processExpiredParticles(this);

	this->invalidateSprites |= invalidateTransformationFlag | Entity::updateSpritePosition(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		 Particle::transform(node->data);
	}

}

void ParticleSystem::synchronizeGraphics()
{
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
 * @param telegram	The received message
 * @return			Always returns false
 */
bool ParticleSystem::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

void ParticleSystem::show()
{
	Base::show(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle::show(node->data);
	}
}

void ParticleSystem::hide()
{
	Base::hide(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle::hide(node->data);
	}
}

void ParticleSystem::resume()
{
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
		Particle::hide(node->data);
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
}

void ParticleSystem::suspend()
{
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
 * @private
 * @param particle
 */
void ParticleSystem::particleExpired(Particle particle)
{
	VirtualList::pushBack(this->expiredParticles, particle);
	Particle::hide(particle);
}

/**
 * @private
 * @return		Time
 */
int ParticleSystem::computeNextSpawnTime()
{
	return this->particleSystemDefinition->minimumSpawnDelay +
			Utilities::random(Utilities::randomSeed(), this->particleSystemDefinition->spawnDelayDelta);
}

void ParticleSystem::start()
{
	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	this->paused = false;
}

void ParticleSystem::pause()
{
	this->paused = true;
}
