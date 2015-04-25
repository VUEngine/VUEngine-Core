/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SolidParticle_METHODS													\
		Particle_METHODS														\

// define the virtual methods
#define SolidParticle_SET_VTABLE(ClassName)										\
		Particle_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, SolidParticle, getShape);						\

#define SolidParticle_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Particle_ATTRIBUTES;														\
																				\
	/* physical body */															\
	Shape shape;																\


__CLASS(SolidParticle);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a SolidParticle in ROM memory
typedef struct SolidParticleDefinition
{
	// the class type
	ParticleDefinition particleDefinition;

	// shape's type
	int shapeType;

} SolidParticleDefinition;

typedef const SolidParticleDefinition SolidParticleROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);

void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);
void SolidParticle_destructor(SolidParticle this);
Shape SolidParticle_getShape(SolidParticle this);


#endif