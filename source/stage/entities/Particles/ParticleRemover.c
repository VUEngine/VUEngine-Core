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

#include <ParticleRemover.h>
#include <Particle.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ParticleRemover
 * @extends Object
 * @ingroup stage-entities-particles
 */

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	ParticleRemover
 * @private
 *
 * @param this	Function scope
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
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
 */
void ParticleRemover::destructor()
{
	ParticleRemover::reset(this);

	delete this->particlesLists;
	this->particlesLists = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
/**
 * Reset
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
 */
void ParticleRemover::reset()
{
	VirtualNode node = this->particlesLists->head;

	for(; node; node = node->next)
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
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
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
 * @memberof		ParticleRemover
 * @public
 *
 * @param this		Function scope
 * @param particles	List of Particles to delete
 */
void ParticleRemover::deleteParticles(VirtualList particles)
{
	if(__SAFE_CAST(VirtualList, particles))
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
 * @memberof					ParticleRemover
 * @public
 *
 * @param this					Function scope
 * @param removalDelayCycles	New value
 */
void ParticleRemover::setRemovalDelayCycles(int removalDelayCycles)
{
	this->removalDelayCycles = removalDelayCycles;
}
