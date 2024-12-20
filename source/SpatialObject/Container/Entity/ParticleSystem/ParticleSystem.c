/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Particle.h>
#include <Printing.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VUEngine.h>

#include "ParticleSystem.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Particle;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ParticleSystem::constructor(const ParticleSystemSpec* particleSystemSpec, int16 internalId, const char* const name)
{
	// construct base
	Base::constructor((EntitySpec*)&particleSystemSpec->entitySpec, internalId, name);

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
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::destructor()
{
	ParticleSystem::deleteAllParticles(this);

	if(!isDeleted(this->particles))
	{
		delete this->particles;
		this->particles = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
			this->aliveParticlesCount--;
		}

		NM_ASSERT(0 <= this->aliveParticlesCount, "ParticleSystem::update: negative particle count");
	}

	if(this->selfDestroyWhenDone && this->totalSpawnedParticles >= this->maximumNumberOfAliveParticles && 0 == this->aliveParticlesCount && !this->loop)
	{
		ParticleSystem::deleteMyself(this);
		return;
	}

	if(this->paused || this->hidden)
	{
		return;
	}

	// check if it is time to spawn new particles
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

			if(!((ParticleSystemSpec*)this->entitySpec)->recycleParticles)
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
		while(++spawnedParticles < ((ParticleSystemSpec*)this->entitySpec)->maximumNumberOfParticlesToSpawnPerCycle && 0 == ((ParticleSystemSpec*)this->entitySpec)->minimumSpawnDelay && this->aliveParticlesCount < this->maximumNumberOfAliveParticles);
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::resume()
{
	if(isDeleted(this->particles))
	{
		return;
	}

	for(VirtualNode node = this->particles->head; NULL != node; node = node->next)
	{
		Particle particle = Particle::safeCast(node->data);

		Particle::resume(particle, ParticleSystem::getVisualComponentSpec(this), ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation);
	}

	this->nextSpawnTime = ParticleSystem::computeNextSpawnTime(this);

	// Now call base
	Base::resume(this);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::setSpec(void* particleSystemSpec)
{
	if(NULL == particleSystemSpec)
	{
		return;
	}

	Base::setSpec(this, particleSystemSpec);
	ParticleSystem::setup(this);
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::start()
{
	this->update = true;
	this->nextSpawnTime = 0;
	this->totalSpawnedParticles = 0;
	this->paused = false;
	ParticleSystem::show(this);
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::pause()
{
	this->paused = true;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
bool ParticleSystem::isPaused()
{
	return this->paused && 0 == this->aliveParticlesCount;
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::deleteAllParticles()
{
	if(!isDeleted(this->particles))
	{
		VirtualList::deleteData(this->particles);
	}

	this->aliveParticlesCount = 0;
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::setLoop(bool value)
{
	this->loop = value;
}
//---------------------------------------------------------------------------------------------------------
bool ParticleSystem::getLoop()
{
	return this->loop;
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::setSelfDestroyWhenDone(bool selfDestroyWhenDone)
{
	this->selfDestroyWhenDone = selfDestroyWhenDone;
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::setElapsedTime(uint32 elapsedTime)
{
	this->elapsedTime = elapsedTime;
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::print(int16 x, int16 y)
{
	Printing::text(Printing::getInstance(), "PARTICLE SYSTEM ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Particles", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Maximum:    ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->maximumNumberOfAliveParticles, x + 10, y, NULL);
	Printing::text(Printing::getInstance(), "Spawned:    ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->particles), x + 10, y, NULL);
	Printing::text(Printing::getInstance(), "Alive:      ", x + 1, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->aliveParticlesCount, x + 10, y, NULL);
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::particleSpawned(Particle particle __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::particleRecycled(Particle particle __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ParticleSystem::setup()
{
	NM_ASSERT(this->entitySpec, "ParticleSystem::setup: NULL spec");

	if(NULL == this->entitySpec)
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
	this->paused = !((ParticleSystemSpec*)this->entitySpec)->autoStart;
	this->maximumNumberOfAliveParticles = 0;

	ParticleSystem::configure(this);

	this->update = ((ParticleSystemSpec*)this->entitySpec)->autoStart;
	this->applyForceToParticles = ParticleSystem::appliesForceToParticles(this);
}
//---------------------------------------------------------------------------------------------------------
void ParticleSystem::configure()
{
	this->size.x += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.x - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x);
	this->size.y += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.y - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y);
	this->size.z += __ABS(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition.z - ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z);

	this->spawnPositionDisplacement = Vector3D::absolute(Vector3D::sub(((ParticleSystemSpec*)this->entitySpec)->maximumRelativeSpawnPosition, ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition));
	this->spawnForceDelta = Vector3D::absolute(Vector3D::sub(((ParticleSystemSpec*)this->entitySpec)->maximumForce, ((ParticleSystemSpec*)this->entitySpec)->minimumForce));

	this->nextSpawnTime = 0;
	this->maximumNumberOfAliveParticles = ((ParticleSystemSpec*)this->entitySpec)->maximumNumberOfAliveParticles;

	// Calculate the number of sprite specs
	for(this->numberOfVisualComponentSpecs = 0; 0 <= this->numberOfVisualComponentSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->visualComponentSpecs && NULL != ((ParticleSystemSpec*)this->entitySpec)->visualComponentSpecs[this->numberOfVisualComponentSpecs]; this->numberOfVisualComponentSpecs++);
}
//---------------------------------------------------------------------------------------------------------
Particle ParticleSystem::spawnParticle()
{
	// call the appropriate allocator to support inheritance
	Particle particle = ((Particle (*)(const ParticleSpec*)) ((ParticleSystemSpec*)this->entitySpec)->particleSpec->allocator)(((ParticleSystemSpec*)this->entitySpec)->particleSpec);

	int16 lifeSpan = ((ParticleSystemSpec*)this->entitySpec)->particleSpec->minimumLifeSpan + Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->lifeSpanDelta);
	Vector3D position = ParticleSystem::getParticleSpawnPosition(this);
	Vector3D force = Vector3D::zero();

	if(this->applyForceToParticles)
	{
		force = ParticleSystem::getParticleSpawnForce(this);
	}

	Particle::setup(particle, ParticleSystem::getVisualComponentSpec(this), lifeSpan, &position, &force, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation);

	if(ParticleSystem::overrides(this, particleSpawned))
	{
		ParticleSystem::particleSpawned(this, particle);
	}

	return particle;
}
//---------------------------------------------------------------------------------------------------------
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
				Particle::setup(particle, ParticleSystem::getVisualComponentSpec(this), lifeSpan, &position, &force, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation);
			}
			else
			{
				Particle::setup(particle, ParticleSystem::getVisualComponentSpec(this), lifeSpan, &position, NULL, ((ParticleSystemSpec*)this->entitySpec)->movementType, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->animationFunctions, ((ParticleSystemSpec*)this->entitySpec)->particleSpec->initialAnimation);
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
//---------------------------------------------------------------------------------------------------------
Vector3D ParticleSystem::getParticleSpawnPosition()
{
	Vector3D position = this->transformation.position;

	if(0 != this->spawnPositionDisplacement.x)
	{
		position.x += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.x + Math::random(Math::randomSeed(), this->spawnPositionDisplacement.x);
	}

	if(0 != this->spawnPositionDisplacement.y)
	{
		position.y += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.y + Math::random(Math::randomSeed(), this->spawnPositionDisplacement.y);
	}

	if(0 != this->spawnPositionDisplacement.z)
	{
		position.z += ((ParticleSystemSpec*)this->entitySpec)->minimumRelativeSpawnPosition.z + Math::random(Math::randomSeed(), this->spawnPositionDisplacement.z);
	}

	return position;
}
//---------------------------------------------------------------------------------------------------------
Vector3D ParticleSystem::getParticleSpawnForce()
{
	Vector3D force = ((ParticleSystemSpec*)this->entitySpec)->minimumForce;

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
//---------------------------------------------------------------------------------------------------------
bool ParticleSystem::appliesForceToParticles()
{
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
//---------------------------------------------------------------------------------------------------------
const VisualComponentSpec* ParticleSystem::getVisualComponentSpec()
{
	if(0 == this->numberOfVisualComponentSpecs)
	{
		return NULL;
	}

	int32 specIndex = 0;

	if(1 < this->numberOfVisualComponentSpecs)
	{
		specIndex = Math::random(_gameRandomSeed, this->numberOfVisualComponentSpecs);
	}

	return (const VisualComponentSpec*)((ParticleSystemSpec*)this->entitySpec)->visualComponentSpecs[specIndex];
}
//---------------------------------------------------------------------------------------------------------
int32 ParticleSystem::computeNextSpawnTime()
{
	return ((ParticleSystemSpec*)this->entitySpec)->minimumSpawnDelay +
			(((ParticleSystemSpec*)this->entitySpec)->spawnDelayDelta ? Math::random(_gameRandomSeed, ((ParticleSystemSpec*)this->entitySpec)->spawnDelayDelta) : 0);
}
//---------------------------------------------------------------------------------------------------------
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
					this->aliveParticlesCount--;
				}
			}

			this->particles = particles;
		}
	}
}//---------------------------------------------------------------------------------------------------------
