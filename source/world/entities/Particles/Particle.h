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

#ifndef PARTICLE_H_
#define PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SpatialObject.h>
#include <ObjectSprite.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __EVENT_PARTICLE_EXPIRED	"particleExpired"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Particle_METHODS																				\
		SpatialObject_METHODS																			\
		__VIRTUAL_DEC(update);																			\
		__VIRTUAL_DEC(transform);																		\
		__VIRTUAL_DEC(resume);																			\
		__VIRTUAL_DEC(suspend);																			\


// define the virtual methods
#define Particle_SET_VTABLE(ClassName)																	\
		SpatialObject_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, Particle, update);														\
		__VIRTUAL_SET(ClassName, Particle, transform);													\
		__VIRTUAL_SET(ClassName, Particle, moves);														\
		__VIRTUAL_SET(ClassName, Particle, canMoveOverAxis);											\
		__VIRTUAL_SET(ClassName, Particle, resume);														\
		__VIRTUAL_SET(ClassName, Particle, suspend);													\
		__VIRTUAL_SET(ClassName, Particle, setPosition);												\
		__VIRTUAL_SET(ClassName, Particle, getPosition);												\


#define Particle_ATTRIBUTES																				\
																										\
	/* super's attributes */																			\
	SpatialObject_ATTRIBUTES;																			\
																										\
	/* definition */																					\
	const ParticleDefinition* particleDefinition;														\
																										\
	/* definition */																					\
	const SpriteDefinition* spriteDefinition;															\
																										\
	/* OBJ based sprite */																				\
	ObjectSprite objectSprite;																			\
																										\
	/* physical body */																					\
	Body body;																							\
																										\
	/* life span in milliseconds */																		\
	int lifeSpan;																						\

__CLASS(Particle);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a Particle in ROM memory
typedef struct ParticleDefinition
{
	// the class type
	void* allocator;

	// particle's minimum life span in milliseconds
	u16 minimumLifeSpan;

	// particle's life span delta in milliseconds
	u16 lifeSpanDelta;

	// particle's minimum mass
	fix19_13 minimumMass;

	// particle's mass delta
	fix19_13 massDelta;

	// axis subject to gravity
	u8 axisSubjectToGravity;

	// function pointer to control particle's behavior
	void (* behavior)(Particle particle);

	// animation's name to play if sprite is animated
	AnimationDescription* animationDescription;

	// animation to play automatically
	char* initialAnimation;

} ParticleDefinition;

typedef const ParticleDefinition ParticleROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Particle, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);

void Particle_constructor(Particle this, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);
void Particle_destructor(Particle this);
void Particle_update(Particle this, u16 elapsedTime, void (* behavior)(Particle particle));
void Particle_transform(Particle this, bool updateSpritePosition);
void Particle_addForce(Particle this, const Force* force);
void Particle_setLifeSpan(Particle this, int lifeSpan);
void Particle_setMass(Particle this, fix19_13 mass);
void Particle_show(Particle this);
void Particle_hide(Particle this);
bool Particle_moves(Particle this);
u8 Particle_canMoveOverAxis(Particle this, const Acceleration* acceleration);
u16 Particle_getHeight(Particle this);
u16 Particle_getDepth(Particle this);
Gap Particle_getGap(Particle this);
void Particle_setPosition(Particle this, const VBVec3D* position);
const VBVec3D* Particle_getPosition(Particle this);
void Particle_resume(Particle this);
void Particle_suspend(Particle this);


#endif