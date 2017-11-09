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

#ifndef PARTICLE_BODY_H_
#define PARTICLE_BODY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ParticleBody_METHODS(ClassName)																	\
		Body_METHODS(ClassName)																			\

#define ParticleBody_SET_VTABLE(ClassName)																\
		Body_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, ParticleBody, update);													\
		__VIRTUAL_SET(ClassName, ParticleBody, getTotalFriction);										\

#define ParticleBody_ATTRIBUTES																			\
		Body_ATTRIBUTES																					\

__CLASS(ParticleBody);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ParticleBody, SpatialObject owner, const PhysicalSpecification* physicalSpecification);

void ParticleBody_constructor(ParticleBody this, SpatialObject owner, const PhysicalSpecification* physicalSpecification);
void ParticleBody_destructor(ParticleBody this);

Force ParticleBody_getTotalFriction(ParticleBody this);
void ParticleBody_update(ParticleBody this);


#endif
