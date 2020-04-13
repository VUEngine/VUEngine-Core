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
	u32 inGameType;

	/// layers in which I live
	u32 layers;

	/// layers to ignore when checking for collisions
	u32 layersToIgnore;

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
	void constructor(const SolidParticleSpec* solidParticleSpec, const SpriteSpec* spriteSpec, int lifeSpan);
	Shape getShape();
	override bool update(u32 elapsedTime, void (* behavior)(Particle particle));
	override fix10_6 getWidth();
	override fix10_6 getHeight();
	override fix10_6 getDepth();
	override bool enterCollision(const CollisionInformation* collisionInformation);
	override bool isSubjectToGravity(Acceleration gravity);
	override bool handleMessage(Telegram telegram);
	override void transform();
	override void setPosition(const Vector3D* position);
	override VirtualList getShapes();
	override u32 getInGameType();
	override Velocity getVelocity();
	override void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void reset();
}


#endif
