/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef PARTICLE_H_
#define PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SpatialObject.h>
#include <ObjectSprite.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Particle;

typedef struct ParticleDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	/// particle's minimum life span in milliseconds
	u16 minimumLifeSpan;

	/// particle's life span delta in milliseconds
	u16 lifeSpanDelta;

	/// particle's minimum mass
	fix10_6 minimumMass;

	/// particle's mass delta
	fix10_6 massDelta;

	/// axes subject to gravity (bitwise or of __X_AXIS, __Y_AXIS, __Z_AXIS, or false to disable)
	u16 axesSubjectToGravity;

	/// function pointer to control particle's behavior
	void (* behavior)(Particle particle);

	/// name of animation to play if sprite is animated
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


class Particle : SpatialObject
{
	/*
	* @var ParticleDefinition* particleDefinition
	* @brief					Particle's definition
	* @memberof				Particle
	*/
	const ParticleDefinition* particleDefinition;
	/*
	* @var SpriteDefinition* 	spriteDefinition
	* @brief					Particle's SpriteDefinition
	* @memberof				Particle
	*/
	const SpriteDefinition* spriteDefinition;
	/*
	* @var ObjectSprite 		objectSprite
	* @brief					OBJ based sprite
	* @memberof				Particle
	*/
	ObjectSprite objectSprite;
	/*
	* @var Body 				body
	* @brief					Particle's physical body
	* @memberof				Particle
	*/
	Body body;
	/*
	* @var int 				lifeSpan
	* @brief					Particle's life span in milliseconds
	* @memberof				Particle
	*/
	int lifeSpan;

	void constructor(Particle this, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass);
	void addForce(Particle this, const Force* force, u32 movementType);
	u16 getDepth(Particle this);
	u16 getHeight(Particle this);
	void hide(Particle this);
	void setLifeSpan(Particle this, int lifeSpan);
	void setMass(Particle this, fix10_6 mass);
	void show(Particle this);
	bool isVisible(Particle this);
	virtual u32 update(Particle this, u32 elapsedTime, void (* behavior)(Particle particle));
	virtual void synchronizeGraphics(Particle this, bool updateSpritePosition);
	virtual void transform(Particle this);
	virtual void resume(Particle this);
	virtual void suspend(Particle this);
	virtual void reset(Particle this);
	override bool isSubjectToGravity(Particle this, Acceleration gravity);
	override void setPosition(Particle this, const Vector3D* position);
	override const Vector3D* getPosition(Particle this);
}


#endif
