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

#include <ParticleRemover.h>
#include <Particle.h>
#include <debugUtilities.h>


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
 * @private
 */
void ParticleRemover::constructor()
{
	// construct base
	Base::constructor();

	this->particlesLists = new VirtualList();
	this->removalDelayCycles = 0;
	this->remainingRemoveDelayCycles = this->removalDelayCycles;
}

/**
 * Class destructor
 */
void ParticleRemover::destructor()
{
	ParticleRemover::reset(this);

	if(!isDeleted(this->particlesLists))
	{
		delete this->particlesLists;
		this->particlesLists = NULL;
	}
	
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
/**
 * Reset
 */
void ParticleRemover::reset()
{
	VirtualNode node = this->particlesLists->head;

	for(; NULL != node; node = node->next)
	{
		VirtualList particlesList = node->data;
		VirtualNode particleNode = particlesList->head;

		for(; particleNode; particleNode = particleNode->next)
		{
			if(!isDeleted(particleNode->data))
			{
				delete particleNode->data;
			}
		}

		delete particlesList;
	}

	VirtualList::clear(this->particlesLists);

	this->remainingRemoveDelayCycles = this->removalDelayCycles;
}

/**
 * Update
 */
void ParticleRemover::update()
{
	if(!this->particlesLists->head)
	{
		return;
	}

	if(0 > this->removalDelayCycles)
	{
		ParticleRemover::reset(this);
	}
	else if(0 >= --this->remainingRemoveDelayCycles)
	{
		VirtualList particlesList = VirtualList::front(this->particlesLists);

		if(particlesList->head)
		{
			delete VirtualList::front(particlesList);
			VirtualList::popFront(particlesList);

			if(!VirtualList::getSize(particlesList))
			{
				delete particlesList;
				VirtualList::popFront(this->particlesLists);
			}
		}
		else
		{
			delete particlesList;
			VirtualList::popFront(this->particlesLists);
		}

		this->remainingRemoveDelayCycles = this->removalDelayCycles;
	}
}

/**
 * Delete given particles
 *
 * @param particles	List of Particles to delete
 */
void ParticleRemover::deleteParticles(VirtualList particles)
{
	if(VirtualList::safeCast(particles))
	{
		VirtualList particlesList = new VirtualList();

		particlesList->head = particles->head;
		particlesList->tail = particles->tail;

		particles->head = NULL;
		particles->tail = NULL;

		delete particles;

		VirtualList::pushBack(this->particlesLists, particlesList);
	}
}

/**
 * Set removal delay cycles
 *
 * @param removalDelayCycles	New value
 */
void ParticleRemover::setRemovalDelayCycles(int32 removalDelayCycles)
{
	this->removalDelayCycles = removalDelayCycles;
}
