/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PARTICLE_REMOVER_H_
#define PARTICLE_REMOVER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Particle.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class ParticleRemover : ListenerObject
{
	// List of Particles
	VirtualList particlesLists;
	// Removal delay
	int32 removalDelayCycles;
	// Removal delay
	int32 remainingRemoveDelayCycles;

	/// @publicsection
	void constructor();
	void deleteParticles(VirtualList particles);
	void reset();
	void setRemovalDelayCycles(int32 removalDelayCycles);
	void update();
}


#endif
