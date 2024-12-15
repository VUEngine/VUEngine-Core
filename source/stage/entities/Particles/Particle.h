/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PARTICLE_H_
#define PARTICLE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <SpatialObject.h>
#include <Sprite.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Particle;
class ParticleSystem;
class Wireframe;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A Particle Spec
/// @memberof Particle
typedef struct ParticleSpec
{
	/// Class allocator
	AllocatorPointer allocator;

	/// Minimum life span in milliseconds
	uint16 minimumLifeSpan;

	/// Life span delta in milliseconds
	uint16 lifeSpanDelta;

	/// Function pointer to control particle's behavior
	void (* behavior)(Particle particle);

	/// Array of available animations
	const AnimationFunction** animationFunctions;

	/// Animation to play automatically
	char* initialAnimation;

} ParticleSpec;

/// A Particle spec that is stored in ROM
/// @memberof Particle
typedef const ParticleSpec ParticleROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Particle
///
/// Inherits from SpatialObject
///
/// Implements a particle that is spawned by particle systems.
/// @ingroup stage-entities-particles
class Particle : SpatialObject
{
	/// @protectedsection

	/// Remaining life span in milliseconds
	int16 lifeSpan;

	/// Sprite visual component
	Sprite sprite;

	/// Wireframe visual component
	Wireframe wireframe;

	/// If true the particle is set to be destroyed or recycled
	bool expired;

	/// @publicsection

	/// Class' constructor
	/// @param particleSpec: Specification that determines how to configure the particle
	void constructor(const ParticleSpec* particleSpec);

	/// Check if the particle is subject to gravity.
	/// @param gravity: Gravity vector
	/// @return True if gravity can affect the particle; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);
	
	/// Configure the particle with the provided arguments.
	/// @param spriteSpec: Specification for a sprite to add to the particle
	/// @param wireframeSpec: Specification for a wireframe to add to the particle
	/// @param lifeSpan: Time that the particle must live
	/// @param position: Starting position
	/// @param force: Force to apply
	/// @param movementType: Movement type on each axis
	/// @param animationFunctions: Array of animations
	/// @param animationName: Animation to play
	void setup(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, const char* animationName);

	/// Configure the particle with the provided arguments after unpausing the game.
	/// @param spriteSpec: Specification for a sprite to add to the particle
	/// @param wireframeSpec: Specification for a wireframe to add to the particle
	/// @param animationFunctions: Array of animations
	/// @param animationName: Animation to play
	void resume(const SpriteSpec* spriteSpec, const WireframeSpec* wireframeSpec, const AnimationFunction** animationFunctions, const char* animationName);

	/// Prepares the particle to become inactive in this state.
	void suspend();

	/// Force the particle to expire.
	void expire();

	/// Make the particle's visual components visible.
	void show();

	/// Make the particle's visual components invisible.
	void hide();

	/// Set the transparency mode of the visual components.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	void setTransparency(uint8 transparency);

	/// Check if the visual components are visible.
	/// @return True if the the particle's sprite or wireframe are visible; false otherwise
	bool isVisible();

	/// Reset the particle's state.
	virtual void reset();

	/// Update the particle's state.
	/// @param elapsedTime: Elapsed time since the last call
	/// @param behavior: Function pointer to control particle's behavior
	virtual bool update(uint32 elapsedTime, void (* behavior)(Particle particle));

	/// Apply a force to the particle.
	/// @param force: Force to be applied
	/// @param movementType: Movement type on each axis
	virtual void applyForce(const Vector3D* force, uint32 movementType);

	/// Configure the particle's mass.
	virtual void configureMass();
}

#endif
