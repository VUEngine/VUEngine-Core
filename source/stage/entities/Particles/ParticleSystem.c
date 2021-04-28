/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

friend class Particle;
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
void ParticleSystem::constructor(const ParticleSystemSpec* particleSystemSpec, s16 internalId, const char* const name)
{
	// construct base
	Base::constructor((EntitySpec*)&particleSystemSpec->entitySpec, internalId, name);

	this->invalidateGraphics = __INVALIDATE_TRANSFORMATION;

	this->particles = NULL;

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = false;
	this->spawnPositionDisplacement = (Vector3DFlag){false, false, false};
	this->spawnForceDelta = (Vector3DFlag){false, false, false};
	this->maximumNumberOfAliveParticles = 0;
	this->animationChanged = true;

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
		this->animationChanged = this->particleSystemSpec->particleSpec->initialAnimation != particleSystemSpec->particleSpec->initialAnimation;
		this->particleSystemSpec = particleSystemSpec;
		ParticleSystem::configure(this);
	}
}

/**
 * Class setup
 */
void ParticleSystem::setup(const ParticleSystemSpec* particleSystemSpec)
{
	this->animationChanged = NULL == this->particleSystemSpec || this->particleSystemSpec->particleSpec->initialAnimation != particleSystemSpec->particleSpec->initialAnimation;

	// save spec
	this->particleSystemSpec = particleSystemSpec;

	NM_ASSERT(this->particleSystemSpec, "ParticleSystem::setup: NULL spec");

	if(NULL == this->particleSystemSpec)
	{
		return;
	}

	this->particles = new VirtualList();

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = !this->particleSystemSpec->autoStart;
	this->maximumNumberOfAliveParticles = 0;

	ParticleSystem::configure(this);

	this->update = this->particleSystemSpec->autoStart;
}

void ParticleSystem::configure()
{
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
	this->maximumNumberOfAliveParticles = this->particleSystemSpec->maximumNumberOfAliveParticles;

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
}

void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}

void ParticleSystem::deleteAllParticles()
{
	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		NM_ASSERT(!isDeleted(particle), "ParticleSystem::expireAllParticles: deleted particle");

		delete particle;
	}

	VirtualList::clear(this->particles);
	this->particleCount = 0;
}

void ParticleSystem::expireAllParticles()
{
	ParticleSystem::processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		Particle::expire(particle);
		this->particleCount--;

		NM_ASSERT(0 <= this->particleCount, "ParticleSystem::update: negative particle count");
	}

	ParticleSystem::processExpiredParticles(this);
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
	if(!isDeleted(this->particles) && !this->particleSystemSpec->recycleParticles)
	{
		VirtualList expiredParticles = new VirtualList();

		VirtualNode node = this->particles->head;

		for(; node; node = node->next)
		{
			Particle particle = Particle::safeCast(node->data);

			if(particle->expired)
			{
				VirtualList::pushBack(expiredParticles, particle);
			}
		}

		node = expiredParticles->head;

		for(; node; node = node->next)
		{
			Particle particle = Particle::safeCast(node->data);
			VirtualList::removeElement(this->particles, particle);

			NM_ASSERT(!isDeleted(particle), "ParticleSystem::processExpiredParticles: deleted particle");

			delete particle;
			this->particleCount--;
		}

		delete expiredParticles;
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

	Base::update(this, elapsedTime);

	ParticleSystem::processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	if(NULL == node && this->paused)
	{
		this->update = ParticleSystem::overrides(this, update);
		return;
	}

	void (* behavior)(Particle particle) = this->particleSystemSpec->particleSpec->behavior;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		if(Particle::update(particle, elapsedTime, behavior))
		{
			Particle::expire(particle);
			this->particleCount--;
		}

		NM_ASSERT(0 <= this->particleCount, "ParticleSystem::update: negative particle count");
	}

	if(!this->transformed)
	{
		return;
	}

	if(this->paused)
	{
		return;
	}

	// check if it is time to spawn new particles
	this->nextSpawnTime -= elapsedTime;

	if(0 > this->nextSpawnTime && this->particleCount < this->maximumNumberOfAliveParticles)
	{
		u16 spawnedParticles = 0;
		do 
		{
			++this->totalSpawnedParticles;

			if(!this->loop && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles)
			{
				ParticleSystem::pause(this);
				return;
			}

			if(this->particleSystemSpec->recycleParticles)
			{
				if(!ParticleSystem::recycleParticle(this))
				{
					VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
				}
			}
			else
			{
				VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
			}

			this->particleCount++;
			this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
		}
		while(++spawnedParticles < this->particleSystemSpec->maximumNumberOfParticlesToSpawnPerCycle && 0 == this->particleSystemSpec->minimumSpawnDelay && this->particleCount < this->maximumNumberOfAliveParticles);
	}
}

