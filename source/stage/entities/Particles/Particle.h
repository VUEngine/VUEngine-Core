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


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Particle;
class ParticleSystem;
class Wireframe;

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
	const AnimationFunction** animationFunctions;

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
	fixed_t previousZ;
	// sprite
	Sprite sprite;
	// sprite
	Wireframe wireframe;
	// Particle's life span in milliseconds
	int16 lifeSpan;
	bool expired;
	bool transform;

	/// @publicsection
	void constructor(const ParticleSpec* particleSpec, const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, ParticleSystem creator);
	void setLifeSpan(int16 lifeSpan);
	bool isVisible();
	void setup(int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName, bool forceAnimation);
	void expire();
	void hide();
	void show();
	void setTransparent(uint8 transparent);
	void resume(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions, const char* animationName);
	void suspend();
	virtual void applySustainedForce(const Vector3D* force, uint32 movementType);
	virtual bool update(uint32 elapsedTime, void (* behavior)(Particle particle));
	virtual void transform();
	virtual void reset();
	virtual void changeMass();

	override bool isSubjectToGravity(Vector3D gravity);
	override void setPosition(const Vector3D* position);
	override const Vector3D* getPosition();
}


#endif
