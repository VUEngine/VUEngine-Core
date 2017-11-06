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

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <Shape.h>
#include <CollisionSolver.h>


//---------------------------------------------------------------------------------------------------------
//										MISC
//---------------------------------------------------------------------------------------------------------

// needed because of interdependency between Shape's and SpatialObject's headers
Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SolidParticle_METHODS(ClassName)																\
		Particle_METHODS(ClassName)																		\

// define the virtual methods
#define SolidParticle_SET_VTABLE(ClassName)																\
		Particle_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, SolidParticle, update);												\
		__VIRTUAL_SET(ClassName, SolidParticle, getWidth);												\
		__VIRTUAL_SET(ClassName, SolidParticle, getHeight);												\
		__VIRTUAL_SET(ClassName, SolidParticle, getDepth);												\
		__VIRTUAL_SET(ClassName, SolidParticle, processCollision);										\
		__VIRTUAL_SET(ClassName, SolidParticle, handleMessage);											\
		__VIRTUAL_SET(ClassName, Particle, setPosition);												\

#define SolidParticle_ATTRIBUTES																		\
		Particle_ATTRIBUTES																				\
		/*
		 * @var Shape 						shape
		 * @brief							Particle's shape for collision detection
		 * @memberof						SolidParticle
		 */																								\
		Shape shape;																					\
		/*
		 * @var SolidParticleDefinition*	shapeParticleDefinition
		 * @brief
		 * @memberof						SolidParticle
		 */																								\
		const SolidParticleDefinition* shapeParticleDefinition;											\
		/*
		 * @var VBVec3D 					previousGlobalPosition
		 * @brief							Particle's previous position, for collision handling
		 * @memberof						SolidParticle
		 */																								\
		VBVec3D previousGlobalPosition;																	\
		/*
		 * @var CollisionSolver 			collisionSolver
		 * @brief							Particle's collision solver
		 * @memberof						SolidParticle
		 */																								\
		CollisionSolver collisionSolver;																\
		/*
		 * @var VBVec3D 					position
		 * @brief							Particle's position
		 * @memberof						SolidParticle
		 */																								\
		VBVec3D position;																				\

__CLASS(SolidParticle);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Defines a SolidParticle
 *
 * @memberof	SolidParticle
 */
typedef struct SolidParticleDefinition
{
	/// the class type
	ParticleDefinition particleDefinition;

	/// shape definition
	const ShapeDefinition* shapeDefinition;

	/// object's size over the x axis
	u16 width;

	/// object's size over the y axis
	u16 height;

	/// object's size over the z axis
	u16 depth;

	/// friction for physics
	fix19_13 friction;

	/// elasticity for physics
	fix19_13 elasticity;

	/// flag to ignore collisions against other particles
	bool ignoreParticles;

	/// flag to allow bouncing on each axis
	u8 axisAllowedForBouncing;

} SolidParticleDefinition;

/**
 * A SolidParticle that is stored in ROM
 *
 * @memberof	SolidParticle
 */
typedef const SolidParticleDefinition SolidParticleROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(SolidParticle, const SolidParticleDefinition* shapeParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);

void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* shapeParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);
void SolidParticle_destructor(SolidParticle this);

u16 SolidParticle_getWidth(SolidParticle this);
u16 SolidParticle_getHeight(SolidParticle this);
u16 SolidParticle_getDepth(SolidParticle this);
Shape SolidParticle_getShape(SolidParticle this);
bool SolidParticle_processCollision(SolidParticle this, const CollisionInformation* collisionInformation);
bool SolidParticle_handleMessage(SolidParticle this, Telegram telegram);
void SolidParticle_setPosition(SolidParticle this, const VBVec3D* position);
u32 SolidParticle_update(SolidParticle this, int timeElapsed, void (* behavior)(Particle particle));


#endif
