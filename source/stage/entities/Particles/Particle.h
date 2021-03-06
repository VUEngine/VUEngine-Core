/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <Sprite.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Particle;

typedef struct ParticleSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// particle's minimum life span in milliseconds
	u16 minimumLifeSpan;

	/// particle's life span delta in milliseconds
	u16 lifeSpanDelta;

	/// function pointer to control particle's behavior
	void (* behavior)(Particle particle);

	/// name of animation to play if sprite is animated
	AnimationDescription* animationDescription;

	/// animation to play automatically
	char* initialAnimation;

} ParticleSpec;

/**
 * A Particle that is stored in ROM
 *
 * @memberof	Particle
 */
typedef const ParticleSpec ParticleROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class Particle : SpatialObject
{
	// Particle's life span in milliseconds
	Vector3D position;
	// To optimizize parallax computation
	fix10_6 previousZ;
	// sprite
	Sprite sprite;
	// Particle's life span in milliseconds
	s16 lifeSpan;
	bool expired;

	/// @publicsection
	void constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, s16 lifeSpan);
	void setLifeSpan(s16 lifeSpan);
	bool isVisible();
	void setup(s16 lifeSpan, const Vector3D* position, const Force* force, u32 movementType, const AnimationDescription* animationDescription, const char* animationName, bool forceAnimation);
	void expire();
	virtual void synchronizeGraphics();
	virtual void addForce(const Force* force, u32 movementType);
	virtual bool update(u32 elapsedTime, void (* behavior)(Particle particle));
	virtual void transform();
	virtual void resume(const SpriteSpec* spriteSpec, const AnimationDescription* animationDescription, const char* animationName);
	virtual void suspend();
	virtual void reset();
	virtual void setMass(fix10_6 mass);
	virtual void changeMass();
	virtual void hide();
	virtual void show();
	void setAnimationName(const char* animationName);
	override bool isSubjectToGravity(Acceleration gravity);
	override void setPosition(const Vector3D* position);
	override const Vector3D* getPosition();
}


#endif
