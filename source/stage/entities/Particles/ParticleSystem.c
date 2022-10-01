/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleSystem.h>
#include <VUEngine.h>
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
void ParticleSystem::constructor(const ParticleSystemSpec* particleSystemSpec, int16 internalId, const char* const name)
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
	this->previousGlobalPosition = (Vector3D){0, 0, 0};
	this->selfDestroyWhenDone = false;

	ParticleSystem::setup(this, particleSystemSpec);
}

/**
 * Class destructor
 */
void ParticleSystem::destructor()
{
	ParticleSystem::reset(this);

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
		ParticleSystem::reset(this);
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

	this->nextSpawnTime = 0;
	this->maximumNumberOfAliveParticles = this->particleSystemSpec->maximumNumberOfAliveParticles;

	// calculate the number of sprite specs
	for(this->numberOfSpriteSpecs = 0; 0 <= this->numberOfSpriteSpecs && NULL != this->particleSystemSpec->spriteSpecs && NULL != this->particleSystemSpec->spriteSpecs[this->numberOfSpriteSpecs]; this->numberOfSpriteSpecs++);
	// calculate the number of wireframe specs
	for(this->numberOfWireframeSpecs = 0; 0 <= this->numberOfWireframeSpecs && NULL != this->particleSystemSpec->wireframeSpecs && NULL != this->particleSystemSpec->wireframeSpecs[this->numberOfWireframeSpecs]; this->numberOfWireframeSpecs++);
}

/**
 * Class reset
 */
void ParticleSystem::reset()
{
	ParticleSystem::processExpiredParticles(this);

	if(!isDeleted(this->particles))
	{
		VirtualList::deleteData(this->particles);
		delete this->particles;
		this->particles = NULL;
	}
}

void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}

void ParticleSystem::deleteAllParticles()
{
	if(!isDeleted(this->particles))
	{
		for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
		{
			Particle particle = Particle::safeCast(node->data);

			NM_ASSERT(!isDeleted(particle), "ParticleSystem::expireAllParticles: deleted particle");

			delete particle;
		}

		VirtualList::clear(this->particles);
	}

	this->particleCount = 0;
}

void ParticleSystem::expireAllParticles()
{
	ParticleSystem::processExpiredParticles(this);

	if(!isDeleted(this->particles))
	{
		for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
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
		VirtualList particles = this->particles;
		this->particles = NULL;

		for(VirtualNode node = particles->head, nextNode; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Particle particle = Particle::safeCast(node->data);

			if(particle->expired)
			{
				VirtualList::removeNode(particles, node);

				NM_ASSERT(!isDeleted(particle), "ParticleSystem::processExpiredParticles: deleted particle");

				delete particle;
				this->particleCount--;
			}
		}

		this->particles = particles;
	}
}

/**
 * @param elapsedTime
 */
