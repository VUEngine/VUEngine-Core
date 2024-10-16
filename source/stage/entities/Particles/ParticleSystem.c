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

#include <Particle.h>
#include <Printing.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VUEngine.h>

#include "ParticleSystem.h"


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
	this->elapsedTime = __MILLISECONDS_PER_SECOND / __TARGET_FPS;
	this->transformed = false;

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
	else if(NULL != particleSystemSpec && particleSystemSpec != (ParticleSystemSpec*)this->entitySpec)
	{
		this->animationChanged = ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation != particleSystemSpec->particleSpec->initialAnimation;
		ParticleSystem::setSpec(this, (EntitySpec*)particleSystemSpec);
		ParticleSystem::configure(this);
	}
}

/**
 * Class setup
 */
void ParticleSystem::setup(const ParticleSystemSpec* particleSystemSpec)
{
	this->animationChanged = NULL == this->entitySpec || ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation != particleSystemSpec->particleSpec->initialAnimation;

	// save spec
	ParticleSystem::setSpec(this, (EntitySpec*)particleSystemSpec);

	NM_ASSERT(this->entitySpec, "ParticleSystem::setup: NULL spec");

	if(NULL == this->entitySpec)
	{
		return;
	}

	this->particles = new VirtualList();

	this->particleCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = !((ParticleSystemSpec*)this->entitySpec)->autoStart;
	this->maximumNumberOfAliveParticles = 0;

	ParticleSystem::configure(this);

	this->update = ((ParticleSystemSpec*)this->entitySpec)->autoStart;
	this->applyForceToParticles = ParticleSystem::appliesForceToParticles(this);
}

