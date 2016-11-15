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

#ifndef PARTICLE_BODY_H_
#define PARTICLE_BODY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Body.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ParticleBody_METHODS(ClassName)																	\
		Body_METHODS(ClassName)																	        \

#define ParticleBody_SET_VTABLE(ClassName)																\
		Body_SET_VTABLE(ClassName)																        \
		__VIRTUAL_SET(ClassName, ParticleBody, update);									                \
		__VIRTUAL_SET(ClassName, ParticleBody, calculateFrictionForce);									\

#define ParticleBody_ATTRIBUTES																			\
        Body_ATTRIBUTES																			        \

__CLASS(ParticleBody);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ParticleBody, SpatialObject owner, fix19_13 mass);

void ParticleBody_constructor(ParticleBody this, SpatialObject owner, fix19_13 mass);
void ParticleBody_destructor(ParticleBody this);

Force ParticleBody_calculateFrictionForce(ParticleBody this);
void ParticleBody_update(ParticleBody this);


#endif
