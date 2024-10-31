/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <PhysicalParticle.h>
#include <ParticleSystem.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Collider;

/**
 * Defines a SolidParticle
 *
 * @memberof	SolidParticle
 */
typedef struct SolidParticleSpec
{
	/// the class type
	PhysicalParticleSpec physicalParticleSpec;

	/// ball's radius
	fixed_t radius;

	/// friction for physics
	fixed_t frictionCoefficient;

	/// bounciness for physics
	fixed_t bounciness;

	/// object's in-game type
	uint32 inGameType;

	/// layers in which I live
	uint32 layers;

	/// layers to ignore when checking for collisions
	uint32 layersToIgnore;

	/// disable collision detection when the particle stops
	bool disableCollisionOnStop;

	/// animation to play upon collision
	const char* onCollisionAnimation;

} SolidParticleSpec;

/**
 * A SolidParticle that is stored in ROM
 *
 * @memberof	SolidParticle
 */
typedef const SolidParticleSpec SolidParticleROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class SolidParticle : PhysicalParticle
{
	ParticleSystem creator;
	// Particle's collider for collision detection
	Collider collider;
	//
	const SolidParticleSpec* solidParticleSpec;

	/// @publicsection
	void constructor(const SolidParticleSpec* solidParticleSpec, ParticleSystem creator);
	Collider getCollider();
	VirtualList getColliders();
	override fixed_t getWidth();
	override fixed_t getHeight();
	override fixed_t getDepth();
	override bool enterCollision(const CollisionInformation* collisionInformation);
	override void exitCollision(const CollisionInformation* collisionInformation);
	override bool isSubjectToGravity(Vector3D gravity);
	override bool handleMessage(Telegram telegram);
	override uint32 getInGameType();
	override const Vector3D* getVelocity();
	override void reset();
}


#endif
