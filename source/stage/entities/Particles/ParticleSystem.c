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

	this->particles = NULL;
	this->expiredParticles = NULL;
	this->recyclableParticles = NULL;

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = false;
	this->spawnPositionDisplacement = (Vector3DFlag){false, false, false};
	this->spawnForceDelta = (Vector3DFlag){false, false, false};

	ParticleSystem::setup(this, particleSystemSpec);
}

/**
 * Class destructor
 */
void ParticleSystem::destructor()
{
	ParticleSystem::reset(this, false);

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Class set ParticleSystemSpec
 */
void ParticleSystem::setParticleSystemSpec(ParticleSystemSpec* particleSystemSpec, bool reset)
{
	if(reset)
	{
		ParticleSystem::reset(this, true);
		ParticleSystem::setup(this, particleSystemSpec);
	}
	else if(particleSystemSpec && particleSystemSpec != this->particleSystemSpec)
	{
		this->particleSystemSpec = particleSystemSpec;
		ParticleSystem::configure(this);
	}
}

/**
 * Class setup
 */
void ParticleSystem::setup(ParticleSystemSpec* particleSystemSpec)
{
	// save spec
	this->particleSystemSpec = particleSystemSpec;

	NM_ASSERT(this->particleSystemSpec, "ParticleSystem::setup: NULL spec");

	if(NULL == this->particleSystemSpec)
	{
		return;
	}

	this->particles = new VirtualList();
	this->expiredParticles = new VirtualList();
	this->recyclableParticles = this->particleSystemSpec->recycleParticles ? new VirtualList() : NULL;

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = !this->particleSystemSpec->autoStart;

	ParticleSystem::configure(this);
}

void ParticleSystem::configure()
{
	this->animationName = this->particleSystemSpec->particleSpec->initialAnimation;

	// set size from spec
	this->size.x += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.x - this->particleSystemSpec->minimumRelativeSpawnPosition.x);
	this->size.y += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.y - this->particleSystemSpec->minimumRelativeSpawnPosition.y);
	this->size.z += __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.z - this->particleSystemSpec->minimumRelativeSpawnPosition.z);

	this->spawnPositionDisplacement.x = this->particleSystemSpec->maximumRelativeSpawnPosition.x | this->particleSystemSpec->minimumRelativeSpawnPosition.x;
	this->spawnPositionDisplacement.y = this->particleSystemSpec->maximumRelativeSpawnPosition.y | this->particleSystemSpec->minimumRelativeSpawnPosition.y;
	this->spawnPositionDisplacement.z = this->particleSystemSpec->maximumRelativeSpawnPosition.z | this->particleSystemSpec->minimumRelativeSpawnPosition.z;

	this->spawnForceDelta.x = this->particleSystemSpec->maximumForce.x | this->particleSystemSpec->minimumForce.x;
	this->spawnForceDelta.y = this->particleSystemSpec->maximumForce.y | this->particleSystemSpec->minimumForce.y;
	this->spawnForceDelta.z = this->particleSystemSpec->maximumForce.z | this->particleSystemSpec->minimumForce.z;

	this->nextSpawnTime = this->paused ? 0 : ParticleSystem::computeNextSpawnTime(this);

	// calculate the number of sprite specs
	for(this->numberOfSpriteSpecs = 0; 0 <= this->numberOfSpriteSpecs && this->particleSystemSpec->spriteSpecs[this->numberOfSpriteSpecs]; this->numberOfSpriteSpecs++);

	ASSERT(0 < this->numberOfSpriteSpecs, "ParticleSystem::constructor: 0 sprite specs");
}

/**
 * Class reset
 */