/**
 * @private
 * @return		Boolean
 */
bool ParticleSystem::recycleParticle()
{
	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
			Force force = ParticleSystem::getParticleSpawnForce(this);
			s16 lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + (this->particleSystemSpec->particleSpec->lifeSpanDelta ? Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta) : 0);

			Particle::setup(particle, lifeSpan, &position, &force, this->particleSystemSpec->movementType, this->particleSystemSpec->particleSpec->animationDescription, this->particleSystemSpec->particleSpec->initialAnimation, this->animationChanged);

			ParticleSystem::particleRecycled(this, particle);

			return true;
		}
	}

	return false;
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
	while(this->particleCount < this->maximumNumberOfAliveParticles)
	{
		Particle particle = ParticleSystem::spawnParticle(this);
		Particle::hide(particle);
		VirtualList::pushBack(this->particles, particle);
		this->particleCount++;
	}
}

const SpriteSpec* ParticleSystem::getSpriteSpec()
{
	if(NULL == this->particleSystemSpec || NULL == this->particleSystemSpec->spriteSpecs[0])
	{
		return NULL;
	}

	int spriteSpecIndex = 0;

	if(1 < this->numberOfSpriteSpecs)
	{
		spriteSpecIndex = Utilities::random(_gameRandomSeed, this->numberOfSpriteSpecs);
	}

	return (const SpriteSpec*)this->particleSystemSpec->spriteSpecs[spriteSpecIndex];
}

void ParticleSystem::particleSpawned(Particle particle __attribute__ ((unused)))
{
}

void ParticleSystem::particleRecycled(Particle particle __attribute__ ((unused)))
{
}

/**
 * Spawn a particle
 *
 * @private
 */
Particle ParticleSystem::spawnParticle()
{
	s16 lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta);

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*, const SpriteSpec*, int)) this->particleSystemSpec->particleSpec->allocator)(this->particleSystemSpec->particleSpec, ParticleSystem::getSpriteSpec(this), lifeSpan);
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Force force = ParticleSystem::getParticleSpawnForce(this);
	Particle::setPosition(particle, &position);
	Particle::addForce(particle, &force, this->particleSystemSpec->movementType);

	ParticleSystem::particleSpawned(this, particle);

	return particle;
}

/**
 * @param environmentTransform
 */
void ParticleSystem::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	this->invalidateGraphics = __INVALIDATE_TRANSFORMATION;

	bool transformed = this->transformed;

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	if(!transformed)
	{
		ParticleSystem::resetParticlesPositions(this);
	}

	ParticleSystem::transformParticles(this);
}

void ParticleSystem::resetParticlesPositions()
{
	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
		Particle::setPosition(particle, &position);
	}
}

void ParticleSystem::transformParticles()
{
	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		Particle::transform(particle);
	}
}

void ParticleSystem::synchronizeGraphics()
{
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		Particle::synchronizeGraphics(particle);
	}
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
	if(!this->hidden)
	{
		return;
	}

	Base::show(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		Particle::show(particle);
	}
}

void ParticleSystem::hide()
{
	if(this->hidden)
	{
		return;
	}

	Base::hide(this);

	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle::hide(Particle::safeCast(node->data));
	}
}

void ParticleSystem::resume()
{
	// Must recover the particles first so their sprites are recreated
	VirtualNode node = this->particles->head;

	for(; node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		Particle::resume(particle, ParticleSystem::getSpriteSpec(this), this->particleSystemSpec->particleSpec->animationDescription, this->particleSystemSpec->particleSpec->initialAnimation);
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

/**
 * @public
 * @param maximumNumberOfAliveParticles		Maximum number of particles alive
 */
void ParticleSystem::setMaximumNumberOfAliveParticles(u8 maximumNumberOfAliveParticles)
{
	this->maximumNumberOfAliveParticles = maximumNumberOfAliveParticles;
}

void ParticleSystem::start()
{
	this->update = true;
	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
	this->totalSpawnedParticles = 0;
	this->paused = false;
	this->transformed = false;
	ParticleSystem::show(this);
}

void ParticleSystem::pause()
{
	this->paused = true;
}

void ParticleSystem::unpause()
{
	this->update = true;

	if(this->paused)
	{
		this->paused = false;
		this->transformed = false;
		this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
	}
}

bool ParticleSystem::isPaused()
{
	return this->paused && 0 == this->particleCount;
}