void ParticleSystem::configure()
{
	// set size from spec
	this->size.x += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.x - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x);
	this->size.y += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.y - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y);
	this->size.z += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.z - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z);

	this->spawnPositionDisplacement.x = ((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.x | ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x;
	this->spawnPositionDisplacement.y = ((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.y | ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y;
	this->spawnPositionDisplacement.z = ((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.z | ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z;

	this->spawnForceDelta.x = ((ParticleSystemSpec*)this->entitySpec)->maximumForce.x | ((ParticleSystemSpec*)this->entitySpec)->minimumForce.x;
	this->spawnForceDelta.y = ((ParticleSystemSpec*)this->entitySpec)->maximumForce.y | ((ParticleSystemSpec*)this->entitySpec)->minimumForce.y;
	this->spawnForceDelta.z = ((ParticleSystemSpec*)this->entitySpec)->maximumForce.z | ((ParticleSystemSpec*)this->entitySpec)->minimumForce.z;

	this->nextSpawnTime = 0;
	this->maximumNumberOfAliveParticles = ((ParticleSystemSpec*)this->entitySpec)->maximumNumberOfAliveParticles;

	// calculate the number of sprite specs
	for(this->numberOfSpriteSpecs = 0; 0 <= this->numberOfSpriteSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->spriteSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->spriteSpecs[this->numberOfSpriteSpecs]; this->numberOfSpriteSpecs++);
	// calculate the number of wireframe specs
	for(this->numberOfWireframeSpecs = 0; 0 <= this->numberOfWireframeSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->wireframeSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->wireframeSpecs[this->numberOfWireframeSpecs]; this->numberOfWireframeSpecs++);
}

/**
 * Class reset
 */
void ParticleSystem::reset()
{
	if(!isDeleted(this->particles))
	{
		VirtualList::deleteData(this->particles);
		delete this->particles;
		this->particles = NULL;
	}

	this->particleCount	= 0;
}

void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}

void ParticleSystem::deleteAllParticles()
{
	if(!isDeleted(this->particles))
	{
		VirtualList::deleteData(this->particles);
	}

	this->particleCount = 0;
}

void ParticleSystem::expireAllParticles()
{
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
	if(!((ParticleSystemSpec*)this->entitySpec)->recycleParticles)
	{
		if(!isDeleted(this->particles))
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
}

/**
 * @public
 */
void ParticleSystem::update()
{
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	if(NULL != this->children && NULL != this->behaviors)
	{
		Base::update(this);
	}

	if(isDeleted(this->particles))
	{
		return;
	}

	ParticleSystem::processExpiredParticles(this);

	if(this->transformation.invalid && !this->transformed)
	{
		return;
	}

	VirtualNode node = this->particles->head;

	if(NULL == node && this->paused)
	{
		this->update = ParticleSystem::overrides(this, update);
		return;
	}

	void (* behavior)(Particle particle) = ((ParticleSystemSpec*)this->entitySpec)->particleSpec->behavior;

	for(; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		if(Particle::update(particle, this->elapsedTime, behavior))
		{
			this->particleCount--;
		}

		NM_ASSERT(0 <= this->particleCount, "ParticleSystem::update: negative particle count");
	}

	if(this->selfDestroyWhenDone && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles && 0 == this->particleCount && !this->loop)
	{
		ParticleSystem::deleteMyself(this);
		return;
	}

	if(!this->transformed || this->paused || this->hidden)
	{
		return;
	}

	// check if it is time to spawn new particles
	this->nextSpawnTime -= this->elapsedTime;

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

			if(!((ParticleSystemSpec*)this->entitySpec)->recycleParticles)
			{
				VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
				this->particleCount++;
			}
			else
			{
				if(!ParticleSystem::recycleParticle(this))
				{
					VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
					this->particleCount++;
				}
				else
				{
					this->particleCount++;
				}
			}

			this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
		}
		while(++spawnedParticles < ((ParticleSystemSpec*)this->entitySpec)->maximumNumberOfParticlesToSpawnPerCycle && 0 == ((ParticleSystemSpec*)this->entitySpec)->minimumSpawnDelay && this->particleCount < this->maximumNumberOfAliveParticles);
	}
}

/**
 * @private
 * @return		Boolean
 */
bool ParticleSystem::recycleParticle()
{
	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
			int16 lifeSpan = ((ParticleSystemSpec*)this->entitySpec)->particleSpec->minimumLifeSpan + (0 != ((ParticleSystemSpec*)this->entitySpec)->particleSpec->lifeSpanDelta ? Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->lifeSpanDelta) : 0);

			if(this->applyForceToParticles)
			{
				Vector3D force = ParticleSystem::getParticleSpawnForce(this);
				Particle::setup(particle, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), lifeSpan, &position, &force, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation, this->animationChanged);
			}
			else
			{
				Particle::setup(particle, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), lifeSpan, &position, NULL, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation, this->animationChanged);
			}

			if(ParticleSystem::overrides(this, particleRecycled))
			{
				ParticleSystem::particleRecycled(this, particle);
			}

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
	Vector3D position = this->transformation.position;

	if(0 != this->spawnPositionDisplacement.x)
	{
		position.x += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x + Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.x - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x));
	}

	if(0 != this->spawnPositionDisplacement.y)
	{
		position.y += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y + Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.y - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y));
	}

	if(0 != this->spawnPositionDisplacement.z)
	{
		position.z += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z + Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.z - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z));
	}

	return position;
}

/**
 * @private
 * @return		Vector3D
 */
Vector3D ParticleSystem::getParticleSpawnForce()
{
	if(((ParticleSystemSpec*)this->entitySpec)->useMovementVector)
	{
		Vector3D direction = Vector3D::normalize(Vector3D::get(this->previousGlobalPosition, this->transformation.position));
		fixed_t strength = (Vector3D::length(((ParticleSystemSpec*)this->entitySpec)->minimumForce) + Vector3D::length(((ParticleSystemSpec*)this->entitySpec)->maximumForce)) >> 1;
		return Vector3D::scalarProduct(direction, strength);
	}

	Vector3D force = ((ParticleSystemSpec*)this->entitySpec)->minimumForce;

	if(0 != this->spawnForceDelta.x)
	{
		force.x += Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumForce.x - ((ParticleSystemSpec*)this->entitySpec)->minimumForce.x));
	}

	if(0 != this->spawnForceDelta.y)
	{
		force.y += Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumForce.y - ((ParticleSystemSpec*)this->entitySpec)->minimumForce.y));
	}

	if(0 != this->spawnForceDelta.z)
	{
		force.z += Math::random(Math::randomSeed(), __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumForce.z - ((ParticleSystemSpec*)this->entitySpec)->minimumForce.z));
	}

	return force;
}