void ParticleSystem::reset(bool deleteParticlesImmeditely)
{
	ParticleSystem::processExpiredParticles(this);

	ParticleRemover particleRemover = deleteParticlesImmeditely ? NULL : Stage::getParticleRemover(Game::getStage(Game::getInstance()));

	if(!isDeleted(this->particles))
	{
		// the remover handles all the cleaning
		if(!isDeleted(particleRemover))
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

	if(!isDeleted(this->recyclableParticles))
	{	
		// the remover handles all the cleaning
		if(!isDeleted(particleRemover))
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
	if(!isDeleted(this->expiredParticles) && this->expiredParticles->head)
	{
		VirtualNode node = this->expiredParticles->head;

		if(this->particleSystemSpec->recycleParticles)
		{
			for(; node; node = node->next)
			{
				Particle particle = Particle::safeCast(node->data);
				NM_ASSERT(!isDeleted(particle), "ParticleSystem::processExpiredParticles: deleted recyclable particle");
				VirtualList::pushBack(this->recyclableParticles, particle);
				VirtualList::removeElement(this->particles, particle);
				this->particleCount--;
			}
		}
		else
		{
			for(; node; node = node->next)
			{
				Particle particle = Particle::safeCast(node->data);
				VirtualList::removeElement(this->particles, particle);

				NM_ASSERT(!isDeleted(particle), "ParticleSystem::processExpiredParticles: deleted particle");

				delete particle;
				this->particleCount--;
			}
		}

		VirtualList::clear(this->expiredParticles);
	}
}

/**
 * @param elapsedTime
 */
void ParticleSystem::update(u32 elapsedTime)
{
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	if(this->hidden)
	{
		return;
	}

	Base::update(this, elapsedTime);

	ParticleSystem::processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	if(NULL == node && this->paused)
	{
		return;
	}

	void (* behavior)(Particle particle) = this->particleSystemSpec->particleSpec->behavior;

	for(; node; node = node->next)
	{
		if(Particle::update(node->data, elapsedTime, behavior))
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
			++this->totalSpawnedParticles;

			if(!this->loop && this->totalSpawnedParticles >= this->particleSystemSpec->maximumNumberOfAliveParticles)
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
		int lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + (this->particleSystemSpec->particleSpec->lifeSpanDelta ? Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta) : 0);

		// call the appropriate allocator to support inheritance
		Particle particle = Particle::safeCast(VirtualList::front(this->recyclableParticles));

		Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
		Force force = ParticleSystem::getParticleSpawnForce(this);

		Particle::setup(particle, lifeSpan, &position, &force, this->particleSystemSpec->movementType, this->animationName);

		VirtualList::popFront(this->recyclableParticles);

		return particle;
	}

	return ParticleSystem::spawnParticle(this);
}

/**
 * @private
 * @return		Spawn position
 */
Vector3D ParticleSystem::getParticleSpawnPosition()
{
	Vector3D position = this->transformation.globalPosition;

	if(this->spawnPositionDisplacement.x)
	{
		position.x += this->particleSystemSpec->minimumRelativeSpawnPosition.x + Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.x - this->particleSystemSpec->minimumRelativeSpawnPosition.x));
	}

	if(this->spawnPositionDisplacement.y)
	{
		position.y += this->particleSystemSpec->minimumRelativeSpawnPosition.y + Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.y - this->particleSystemSpec->minimumRelativeSpawnPosition.y));
	}

	if(this->spawnPositionDisplacement.z)
	{
		position.z += this->particleSystemSpec->minimumRelativeSpawnPosition.z + Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumRelativeSpawnPosition.z - this->particleSystemSpec->minimumRelativeSpawnPosition.z));
	}

	return position;
}

/**
 * @private
 * @return		Force
 */
Force ParticleSystem::getParticleSpawnForce()
{
	Force force = this->particleSystemSpec->minimumForce;

	if(this->spawnForceDelta.x)
	{
		force.x += Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumForce.x - this->particleSystemSpec->minimumForce.x));
	}

	if(this->spawnForceDelta.y)
	{
		force.y += Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumForce.y - this->particleSystemSpec->minimumForce.y));
	}

	if(this->spawnForceDelta.z)
	{
		force.z += Utilities::random(Utilities::randomSeed(), __ABS(this->particleSystemSpec->maximumForce.z - this->particleSystemSpec->minimumForce.z));
	}

	return force;
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
	int lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta);

	int spriteSpecIndex = 0;

	if(1 < this->numberOfSpriteSpecs)
	{
		spriteSpecIndex = Utilities::random(_gameRandomSeed, this->numberOfSpriteSpecs);
	}

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*, const SpriteSpec*, int)) this->particleSystemSpec->particleSpec->allocator)(this->particleSystemSpec->particleSpec, (const SpriteSpec*)this->particleSystemSpec->spriteSpecs[spriteSpecIndex], lifeSpan);
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Force force = ParticleSystem::getParticleSpawnForce(this);
	Particle::setPosition(particle, &position);
	Particle::addForce(particle, &force, this->particleSystemSpec->movementType);

	return particle;
}

/**
 * @param environmentTransform
 */
void ParticleSystem::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	if(this->hidden)
	{
		return;
	}

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
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	if(this->hidden)
	{
		return;
	}

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
		Particle::hide(Particle::safeCast(node->data), &this->transformation.globalPosition);
	}
}

void ParticleSystem::resume()
{
	// Must recover the particles first so their sprites are recreated
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
		Particle::hide(node->data, &this->transformation.globalPosition);
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	// Now call base
	Base::resume(this);
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
	
	Particle::hide(particle, &this->transformation.globalPosition);
}

/**
 * @private
 * @return		Time
 */
int ParticleSystem::computeNextSpawnTime()
{
	return this->particleSystemSpec->minimumSpawnDelay +
			(this->particleSystemSpec->spawnDelayDelta ? Utilities::random(_gameRandomSeed, this->particleSystemSpec->spawnDelayDelta) : 0);
}

void ParticleSystem::start()
{
	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
	this->totalSpawnedParticles = 0;
	this->paused = false;
	ParticleSystem::show(this);
}

void ParticleSystem::pause()
{
	this->paused = true;
}

void ParticleSystem::unpause()
{
	this->paused = false;
	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
}

bool ParticleSystem::isPaused()
{
	return this->paused && !this->particles->head;
}
