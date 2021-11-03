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
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

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
	fix10_6 radius;

	/// friction for physics
	fix10_6 frictionCoefficient;

	/// bounciness for physics
	fix10_6 bounciness;

	/// object's in-game type
	uint32 inGameType;

	/// layers in which I live
	uint32 layers;

	/// layers to ignore when checking for collisions
	uint32 layersToIgnore;

	/// disable collision detection when the particle stops
	bool disableCollisionOnStop;

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
	// Particle's shape for collision detection
	Shape shape;
	//
	const SolidParticleSpec* solidParticleSpec;

	/// @publicsection
	void constructor(const SolidParticleSpec* solidParticleSpec, const SpriteSpec* spriteSpec, int16 lifeSpan);
	Shape getShape();
	VirtualList getShapes();
	override fix10_6 getWidth();
	override fix10_6 getHeight();
	override fix10_6 getDepth();
	override bool enterCollision(const CollisionInformation* collisionInformation);
	override bool isSubjectToGravity(Acceleration gravity);
	override bool handleMessage(Telegram telegram);
	override void transform();
	override void setPosition(const Vector3D* position);
	override uint32 getInGameType();
	override Velocity getVelocity();
	override void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void reset();
}


#endif
