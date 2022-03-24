/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	uint16 minimumLifeSpan;

	/// particle's life span delta in milliseconds
	uint16 lifeSpanDelta;

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
	int16 lifeSpan;
	bool expired;

	/// @publicsection
	void constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, int16 lifeSpan);
	void setLifeSpan(int16 lifeSpan);
	bool isVisible();
	void setup(int16 lifeSpan, const Vector3D* position, const Force* force, uint32 movementType, const AnimationDescription* animationDescription, const char* animationName, bool forceAnimation);
	void expire();
	virtual void synchronizeGraphics();
	virtual void applySustainedForce(const Force* force, uint32 movementType);
	virtual bool update(uint32 elapsedTime, void (* behavior)(Particle particle));
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