/**
 * @private
 * @return		Bool
 */
bool ParticleSystem::appliesForceToParticles()
{
	if(((ParticleSystemSpec*)this->entitySpec)->useMovementVector)
	{
		return true;
	}

	if
	(
		0 != ((ParticleSystemSpec*)this->entitySpec)->minimumForce.x ||
		0 != ((ParticleSystemSpec*)this->entitySpec)->minimumForce.y ||
		0 != ((ParticleSystemSpec*)this->entitySpec)->minimumForce.z
	)
	{
		return true;
	}

	if
	(
		0 != ((ParticleSystemSpec*)this->entitySpec)->maximumForce.x ||
		0 != ((ParticleSystemSpec*)this->entitySpec)->maximumForce.y ||
		0 != ((ParticleSystemSpec*)this->entitySpec)->maximumForce.z
	)
	{
		return true;
	}

	return false;
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
		specIndex = Math::random(_gameRandomSeed, this->numberOfSpriteSpecs);
	}

	return (const SpriteSpec*)((ParticleSystemSpec*)this->entitySpec)->spriteSpecs[specIndex];
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
		specIndex = Math::random(_gameRandomSeed, this->numberOfWireframeSpecs);
	}

	return (const WireframeSpec*)((ParticleSystemSpec*)this->entitySpec)->wireframeSpecs[specIndex];
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
	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*, ParticleSystem)) ((ParticleSystemSpec*)this->entitySpec)->particleSpec->allocator)(((ParticleSystemSpec*)this->entitySpec)->particleSpec, this);

	int16 lifeSpan = ((ParticleSystemSpec*)this->entitySpec)->particleSpec->minimumLifeSpan + Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->lifeSpanDelta);
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Vector3D force = Vector3D::zero();

	if(this->applyForceToParticles)
	{
		force = ParticleSystem::getParticleSpawnForce(this);
	}

	Particle::setup(particle, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), lifeSpan, &position, &force, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation, this->animationChanged);

	if(ParticleSystem::overrides(this, particleSpawned))
	{
		ParticleSystem::particleSpawned(this, particle);
	}

	return particle;
}

/**
 * @param environmentTransform
 */
void ParticleSystem::transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag)
{
	this->previousGlobalPosition = this->transformation.position;

	bool transformed = this->transformed;

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	this->transformed = true;

	if(!transformed)
	{
		ParticleSystem::resetParticlesPositions(this);
	}
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

void ParticleSystem::setTransparent(uint8 transparent)
{
	Base::setTransparent(this, transparent);

	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle::setTransparent(Particle::safeCast(node->data), transparent);
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

		Particle::resume(particle, ParticleSystem::getSpriteSpec(this), ParticleSystem::getWireframeSpec(this), ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation);
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
	return ((ParticleSystemSpec*)this->entitySpec)->minimumSpawnDelay +
			(((ParticleSystemSpec*)this->entitySpec)->spawnDelayDelta ? Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->entitySpec)->spawnDelayDelta) : 0);
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

/**
 * @public
 * @param elapsedTime		Elapsed time per tick
 */
void ParticleSystem::setElapsedTime(uint32 elapsedTime)
{
	this->elapsedTime = elapsedTime;
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

	this->transformation.invalid |= __INVALIDATE_POSITION;
}

bool ParticleSystem::isPaused()
{
	return this->paused && 0 == this->particleCount;
}

const AnimationFunction** ParticleSystem::getAnimationFunctions()
{
	return ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions;
}

void ParticleSystem::print(int16 x, int16 y)
{
	Printing::text(Printing::getInstance(), "PARTICLE SYSTEM ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Particles", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Maximum:    ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->maximumNumberOfAliveParticles, x + 10, y, NULL);
	Printing::text(Printing::getInstance(), "Spawned:    ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->particles), x + 10, y, NULL);
	Printing::text(Printing::getInstance(), "Alive:      ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->particleCount, x + 10, y, NULL);
}