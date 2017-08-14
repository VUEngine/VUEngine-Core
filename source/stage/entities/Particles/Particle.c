/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <PhysicalWorld.h>
#include <ObjectAnimatedSprite.h>
#include <Game.h>
#include <Clock.h>
#include <ParticleBody.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Particle
 * @extends SpatialObject
 * @ingroup stage-entities-particles
 */
__CLASS_DEFINITION(Particle, SpatialObject);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Particle_addSprite(Particle this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Particle, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
__CLASS_NEW_END(Particle, particleDefinition, spriteDefinition, lifeSpan, mass);

/**
 * Class constructor
 *
 * @memberof					Particle
 * @public
 *
 * @param this					Function scope
 * @param particleDefinition	Definition of the Particle
 * @param spriteDefinition
 * @param lifeSpan
 * @param mass
 */
void Particle_constructor(Particle this, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "Particle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(SpatialObject);

	this->particleDefinition = particleDefinition;
	this->spriteDefinition = spriteDefinition;
	this->lifeSpan = lifeSpan;
	this->body = PhysicalWorld_createBody(Game_getPhysicalWorld(Game_getInstance()), (BodyAllocator)__TYPE(ParticleBody), __SAFE_CAST(SpatialObject, this), mass);
	Body_setAxisSubjectToGravity(this->body, particleDefinition->axisSubjectToGravity);

	this->objectSprite = NULL;
	Particle_addSprite(this);
}

/**
 * Class destructor
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle_destructor(Particle this)
{
	ASSERT(__SAFE_CAST(Particle, this), "Particle::destructor: null this");

	// remove a body
	if(this->body)
	{
		// remove a body
		PhysicalWorld_destroyBody(Game_getPhysicalWorld(Game_getInstance()), this->body);
		this->body = NULL;
	}

	if(this->objectSprite)
	{
		__DELETE(this->objectSprite);
		this->objectSprite = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Add a sprite
 *
 * @memberof	Particle
 * @private
 *
 * @param this	Function scope
 */
static void Particle_addSprite(Particle this)
{
	ASSERT(this, "Particle::addSprite: null this");
	ASSERT(this->spriteDefinition->allocator, "Particle::load: no sprite allocator");

	// call the appropriate allocator to support inheritance
	this->objectSprite = __SAFE_CAST(ObjectSprite, ((Sprite (*)(const SpriteDefinition*, Object)) this->spriteDefinition->allocator)((SpriteDefinition*)this->spriteDefinition, __SAFE_CAST(Object, this)));

	if(this->particleDefinition->initialAnimation && this->particleDefinition->animationDescription && __SAFE_CAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite_play(__SAFE_CAST(Sprite, this->objectSprite), this->particleDefinition->animationDescription, this->particleDefinition->initialAnimation);
	}

	ASSERT(this->objectSprite, "Particle::addSprite: sprite not created");
}

/**
 * Update
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 * @param behavior
 *
 * @return				Boolean that tells whether a body was set active(?)
 */
u32 Particle_update(Particle this, u32 elapsedTime, void (* behavior)(Particle particle))
{
	ASSERT(this, "Particle::update: null this");

	if(0 <= this->lifeSpan)
	{
		this->lifeSpan -= elapsedTime;

		if(behavior)
		{
			behavior(this);
		}

		if(0 > this->lifeSpan)
		{
			Body_setActive(this->body, false);

			return true;
		}

		Sprite_updateAnimation(__SAFE_CAST(Sprite, this->objectSprite));
	}

	return false;
}

/**
 * Update Visual Representation
 *
 * @memberof					Particle
 * @public
 *
 * @param this					Function scope
 * @param updateSpritePosition
 */
void Particle_synchronizeGraphics(Particle this, bool updateSpritePosition)
{
	ASSERT(this, "Particle::synchronizeGraphics: null this");

	if(!(updateSpritePosition | Body_isAwake(this->body)))
	{
		return;
	}

	const VBVec3D* position = Body_getPosition(this->body);

	ASSERT(this->objectSprite, "Particle::synchronizeGraphics: null objectSprite");

	if(__Z_AXIS & Body_isMoving(this->body))
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(Sprite, calculateParallax, this->objectSprite, position->z);
	}

	// update sprite's 2D position
	__VIRTUAL_CALL(Sprite, position, this->objectSprite, position);
}

/**
 * Add force
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param force
 * @param movementType
 */