void ParticleSystem::update(uint32 elapsedTime)
{
	if(ParticleSystem::isPaused(this) || this->invalidateGlobalTransformation)
	{
		return;
	}

	Base::update(this, elapsedTime);

	if(isDeleted(this->particles))
	{
		return;
	}

	ParticleSystem::processExpiredParticles(this);

	VirtualNode node = this->particles->head;

	if(NULL == node && this->paused)
	{
		this->update = ParticleSystem::overrides(this, update);
		return;
	}

	void (* behavior)(Particle particle) = this->particleSystemSpec->particleSpec->behavior;

	for(; NULL != node; node = node->next)
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

	if(this->selfDestroyWhenDone && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles && 0 == this->particleCount && !this->loop)
	{
		ParticleSystem::deleteMyself(this);
	}

	if(!this->transformed || this->paused || this->hidden)
	{
		return;
	}

	// check if it is time to spawn new particles
	this->nextSpawnTime -= elapsedTime;

	if(0 > this->nextSpawnTime && this->particleCount < this->maximumNumberOfAliveParticles)
	{
		uint16 spawnedParticles = 0;
		do 
		{
			if(!this->loop && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles)
			{
				ParticleSystem::pause(this);
				return;
			}

			++this->totalSpawnedParticles;

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
	if(isDeleted(this->particles))
	{
		return false;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
			Force force = ParticleSystem::getParticleSpawnForce(this);
			int16 lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + (this->particleSystemSpec->particleSpec->lifeSpanDelta ? Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta) : 0);

			Particle::setup(particle, lifeSpan, &position, &force, this->particleSystemSpec->movementType, this->particleSystemSpec->particleSpec->animationFunctions, this->particleSystemSpec->particleSpec->initialAnimation, this->animationChanged);

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
	if(this->particleSystemSpec->useMovementVector)
	{
		Vector3D direction = Vector3D::normalize(Vector3D::get(this->previousGlobalPosition, this->transformation.globalPosition));
//		fixed_t strength = (Vector3D::length(this->particleSystemSpec->minimumForce) + Vector3D::length(this->particleSystemSpec->maximumForce)) >> 1;
		fixed_t strength = Vector3D::length(this->particleSystemSpec->minimumForce);
		return Vector3D::scalarProduct(direction, strength);
	}

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
	VirtualList particles = this->particles;
	this->particles = NULL;

	while(this->particleCount < this->maximumNumberOfAliveParticles)
	{
		Particle particle = ParticleSystem::spawnParticle(this);
		Particle::hide(particle);
		VirtualList::pushBack(particles, particle);
		this->particleCount++;
	}

	this->particles = particles;
}

const SpriteSpec* ParticleSystem::getSpriteSpec()
{
	if(0 == this->numberOfSpriteSpecs)
	{
		return NULL;
	}

	int32 specIndex = 0;

	if(1 < this->numberOfSpriteSpecs)
	{
		specIndex = Utilities::random(_gameRandomSeed, this->numberOfSpriteSpecs);
	}

	return (const SpriteSpec*)this->particleSystemSpec->spriteSpecs[specIndex];
}

const WireframeSpec* ParticleSystem::getWireframeSpec()
{
	if(0 == this->numberOfWireframeSpecs)
	{
		return NULL;
	}

	int32 specIndex = 0;

	if(1 < this->numberOfWireframeSpecs)
	{
		specIndex = Utilities::random(_gameRandomSeed, this->numberOfWireframeSpecs);
	}

	return (const WireframeSpec*)this->particleSystemSpec->wireframeSpecs[specIndex];
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
	int16 lifeSpan = this->particleSystemSpec->particleSpec->minimumLifeSpan + Utilities::random(_gameRandomSeed, this->particleSystemSpec->particleSpec->lifeSpanDelta);

	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*, const SpriteSpec*, const WireframeSpec*, int32)) this->particleSystemSpec->particleSpec->allocator)(this->particleSystemSpec->particleSpec, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), lifeSpan);
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Force force = ParticleSystem::getParticleSpawnForce(this);
	Particle::setPosition(particle, &position);
	Particle::applySustainedForce(particle, &force, this->particleSystemSpec->movementType);

	ParticleSystem::particleSpawned(this, particle);

	return particle;
}

/**
 * @param environmentTransform
 */
void ParticleSystem::transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag)
{
	this->previousGlobalPosition = this->transformation.globalPosition;

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
	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
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
	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
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
	NM_ASSERT(__GET_CAST(ParticleSystem, this), "ParticleSystem::synchronizeGraphics: not a particle system");

	if(ParticleSystem::isPaused(this) || isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		NM_ASSERT(!isDeleted(particle), "ParticleSystem::synchronizeGraphics: deleted particle");

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

	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
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

	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle::hide(Particle::safeCast(node->data));
	}
}

void ParticleSystem::resume()
{
	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		Particle::resume(particle, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), this->particleSystemSpec->particleSpec->animationFunctions, this->particleSystemSpec->particleSpec->initialAnimation);
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	// Now call base
	Base::resume(this);
}

void ParticleSystem::suspend()
{
	Base::suspend(this);

	ParticleSystem::processExpiredParticles(this);

	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle::suspend(node->data);
	}
}

/**
 * @private
 * @return		Time
 */
int32 ParticleSystem::computeNextSpawnTime()
{
	return this->particleSystemSpec->minimumSpawnDelay +
			(this->particleSystemSpec->spawnDelayDelta ? Utilities::random(_gameRandomSeed, this->particleSystemSpec->spawnDelayDelta) : 0);
}

/**
 * @public
 * @param maximumNumberOfAliveParticles		Maximum number of particles alive
 */
void ParticleSystem::setMaximumNumberOfAliveParticles(uint8 maximumNumberOfAliveParticles)
{
	this->maximumNumberOfAliveParticles = maximumNumberOfAliveParticles;
}

/**
 * @public
 * @param selfDestroyWhenDone		Set to true to destroy the particle system when all the particles have expired
 */
void ParticleSystem::setSelfDestroyWhenDone(bool selfDestroyWhenDone)
{
	this->selfDestroyWhenDone = selfDestroyWhenDone;
}

void ParticleSystem::start()
{
	this->update = true;
	this->nextSpawnTime = 0;
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
		this->nextSpawnTime = 0;
	}

	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;
}

bool ParticleSystem::isPaused()
{
	return this->paused && 0 == this->particleCount;
}
