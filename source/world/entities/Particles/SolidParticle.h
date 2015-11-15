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

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <Shape.h>
#include <CollisionSolver.h>


//---------------------------------------------------------------------------------------------------------
// 										MISC
//---------------------------------------------------------------------------------------------------------

// needed because of interdependency between Shape's and SpatialObject's headers
Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SolidParticle_METHODS													\
		Particle_METHODS														\

// define the virtual methods
#define SolidParticle_SET_VTABLE(ClassName)										\
		Particle_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, SolidParticle, update);						\
		__VIRTUAL_SET(ClassName, SolidParticle, getShape);						\
		__VIRTUAL_SET(ClassName, SolidParticle, getWidth);						\
		__VIRTUAL_SET(ClassName, SolidParticle, getHeight);						\
		__VIRTUAL_SET(ClassName, SolidParticle, getDepth);						\
		__VIRTUAL_SET(ClassName, SolidParticle, getShape);						\
		__VIRTUAL_SET(ClassName, SolidParticle, handleMessage);					\
		__VIRTUAL_SET(ClassName, Particle, setPosition);						\


#define SolidParticle_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Particle_ATTRIBUTES;														\
																				\
	/* physical body */															\
	Shape shape;																\
																				\
	/* shape for collision detection */											\
	const SolidParticleDefinition* solidParticleDefinition;						\
																				\
	/* previous position for collision handling */								\
	VBVec3D previousGlobalPosition;												\
																				\
	/* collision solver */														\
	CollisionSolver collisionSolver;											\
																				\
	/* position */																\
	VBVec3D position;															\

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
	
	// object's size over the x axis
	u16 width;

	// object's size over the y axis
	u16 height;

	// object's size over the z axis
	u16 depth;
	
	// friction for physics
	fix19_13 friction;

	// elasticity for physics
	fix19_13 elasticity;

	// flag to ignore collisions against other particles
	bool ignoreParticles;

} SolidParticleDefinition;

typedef const SolidParticleDefinition SolidParticleROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);

void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);
void SolidParticle_destructor(SolidParticle this);
void SolidParticle_update(SolidParticle this, u16 timeElapsed, void (* behavior)(Particle particle));
Shape SolidParticle_getShape(SolidParticle this);
u16 SolidParticle_getWidth(SolidParticle this);
u16 SolidParticle_getHeight(SolidParticle this);
u16 SolidParticle_getDepth(SolidParticle this);
bool SolidParticle_handleMessage(SolidParticle this, Telegram telegram);
void SolidParticle_setPosition(SolidParticle this, const VBVec3D* position);


#endif