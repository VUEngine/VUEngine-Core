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
        VirtualList particles;																			\
        /* remove delay */																				\
        int removalDelayCicles;																			\
        /* remove delay */																				\
        int leftRemoveDelayCicles;																		\

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

__SINGLETON(ParticleRemover);

// class's constructor
static void __attribute__ ((noinline)) ParticleRemover_constructor(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(Object);

	this->particles = __NEW(VirtualList);
	this->removalDelayCicles = 0;
	this->leftRemoveDelayCicles = this->removalDelayCicles;
}

// class's destructor
void ParticleRemover_destructor(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::destructor: null this");

    ParticleRemover_reset(this);

    __DELETE(this->particles);
    this->particles = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

void ParticleRemover_reset(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::reset: null this");

    VirtualNode node = this->particles->head;

    for(; node; node = node->next)
    {
        __DELETE(node->data);
    }

    VirtualList_clear(this->particles);

    this->leftRemoveDelayCicles = this->removalDelayCicles;
}

void ParticleRemover_update(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::update: null this");

    if(0 >= --this->leftRemoveDelayCicles)
    {
        if(this->particles->head)
        {
            __DELETE(VirtualList_front(this->particles));
            VirtualList_popFront(this->particles);
    	}

    	this->leftRemoveDelayCicles = this->removalDelayCicles;
    }
}

void ParticleRemover_registerParticle(ParticleRemover this, Particle particle)
{
	ASSERT(this, "ParticleRemover::registerParticle: null this");

    ASSERT(!VirtualList_find(this->particles, particle), "ParticleRemover::registerParticle: particle already registerd for deletion");

	VirtualList_pushBack(this->particles, particle);
}

void ParticleRemover_setRemovalDelayCicles(ParticleRemover this, int removalDelayCicles)
{
	ASSERT(this, "ParticleRemover::registerParticle: null this");

    this->removalDelayCicles = removalDelayCicles;
}
