/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ParticleRemover.h>
#include <Particle.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ParticleRemover_ATTRIBUTES																		\
        Object_ATTRIBUTES																				\
		/**
		 * @var VirtualList particlesLists
		 * @brief			List of Particles
		 * @memberof		ParticleRemover
		 */																								\
        VirtualList particlesLists;																		\
		/**
		 * @var int 		removalDelayCycles
		 * @brief			Removal delay
		 * @memberof		ParticleRemover
		 */																								\
        int removalDelayCycles;																			\
		/**
		 * @var int 		leftRemoveDelayCycles
		 * @brief			Removal delay
		 * @memberof		ParticleRemover
		 */																								\
        int leftRemoveDelayCycles;																		\

/**
 * @class	ParticleRemover
 * @extends Object
 */
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

/**
 * Class constructor
 *
 * @memberof	ParticleRemover
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) ParticleRemover_constructor(ParticleRemover this)
{
	ASSERT(this, "ParticleRemover::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(Object);

	this->particlesLists = __NEW(VirtualList);
	this->removalDelayCycles = 0;
	this->leftRemoveDelayCycles = this->removalDelayCycles;
}

/**
 * Class destructor
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
 */
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
/**
 * Reset
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
 */
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

/**
 * Update
 *
 * @memberof	ParticleRemover
 * @public
 *
 * @param this	Function scope
 */
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

/**
 * Delete given particles
 *
 * @memberof		ParticleRemover
 * @public
 *
 * @param this		Function scope
 * @param particles	List of Particles to delete
 */
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

/**
 * Set removal delay cycles
 *
 * @memberof					ParticleRemover
 * @public
 *
 * @param this					Function scope
 * @param removalDelayCycles	New value
 */
void ParticleRemover_setRemovalDelayCycles(ParticleRemover this, int removalDelayCycles)
{
	ASSERT(this, "ParticleRemover::registerParticle: null this");

    this->removalDelayCycles = removalDelayCycles;
}
