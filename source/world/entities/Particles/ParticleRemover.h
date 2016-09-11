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

#ifndef PARTICLE_REMOVER_H_
#define PARTICLE_REMOVER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Particle.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ParticleRemover_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																					\

#define ParticleRemover_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\


__CLASS(ParticleRemover);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ParticleRemover ParticleRemover_getInstance();

void ParticleRemover_destructor(ParticleRemover this);
void ParticleRemover_reset(ParticleRemover this);
void ParticleRemover_update(ParticleRemover this);
void ParticleRemover_deleteParticles(ParticleRemover this, VirtualList particles);
void ParticleRemover_setRemovalDelayCicles(ParticleRemover this, int removalDelayCicles);

#endif
