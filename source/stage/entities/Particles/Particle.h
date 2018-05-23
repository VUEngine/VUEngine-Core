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
//											TYPE DEFINITIONS
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class Particle : SpatialObject
{
	// Particle's definition
	const ParticleDefinition* particleDefinition;
	// Particle's SpriteDefinition
	const SpriteDefinition* spriteDefinition;
	// OBJ based sprite
	ObjectSprite objectSprite;
	// Particle's physical body
	Body body;
	// Particle's life span in milliseconds
	int lifeSpan;

	/// @publicsection
	void constructor(const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass);
	void addForce(const Force* force, u32 movementType);
	u16 getDepth();
	u16 getHeight();
	void hide();
	void setLifeSpan(int lifeSpan);
	void setMass(fix10_6 mass);
	void show();
	bool isVisible();
	virtual u32 update(u32 elapsedTime, void (* behavior)(Particle particle));
	virtual void synchronizeGraphics(bool updateSpritePosition);
	virtual void transform();
	virtual void resume();
	virtual void suspend();
	virtual void reset();
	override bool isSubjectToGravity(Acceleration gravity);
	override void setPosition(const Vector3D* position);
	override const Vector3D* getPosition();
}


#endif
