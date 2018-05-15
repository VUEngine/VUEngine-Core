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

#ifndef SOLID_PARTICLE_H_
#define SOLID_PARTICLE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <Shape.h>

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Defines a SolidParticle
 *
 * @memberof	SolidParticle
 */
typedef struct SolidParticleDefinition
{
	/// the class type
	ParticleDefinition particleDefinition;

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

} SolidParticleDefinition;

/**
 * A SolidParticle that is stored in ROM
 *
 * @memberof	SolidParticle
 */
typedef const SolidParticleDefinition SolidParticleROMDef;


class SolidParticle : Particle
{
	/*
	* @var Shape 						shape
	* @brief							Particle's shape for collision detection
	* @memberof						SolidParticle
	*/
	Shape shape;
	/*
	* @var SolidParticleDefinition*	shapeParticleDefinition
	* @brief
	* @memberof						SolidParticle
	*/
	const SolidParticleDefinition* solidParticleDefinition;

	void constructor(SolidParticle this, const SolidParticleDefinition* shapeParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix10_6 mass);
	Shape getShape(SolidParticle this);
	override u32 update(SolidParticle this, int timeElapsed, void (* behavior)(Particle particle));
	override u16 getWidth(SolidParticle this);
	override u16 getHeight(SolidParticle this);
	override u16 getDepth(SolidParticle this);
	override bool enterCollision(SolidParticle this, const CollisionInformation* collisionInformation);
	override bool isSubjectToGravity(SolidParticle this, Acceleration gravity);
	override bool handleMessage(SolidParticle this, Telegram telegram);
	override void transform(SolidParticle this);
	override void setPosition(SolidParticle this, const Vector3D* position);
	override VirtualList getShapes(SolidParticle this);
	override u32 getInGameType(SolidParticle this);
	override Velocity getVelocity(SolidParticle this);
	override void exitCollision(SolidParticle this, Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void reset(SolidParticle this);
}


#endif
