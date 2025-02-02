/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Particle.h>
#include <Printer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VUEngine.h>

#include "ParticleSystem.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Particle;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::constructor(const ParticleSystemSpec* particleSystemSpec, int16 internalId, const char* const name)
{
	// Construct base
	// Always explicitly call the base's constructor 
	Base::constructor((ActorSpec*)&particleSystemSpec->actorSpec, internalId, name);

	this->particles = NULL;
	this->aliveParticlesCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = false;
	this->spawnPositionDisplacement = Vector3D::zero();
	this->spawnForceDelta = Vector3D::zero();
	this->maximumNumberOfAliveParticles = 0;
	this->selfDestroyWhenDone = false;
	this->elapsedTime = __MILLISECONDS_PER_SECOND / __TARGET_FPS;

	ParticleSystem::setup(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::destructor()
{
	ParticleSystem::deleteAllParticles(this);

	if(!isDeleted(this->particles))
	{
		delete this->particles;
		this->particles = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::update()
{
	if(ParticleSystem::isPaused(this))
	{
		return;
	}

	if(NULL != this->children)
	{
		Base::update(this);
	}

	if(isDeleted(this->particles))
	{
		return;
	}

	ParticleSystem::processExpiredParticles(this);

	if(__VALID_TRANSFORMATION != this->transformation.invalid)
	{
		return;
	}

	VirtualNode node = this->particles->head;

	if(NULL == node && this->paused)
	{
		this->update = ParticleSystem::overrides(this, update);
		return;
	}

	for(; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			continue;
		}

		if(Particle::update(particle, this->elapsedTime))
		{
			this->aliveParticlesCount--;
		}

		NM_ASSERT(0 <= this->aliveParticlesCount, "ParticleSystem::update: negative particle count");
	}

	if
	(
		this->selfDestroyWhenDone && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles 
		&& 
		0 == this->aliveParticlesCount && !this->loop
	)
	{
		ParticleSystem::deleteMyself(this);
		return;
	}

	if(this->paused || this->hidden)
	{
		return;
	}

	// Check if it is time to spawn new particles
	this->nextSpawnTime -= this->elapsedTime;

	if(0 > this->nextSpawnTime && this->aliveParticlesCount < this->maximumNumberOfAliveParticles)
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

			if(!((ParticleSystemSpec*)this->actorSpec)->recycleParticles)
			{
				VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
				this->aliveParticlesCount++;
			}
			else
			{
				if(!ParticleSystem::recycleParticle(this))
				{
					VirtualList::pushBack(this->particles, ParticleSystem::spawnParticle(this));
					this->aliveParticlesCount++;
				}
				else
				{
					this->aliveParticlesCount++;
				}
			}

			this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);
		}
		while
		(
			++spawnedParticles < ((ParticleSystemSpec*)this->actorSpec)->maximumNumberOfParticlesToSpawnPerCycle 
			&& 
			0 == ((ParticleSystemSpec*)this->actorSpec)->minimumSpawnDelay 
			&& 
			this->aliveParticlesCount < this->maximumNumberOfAliveParticles
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::resume()
{
	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		Particle::resume(particle, ParticleSystem::getVisualComponentSpec(this));
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	// Now call base
	Base::resume(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setTransparency(uint8 transparency)
{
	Base::setTransparency(this, transparency);

	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle::setTransparency(Particle::safeCast(node->data), transparency);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setSpec(void* particleSystemSpec)
{
	if(NULL == particleSystemSpec)
	{
		return;
	}

	Base::setSpec(this, particleSystemSpec);
	ParticleSystem::setup(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::start()
{
	this->update = true;
	this->nextSpawnTime = 0;
	this->totalSpawnedParticles = 0;
	this->paused = false;
	ParticleSystem::show(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::pause()
{
	this->paused = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::unpause()
{
	this->update = true;

	if(this->paused)
	{
		this->paused = false;
		this->nextSpawnTime = 0;
	}

	this->transformation.invalid |= __INVALIDATE_POSITION;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ParticleSystem::isPaused()
{
	return this->paused && 0 == this->aliveParticlesCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::deleteAllParticles()
{
	if(!isDeleted(this->particles))
	{
		VirtualList::deleteData(this->particles);
	}

	this->aliveParticlesCount = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ParticleSystem::getLoop()
{
	return this->loop;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setSelfDestroyWhenDone(bool selfDestroyWhenDone)
{
	this->selfDestroyWhenDone = selfDestroyWhenDone;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setElapsedTime(uint32 elapsedTime)
{
	this->elapsedTime = elapsedTime;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::print(int16 x, int16 y)
{
	Printer::text("PARTICLE SYSTEM ", x, y++, NULL);
	Printer::text("Particles", x, ++y, NULL);
	Printer::text("Maximum:    ", x + 1, ++y, NULL);
	Printer::int32(this->maximumNumberOfAliveParticles, x + 10, y, NULL);
	Printer::text("Spawned:    ", x + 1, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->particles), x + 10, y, NULL);
	Printer::text("Alive:      ", x + 1, ++y, NULL);
	Printer::int32(this->aliveParticlesCount, x + 10, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::particleSpawned(Particle particle __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::particleRecycled(Particle particle __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::setup()
{
	NM_ASSERT(this->actorSpec, "ParticleSystem::setup: NULL spec");

	if(NULL == this->actorSpec)
	{
		return;
	}

	ParticleSystem::deleteAllParticles(this);

	if(isDeleted(this->particles))
	{
		this->particles = new VirtualList();
	}

	this->aliveParticlesCount = 0;
	this->totalSpawnedParticles = 0;
	this->loop = true;
	this->paused = !((ParticleSystemSpec*)this->actorSpec)->autoStart;
	this->maximumNumberOfAliveParticles = 0;

	ParticleSystem::configure(this);

	this->update = ((ParticleSystemSpec*)this->actorSpec)->autoStart;
	this->applyForceToParticles = ParticleSystem::appliesForceToParticles(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::configure()
{
	this->size.x += 
		__ABS(((ParticleSystemSpec*)this->actorSpec)->maximumRelativeSpawnPosition.x - 
		((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.x);
	this->size.y += 
		__ABS(((ParticleSystemSpec*)this->actorSpec)->maximumRelativeSpawnPosition.y - 
		((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.y);
	this->size.z += 
		__ABS(((ParticleSystemSpec*)this->actorSpec)->maximumRelativeSpawnPosition.z - 
		((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.z);

	this->spawnPositionDisplacement = 
		Vector3D::absolute
		(
			Vector3D::sub
			(
				((ParticleSystemSpec*)this->actorSpec)->maximumRelativeSpawnPosition, 
				((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition
			)
		);

	this->spawnForceDelta = 
		Vector3D::absolute
		(
			Vector3D::sub(((ParticleSystemSpec*)this->actorSpec)->maximumForce, ((ParticleSystemSpec*)this->actorSpec)->minimumForce)
		);

	this->nextSpawnTime = 0;
	this->maximumNumberOfAliveParticles = ((ParticleSystemSpec*)this->actorSpec)->maximumNumberOfAliveParticles;

	// Calculate the number of sprite specs
	for
	(
		this->numberOfVisualComponentSpecs = 0; 
		0 <= this->numberOfVisualComponentSpecs 
		&& 
		NULL != ((ParticleSystemSpec*)this->actorSpec)->visualComponentSpecs 
		&& 
		NULL != ((ParticleSystemSpec*)this->actorSpec)->visualComponentSpecs[this->numberOfVisualComponentSpecs]; 
		this->numberOfVisualComponentSpecs++
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Particle ParticleSystem::spawnParticle()
{
	// Call the appropriate allocator to support inheritance
	Particle particle = 
		((Particle (*)
		(
			const ParticleSpec*)) ((ParticleSystemSpec*)this->actorSpec)->particleSpec->allocator)
			(((ParticleSystemSpec*)this->actorSpec)->particleSpec
		);

	int16 lifeSpan = 
		((ParticleSystemSpec*)this->actorSpec)->particleSpec->minimumLifeSpan + 
		Math::random
		(
			_gameRandomSeed, ((ParticleSystemSpec*)this->actorSpec)->particleSpec->lifeSpanDelta
		);
	
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Vector3D force = Vector3D::zero();

	if(this->applyForceToParticles)
	{
		force = ParticleSystem::getParticleSpawnForce(this);
	}

	Particle::setup
	(
		particle, 
		ParticleSystem::getVisualComponentSpec(this), 
		ParticleSystem::getPhysicsComponentSpec(this), 
		ParticleSystem::getColliderComponentSpec(this), 
		lifeSpan, 
		&position, 
		&force, 
		((ParticleSystemSpec*)this->actorSpec)->movementType
	);

	if(ParticleSystem::overrides(this, particleSpawned))
	{
		ParticleSystem::particleSpawned(this, particle);
	}

	return particle;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ParticleSystem::recycleParticle()
{
	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		if(particle->expired)
		{
			Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
			int16 lifeSpan = 
			((ParticleSystemSpec*)this->actorSpec)->particleSpec->minimumLifeSpan + 
			(0 != ((ParticleSystemSpec*)this->actorSpec)->particleSpec->lifeSpanDelta ? 
				Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->actorSpec)->particleSpec->lifeSpanDelta) 
				: 
				0
			);

			if(this->applyForceToParticles)
			{
				Vector3D force = ParticleSystem::getParticleSpawnForce(this);
				Particle::setup
				(
					particle, NULL, NULL, NULL, lifeSpan, &position, &force, ((ParticleSystemSpec*)this->actorSpec)->movementType
				);
			}
			else
			{
				Particle::setup
				(
					particle, NULL, NULL, NULL, lifeSpan, &position, NULL, ((ParticleSystemSpec*)this->actorSpec)->movementType
				);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D ParticleSystem::getParticleSpawnPosition()
{
	Vector3D position = this->transformation.position;

	if(0 != this->spawnPositionDisplacement.x)
	{
		position.x += 
		((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.x + 
		Math::random(Math::randomSeed(), this->spawnPositionDisplacement.x);
	}

	if(0 != this->spawnPositionDisplacement.y)
	{
		position.y += 
			((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.y + 
			Math::random(Math::randomSeed(), this->spawnPositionDisplacement.y);
	}

	if(0 != this->spawnPositionDisplacement.z)
	{
		position.z += 
			((ParticleSystemSpec*)this->actorSpec)->minimumRelativeSpawnPosition.z + 
			Math::random(Math::randomSeed(), this->spawnPositionDisplacement.z);
	}

	return position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D ParticleSystem::getParticleSpawnForce()
{
	Vector3D force = ((ParticleSystemSpec*)this->actorSpec)->minimumForce;

	if(0 != this->spawnForceDelta.x)
	{
		force.x += Math::random(Math::randomSeed(), this->spawnForceDelta.x);
	}

	if(0 != this->spawnForceDelta.y)
	{
		force.y += Math::random(Math::randomSeed(), this->spawnForceDelta.y);
	}

	if(0 != this->spawnForceDelta.z)
	{
		force.z += Math::random(Math::randomSeed(), this->spawnForceDelta.z);
	}

	return force;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ParticleSystem::appliesForceToParticles()
{
	if
	(
		0 != ((ParticleSystemSpec*)this->actorSpec)->minimumForce.x ||
		0 != ((ParticleSystemSpec*)this->actorSpec)->minimumForce.y ||
		0 != ((ParticleSystemSpec*)this->actorSpec)->minimumForce.z
	)
	{
		return true;
	}

	if
	(
		0 != ((ParticleSystemSpec*)this->actorSpec)->maximumForce.x ||
		0 != ((ParticleSystemSpec*)this->actorSpec)->maximumForce.y ||
		0 != ((ParticleSystemSpec*)this->actorSpec)->maximumForce.z
	)
	{
		return true;
	}

	return false;
} 

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const ComponentSpec* ParticleSystem::getVisualComponentSpec()
{
	if(0 == ((ParticleSystemSpec*)this->actorSpec)->visualComponentSpecs)
	{
		return NULL;
	}

	int32 specIndex = 0;

	if(1 < this->numberOfVisualComponentSpecs)
	{
		specIndex = Math::random(_gameRandomSeed, this->numberOfVisualComponentSpecs);
	}

	return ((ParticleSystemSpec*)this->actorSpec)->visualComponentSpecs[specIndex];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const ComponentSpec* ParticleSystem::getPhysicsComponentSpec()
{
	if(0 == ((ParticleSystemSpec*)this->actorSpec)->physicsComponentSpecs)
	{
		return NULL;
	}

	return ((ParticleSystemSpec*)this->actorSpec)->physicsComponentSpecs[0];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const ComponentSpec* ParticleSystem::getColliderComponentSpec()
{
	if(0 == ((ParticleSystemSpec*)this->actorSpec)->colliderComponentSpecs)
	{
		return NULL;
	}

	return ((ParticleSystemSpec*)this->actorSpec)->colliderComponentSpecs[0];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 ParticleSystem::computeNextSpawnTime()
{
	return ((ParticleSystemSpec*)this->actorSpec)->minimumSpawnDelay +
			(((ParticleSystemSpec*)this->actorSpec)->spawnDelayDelta ? 
				Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->actorSpec)->spawnDelayDelta) 
				: 
				0
			);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParticleSystem::processExpiredParticles()
{
	if(!((ParticleSystemSpec*)this->actorSpec)->recycleParticles)
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
					this->aliveParticlesCount--;
				}
			}

			this->particles = particles;
		}
	}
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
