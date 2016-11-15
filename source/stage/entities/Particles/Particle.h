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
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Particle_METHODS(ClassName)																		\
		SpatialObject_METHODS(ClassName)																\
		__VIRTUAL_DEC(ClassName, u32, update, int elapsedTime, void (* behavior)(Particle particle));	\
		__VIRTUAL_DEC(ClassName, void, updateVisualRepresentation, bool updateSpritePosition);			\
		__VIRTUAL_DEC(ClassName, void, resume);															\
		__VIRTUAL_DEC(ClassName, void, suspend);														\


// define the virtual methods
#define Particle_SET_VTABLE(ClassName)																	\
		SpatialObject_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, Particle, update);														\
		__VIRTUAL_SET(ClassName, Particle, updateVisualRepresentation);									\
		__VIRTUAL_SET(ClassName, Particle, moves);														\
		__VIRTUAL_SET(ClassName, Particle, canMoveOverAxis);											\
		__VIRTUAL_SET(ClassName, Particle, resume);														\
		__VIRTUAL_SET(ClassName, Particle, suspend);													\
		__VIRTUAL_SET(ClassName, Particle, setPosition);												\
		__VIRTUAL_SET(ClassName, Particle, getPosition);												\


#define Particle_ATTRIBUTES																				\
        SpatialObject_ATTRIBUTES																		\
		/*
		 * @var ParticleDefinition* particleDefinition
		 * @brief					Particle's definition
		 * @memberof				Particle
		 */																								\
        const ParticleDefinition* particleDefinition;													\
		/*
		 * @var SpriteDefinition* 	spriteDefinition
		 * @brief					Particle's SpriteDefinition
		 * @memberof				Particle
		 */																								\
        const SpriteDefinition* spriteDefinition;														\
		/*
		 * @var ObjectSprite 		objectSprite
		 * @brief					OBJ based sprite
		 * @memberof				Particle
		 */																								\
        ObjectSprite objectSprite;																		\
		/*
		 * @var Body 				body
		 * @brief					Particle's physical body
		 * @memberof				Particle
		 */																								\
        Body body;																						\
		/*
		 * @var int 				lifeSpan
		 * @brief					Particle's life span in milliseconds
		 * @memberof				Particle
		 */																								\
        int lifeSpan;																					\

__CLASS(Particle);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Defines a Particle
 *
 * @memberof	Particle
 */
typedef struct ParticleDefinition
{
	/// the class allocator
	AllocatorPointer allocator;

	/// particle's minimum life span in milliseconds
	u16 minimumLifeSpan;

	/// particle's life span delta in milliseconds
	u16 lifeSpanDelta;

	/// particle's minimum mass
	fix19_13 minimumMass;

	/// particle's mass delta
	fix19_13 massDelta;

	/// axis subject to gravity
	u8 axisSubjectToGravity;

	/// function pointer to control particle's behavior
	void (* behavior)(Particle particle);

	/// animation's name to play if sprite is animated
	AnimationDescription* animationDescription;

	/// animation to play automatically
	char* initialAnimation;

} ParticleDefinition;

/**
 * A Particle that is stored in ROM
 *
 * @memberof	Particle
 */
typedef const ParticleDefinition ParticleROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Particle, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);

void Particle_constructor(Particle this, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass);
void Particle_destructor(Particle this);

void Particle_addForce(Particle this, const Force* force, u32 movementType);
int Particle_canMoveOverAxis(Particle this, const Acceleration* acceleration);
u16 Particle_getDepth(Particle this);
Gap Particle_getGap(Particle this);
u16 Particle_getHeight(Particle this);
const VBVec3D* Particle_getPosition(Particle this);
void Particle_hide(Particle this);
bool Particle_moves(Particle this);
void Particle_resume(Particle this);
void Particle_setLifeSpan(Particle this, int lifeSpan);
void Particle_setMass(Particle this, fix19_13 mass);
void Particle_setPosition(Particle this, const VBVec3D* position);
void Particle_show(Particle this);
void Particle_suspend(Particle this);
void Particle_transform(Particle this);
u32 Particle_update(Particle this, u32 elapsedTime, void (* behavior)(Particle particle));
void Particle_updateVisualRepresentation(Particle this, bool updateSpritePosition);


#endif
