/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleRemover.h>
#include <Particle.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ParticleRemover_ATTRIBUTES																		\
        /* it is derived from */																		\
        Object_ATTRIBUTES																				\
        /* particle list */																				\
        VirtualList particlesLists;																		\
        /* remove delay */																				\
        int removalDelayCycles;																			\
        /* remove delay */																				\
        int leftRemoveDelayCycles;																		\

// define the ParticleRemover
__CLASS_DEFINITION(ParticleRemover, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void __attribute__ ((noinline)) ParticleRemover_constructor(ParticleRemover this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ParticleRemover)
__CLASS_NEW_END(ParticleRemover);

// class's constructor
static void __attribute__ ((noinline)) ParticleRemover_constructor(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(Object);

	this->particlesLists = __NEW(VirtualList);
	this->removalDelayCycles = 0;
	this->leftRemoveDelayCycles = this->removalDelayCycles;
}

// class's destructor
void ParticleRemover_destructor(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::destructor: null this");

    ParticleRemover_reset(this);

    __DELETE(this->particlesLists);
    this->particlesLists = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void ParticleRemover_reset(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::reset: null this");

    VirtualNode node = this->particlesLists->head;

    for(; node; node = node->next)
    {
        VirtualList particlesList = node->data;
        VirtualNode particleNode = particlesList->head;

        for(; particleNode; particleNode = particleNode->next)
        {
            if(*(u32*)particleNode->data)
            {
                __DELETE(particleNode->data);
            }
        }

        __DELETE(particlesList);
    }

    VirtualList_clear(this->particlesLists);

    this->leftRemoveDelayCycles = this->removalDelayCycles;
}

void ParticleRemover_update(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::update: null this");

	if(0 > this->removalDelayCycles)
	{
        ParticleRemover_reset(this);
    }
    else if(0 >= --this->leftRemoveDelayCycles)
    {
        if(this->particlesLists->head)
        {
            VirtualList particlesList = VirtualList_front(this->particlesLists);

            if(particlesList->head)
            {
                __DELETE(VirtualList_front(particlesList));
                VirtualList_popFront(particlesList);

                if(!VirtualList_getSize(particlesList))
                {
                    VirtualList_popFront(this->particlesLists);
                    __DELETE(particlesList);
                }
            }
    	}

    	this->leftRemoveDelayCycles = this->removalDelayCycles;
    }
}

void ParticleRemover_deleteParticles(ParticleRemover this, VirtualList particles)
{
	ASSERT(this, "ParticleRemover::registerParticles: null this");

    if(__SAFE_CAST(VirtualList, particles))
    {
        VirtualList particlesList = __NEW(VirtualList);

        particlesList->head = particles->head;
        particlesList->tail = particles->tail;

        particles->head = NULL;
        particles->tail = NULL;

        __DELETE(particles);

        VirtualList_pushBack(this->particlesLists, particlesList);
	}
}

void ParticleRemover_setRemovalDelayCycles(ParticleRemover this, int removalDelayCycles)
{
	ASSERT(this, "ParticleRemover::registerParticle: null this");

    this->removalDelayCycles = removalDelayCycles;
}
