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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Entity.h>
#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Particle;
class ParticleSystem;
class Wireframe;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
	const char* initialAnimation;

	/// Animation to play upon collision
	const char* onCollisionAnimation;

	/// Particles's in-game type
	uint32 inGameType;

} ParticleSpec;

/// A Particle spec that is stored in ROM
/// @memberof Particle
typedef const ParticleSpec ParticleROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Particle
///
/// Inherits from Entity
///
/// Implements a particle that is spawned by particle systems.
class Particle : Entity
{
	/// @protectedsection

	/// Remaining life span in milliseconds
	int16 lifeSpan;

	/// If true the particle is set to be destroyed or recycled
	bool expired;

	/// ParticleSpec used to configure the particle
	const ParticleSpec* particleSpec;

	/// Cache the VisualComponent to bypass the ComponentManager's usage
	VisualComponent visualComponent;

	/// @publicsection

	/// Class' constructor
	/// @param particleSpec: Specification that determines how to configure the particle
	void constructor(const ParticleSpec* particleSpec);

	/// A component has been removed from this particle. 
	/// @param component: Removed component
	override void removedComponent(Component component);

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	override bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();

	/// Configure the particle with the provided arguments.
	/// @param visualComponentSpec: Specification for a sprite to add to the particle
	/// @param wireframeSpec: Specification for a wireframe to add to the particle
	/// @param lifeSpan: Time that the particle must live
	/// @param position: Starting position
	/// @param force: Force to apply
	/// @param movementType: Movement type on each axis
	/// @param animationFunctions: Array of animations
	/// @param animationName: Animation to play
	void setup
	(
		const ComponentSpec* visualComponentSpec, const ComponentSpec* physicsComponentSpec, const ComponentSpec* colliderComponentSpec, 
		int16 lifeSpan, const Vector3D* position, const Vector3D* force, uint32 movementType, const AnimationFunction** animationFunctions, 
		const char* animationName
	);

	/// Configure the particle with the provided arguments after unpausing the game.
	/// @param visualComponentSpec: Specification for a sprite to add to the particle
	/// @param wireframeSpec: Specification for a wireframe to add to the particle
	/// @param animationFunctions: Array of animations
	/// @param animationName: Animation to play
	void resume(const VisualComponentSpec* visualComponentSpec, const AnimationFunction** animationFunctions, const char* animationName);

	/// Prepares the particle to become inactive in this state.
	void suspend();

	/// Force the particle to expire.
	void expire();

	/// Check if the visual components are visible.
	/// @return True if the the particle's sprite or wireframe are visible; false otherwise
	bool isVisible();

	/// Play an animation.
	/// @param animationFunctions: Array of animations
	/// @param animationName: Animation to play
	void playAnimation(const AnimationFunction** animationFunctions, const char* animationName);

	/// Update the particle's state.
	/// @param elapsedTime: Elapsed time since the last call
	/// @param behavior: Function pointer to control particle's behavior
	virtual bool update(uint32 elapsedTime, void (* behavior)(Particle particle));
}

#endif