void Particle_addForce(Particle this, const Force* force, u32 movementType)
{
	ASSERT(this, "Particle::addForce: null this");

	if(__UNIFORM_MOVEMENT == movementType)
	{
		fix19_13 mass = Body_getMass(this->body);

		Acceleration acceleration =
		{
			force->x,
			force->y,
			force->z
		};

		if(mass)
		{
			acceleration.x = FIX19_13_DIV(acceleration.x, mass);
			acceleration.x = FIX19_13_DIV(acceleration.y, mass);
			acceleration.x = FIX19_13_DIV(acceleration.z, mass);
		};

		Velocity velocity =
		{
			acceleration.x,
			acceleration.y,
			acceleration.z
		};

		Body_moveUniformly(this->body, velocity);
	}
	else
	{
		Body_addForce(this->body, force, false);
	}
}

/**
 * Set lifespan
 *
 * @memberof		Particle
 * @public
 *
 * @param this		Function scope
 * @param lifeSpan
 */
void Particle_setLifeSpan(Particle this, int lifeSpan)
{
	ASSERT(this, "Particle::setLifeSpan: null this");

	this->lifeSpan = lifeSpan;
}

/**
 * Set mass
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 * @param mass
 */
void Particle_setMass(Particle this, fix19_13 mass)
{
	ASSERT(this, "Particle::setMass: null this");

	Body_setMass(this->body, mass);
}

/**
 * Set position
 *
 * @memberof		Particle
 * @public
 *
 * @param this		Function scope
 * @param position
 */
void Particle_setPosition(Particle this, const VBVec3D* position)
{
	ASSERT(this, "Particle::setPosition: null this");
	ASSERT(this->body, "Particle::setPosition: null body");

	Body_setPosition(this->body, position, __SAFE_CAST(SpatialObject, this));

	// sync sprite
	__VIRTUAL_CALL(Sprite, position, this->objectSprite, position);

	// calculate sprite's parallax
	__VIRTUAL_CALL(Sprite, calculateParallax, this->objectSprite, position->z);
}

/**
 * Retrieve position
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Position of particle's body
 */
const VBVec3D* Particle_getPosition(Particle this)
{
	ASSERT(this, "Particle::getPosition: null this");
	ASSERT(this->body, "Particle::getPosition: null body");

	return Body_getPosition(this->body);
}

/**
 * Make Particle visible
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle_show(Particle this)
{
	ASSERT(this, "Particle::show: null this");
	ASSERT(this->objectSprite, "Particle::show: null objectSprite");

	Sprite_show(__SAFE_CAST(Sprite, this->objectSprite));

	Body_setActive(this->body, true);

	if(this->particleDefinition->initialAnimation && this->particleDefinition->animationDescription && __SAFE_CAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite_play(__SAFE_CAST(Sprite, this->objectSprite), this->particleDefinition->animationDescription, this->particleDefinition->initialAnimation);
	}
}

/**
 * Make Particle invisible
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle_hide(Particle this)
{
	ASSERT(this, "Particle::hide: null this");
	ASSERT(this->objectSprite, "Particle::hide: null objectSprite");

	Sprite_hide(__SAFE_CAST(Sprite, this->objectSprite));

	Body_setActive(this->body, false);
}

/**
 * Does it move?
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 *
 * @return		Always true
 */
bool Particle_moves(Particle this __attribute__ ((unused)))
{
	ASSERT(this, "Particle::moves: null this");

	// not necessarily
	return true;
}

/**
 * Can move over axis?
 *
 * @memberof			Particle
 * @public
 *
 * @param this			Function scope
 * @param acceleration
 *
 * @return				Boolean that tells whether the Particle's body can move over axis (defaults to true)
 */
int Particle_canMoveOverAxis(Particle this, const Acceleration* acceleration __attribute__ ((unused)))
{
	ASSERT(this, "Particle::canMoveOverAxis: null this");

	return (int)Body_getAxisSubjectToGravity(this->body);
}

/**
 * Resume
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle_resume(Particle this)
{
	ASSERT(this, "Particle::resume: null this");

	Particle_addSprite(this);

	NM_ASSERT(this->objectSprite, "Particle::resume: null objectSprite");
}

/**
 * Pause
 *
 * @memberof	Particle
 * @public
 *
 * @param this	Function scope
 */
void Particle_suspend(Particle this)
{
	ASSERT(this, "Particle::suspend: null this");

	__DELETE(this->objectSprite);

	this->objectSprite = NULL;
}
