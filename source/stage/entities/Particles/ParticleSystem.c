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
 * @param particleSystemSpec	Spec of the ParticleSystem
 * @param id
 * @param internalId
 * @param name
 */
void ParticleSystem::constructor(ParticleSystemSpec* particleSystemSpec, s16 id, s16 internalId, const char* const name)
{
	// construct base
	Base::constructor(&particleSystemSpec->entitySpec, id, internalId, name);

	// save spec
	this->particleSystemSpec = particleSystemSpec;

	this->particles = new VirtualList();
	this->recyclableParticles = this->particleSystemSpec->recycleParticles ? new VirtualList() : NULL;
	this->expiredParticles = new VirtualList();

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = !this->particleSystemSpec->autoStart;

	// set size from spec
	this->size.x += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.x - this->particleSystemSpec->minimumRelativeSpawnPosition.x);
	this->size.y += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.y - this->particleSystemSpec->minimumRelativeSpawnPosition.y);
	this->size.z += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.z - this->particleSystemSpec->minimumRelativeSpawnPosition.z);

	this->nextSpawnTime = this->paused ? 0 : ParticleSystem::computeNextSpawnTime(this);

	// calculate the number of sprite specs
	for(this->numberOfSpriteSpecs = 0; 0 <= this->numberOfSpriteSpecs && this->particleSystemSpec->objectSpriteSpecs[this->numberOfSpriteSpecs]; this->numberOfSpriteSpecs++);

	ASSERT(0 < this->numberOfSpriteSpecs, "ParticleSystem::constructor: 0 sprite specs");
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

void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}

bool ParticleSystem::getLoop()
{
	return this->loop;
}

/**
 * @private
 */
void ParticleSystem::processExpiredParticles()
{
	VirtualNode node = this->expiredParticles->head;

	if(this->particleSystemSpec->recycleParticles)
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

	void (* behavior)(Particle particle) = this->particleSystemSpec->particleSpec->behavior;

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
		if(this->particleCount < this->particleSystemSpec->maximumNumberOfAliveParticles)
		{
			if(++this->totalSpawnedParticles >= this->particleSystemSpec->maximumNumberOfAliveParticles &&
				!this->loop)
			{
				ParticleSystem::pause(this);
				return;
			}

			if(this->particleSystemSpec->recycleParticles)
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
	if(this->recyclableParticles->head && (VirtualList::getSize(this->particles) + VirtualList::getSize(this->recyclableParticles) >= this->particleSystemSpec->maximumNumberOfAliveParticles))
	{
		long seed = Game::getRandomSeed(Game::getInstance());

		int lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + (this->particleSystemSpec->particleSpec->lifeSpanDelta ? Utilities::random(seed, this->particleSystemSpec->particleSpec->lifeSpanDelta) : 0);

		// call the appropriate allocator to support inheritance
		Particle particle = Particle::safeCast(VirtualList::front(this->recyclableParticles));

		Particle::reset(particle);
		Particle::setLifeSpan(particle, lifeSpan);
		Particle::changeMass(particle);
		Particle::setPosition(particle, ParticleSystem::getParticleSpawnPosition(this, seed));
		Particle::addForce(particle, ParticleSystem::getParticleSpawnForce(this, seed), this->particleSystemSpec->movementType);
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

	position.x = this->transformation.globalPosition.x;
	if(this->particleSystemSpec->maximumRelativeSpawnPosition.x | this->particleSystemSpec->minimumRelativeSpawnPosition.x)
	{
		position.x += this->particleSystemSpec->minimumRelativeSpawnPosition.x + Utilities::random(seed, __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.x - this->particleSystemSpec->minimumRelativeSpawnPosition.x));
	}

	position.y = this->transformation.globalPosition.y;
	if(this->particleSystemSpec->maximumRelativeSpawnPosition.y | this->particleSystemSpec->minimumRelativeSpawnPosition.y)
	{
		position.y += this->particleSystemSpec->minimumRelativeSpawnPosition.y + Utilities::random(seed, __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.y - this->particleSystemSpec->minimumRelativeSpawnPosition.y));
	}

	position.z = this->transformation.globalPosition.z;
	if(this->particleSystemSpec->maximumRelativeSpawnPosition.z | this->particleSystemSpec->minimumRelativeSpawnPosition.z)
	{
		position.z += this->particleSystemSpec->minimumRelativeSpawnPosition.z + Utilities::random(seed, __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.z - this->particleSystemSpec->minimumRelativeSpawnPosition.z));
	}

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

	force.x = this->particleSystemSpec->minimumForce.x;
	if(this->particleSystemSpec->maximumForce.x | this->particleSystemSpec->minimumForce.x)
	{
		force.x += Utilities::random(seed, __ABS(this->particleSystemSpec->maximumForce.x - this->particleSystemSpec->minimumForce.x));
	}

	force.y = this->particleSystemSpec->minimumForce.y;
	if(this->particleSystemSpec->maximumForce.y | this->particleSystemSpec->minimumForce.y)
	{
		force.y += Utilities::random(seed, __ABS(this->particleSystemSpec->maximumForce.y - this->particleSystemSpec->minimumForce.y));
	}

	force.z = this->particleSystemSpec->minimumForce.z;
	if(this->particleSystemSpec->maximumForce.z | this->particleSystemSpec->minimumForce.z)
	{
		force.z += Utilities::random(seed, __ABS(this->particleSystemSpec->maximumForce.z - this->particleSystemSpec->minimumForce.z));
	}

	return &force;
}

/**
 * Spawn all particles at once. This function's intended use is for recyclable particles mainly.
 */
void ParticleSystem::spawnAllParticles()
{
	while(this->particleCount < this->particleSystemSpec->maximumNumberOfAliveParticles)
	{
		if(this->particleSystemSpec->recycleParticles)
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
	long seed = Game::getRandomSeed(Game::getInstance());

	int lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + Utilities::random(seed, this->particleSystemSpec->particleSpec->lifeSpanDelta);

	int spriteSpecIndex = 0;

	if(1 < this->numberOfSpriteSpecs)
	{
		spriteSpecIndex = Utilities::random(seed, this->numberOfSpriteSpecs);
	}

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*, const SpriteSpec*, int)) this->particleSystemSpec->particleSpec->allocator)(this->particleSystemSpec->particleSpec, (const SpriteSpec*)this->particleSystemSpec->objectSpriteSpecs[spriteSpecIndex], lifeSpan);
	Particle::setPosition(particle, ParticleSystem::getParticleSpawnPosition(this, seed));
	Particle::addForce(particle, ParticleSystem::getParticleSpawnForce(this, seed), this->particleSystemSpec->movementType);

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
	return this->particleSystemSpec->minimumSpawnDelay +
			(this->particleSystemSpec->spawnDelayDelta ? Utilities::random(Game::getRandomSeed(Game::getInstance()), this->particleSystemSpec->spawnDelayDelta) : 0);
}

void ParticleSystem::start()
{
	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
	this->totalSpawnedParticles = 0;
	this->paused = false;
}

void ParticleSystem::pause()
{
	this->paused = true;
}
