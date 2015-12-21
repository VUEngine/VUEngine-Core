/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <PhysicalWorld.h>
#include <ObjectAnimatedSprite.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Particle
__CLASS_DEFINITION(Particle, SpatialObject);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
static Clock _gameClock = NULL;

static void Particle_addSprite(Particle this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Particle, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
__CLASS_NEW_END(Particle, particleDefinition, spriteDefinition, lifeSpan, mass);

// class's constructor
void Particle_constructor(Particle this, const ParticleDefinition* particleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "Particle::constructor: null this");

	if(!_gameClock)
	{
		_gameClock = Game_getInGameClock(Game_getInstance());
	}
	
	// construct base Container
	__CONSTRUCT_BASE();

	this->particleDefinition = particleDefinition;
	this->spriteDefinition = spriteDefinition;
	this->lifeSpan = lifeSpan;
	this->body = PhysicalWorld_registerBody(PhysicalWorld_getInstance(), __SAFE_CAST(SpatialObject, this), mass);
	Body_setAxisSubjectToGravity(this->body, particleDefinition->axisSubjectToGravity);
	
	this->objectSprite = NULL;
	Particle_addSprite(this);
}

// class's destructor
void Particle_destructor(Particle this)
{
	ASSERT(__SAFE_CAST(Particle, this), "Particle::destructor: null this");

	// remove a body
	if(this->body)
	{
		// remove a body
		PhysicalWorld_unregisterBody(PhysicalWorld_getInstance(), __SAFE_CAST(SpatialObject, this));
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

// add sprite
static void Particle_addSprite(Particle this)
{
	ASSERT(this, "Particle::addSprite: null this");
	ASSERT(this->spriteDefinition->allocator, "Particle::load: no sprite allocator defined");

	// call the appropiate allocator to support inheritance!
	this->objectSprite = __SAFE_CAST(ObjectSprite, ((Sprite (*)(SpriteDefinition*, ...)) this->spriteDefinition->allocator)((SpriteDefinition*)this->spriteDefinition, this));

	if(this->particleDefinition->initialAnimation && this->particleDefinition->animationDescription && __SAFE_CAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite_play(__SAFE_CAST(Sprite, this->objectSprite), this->particleDefinition->animationDescription, this->particleDefinition->initialAnimation);
	}

	ASSERT(this->objectSprite, "Particle::addSprite: sprite not created");
}

void Particle_update(Particle this, u16 timeElapsed, void (* behavior)(Particle particle))
{
	ASSERT(this, "Particle::update: null this");

	if(0 <= this->lifeSpan)
	{
		this->lifeSpan -= timeElapsed;
		
		if(!Clock_isPaused(_gameClock))
		{
			Sprite_update(__SAFE_CAST(Sprite, this->objectSprite));
		}
		
		if(behavior)
		{
			behavior(this);
		}
		
		if(0 > this->lifeSpan)
		{
			Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_PARTICLE_EXPIRED);
		}
	}
}

// transform 
void Particle_transform(Particle this, bool updateSpritePosition)
{
	ASSERT(this, "Particle::transform: null this");
	ASSERT(this->body, "Particle::transform: null body");

	if(updateSpritePosition || Body_isAwake(this->body))
    {
		const VBVec3D* position = Body_getPosition(this->body);

		ASSERT(this->objectSprite, "Particle::transform: null objectSprite");

		if(__ZAXIS & Body_isMoving(this->body))
		{
			// calculate sprite's parallax
			__VIRTUAL_CALL(void, Sprite, calculateParallax, this->objectSprite, position->z);
		}
		
		// update sprite's 2D position
		__VIRTUAL_CALL(void, Sprite, position, this->objectSprite, position);
    }
}

void Particle_addForce(Particle this, const Force* force)
{
	ASSERT(this, "Particle::position: null this");
	
	Body_addForce(this->body, force);
}

void Particle_setLifeSpan(Particle this, int lifeSpan)
{
	ASSERT(this, "Particle::setLifeSpan: null this");
	
	this->lifeSpan = lifeSpan;
}

void Particle_setMass(Particle this, fix19_13 mass)
{
	ASSERT(this, "Particle::setMass: null this");
	
	Body_setMass(this->body, mass);
}


void Particle_setPosition(Particle this, const VBVec3D* position)
{
	ASSERT(this, "Particle::position: null this");
	ASSERT(this->body, "Particle::position: null body");
	
	Body_setPosition(this->body, position, __SAFE_CAST(SpatialObject, this));

	// sync sprite
	__VIRTUAL_CALL(void, Sprite, position, this->objectSprite, position);

	// calculate sprite's parallax
	__VIRTUAL_CALL(void, Sprite, calculateParallax, this->objectSprite, position->z);
}

// retrieve position
const VBVec3D* Particle_getPosition(Particle this)
{
	ASSERT(this, "Particle::getPosition: null this");
	ASSERT(this->body, "Particle::getPosition: null body");

	return Body_getPosition(this->body);
}

// make it visible
void Particle_show(Particle this)
{
	ASSERT(this, "Particle::show: null this");
	ASSERT(this->objectSprite, "Particle::show: null objectSprite");

	ObjectSprite_show(this->objectSprite);
	Body_setActive(this->body, true);
}

// make it invisible
void Particle_hide(Particle this)
{
	ASSERT(this, "Particle::hide: null this");
	ASSERT(this->objectSprite, "Particle::hide: null objectSprite");
	
	ObjectSprite_hide(this->objectSprite);
	Body_setActive(this->body, false);
}

// does it move?
bool Particle_moves(Particle this)
{
	ASSERT(this, "Particle::moves: null this");

	// not necessarily
	return true;
}


// defaults to true
u8 Particle_canMoveOverAxis(Particle this, const Acceleration* acceleration)
{
	ASSERT(this, "Particle::canMoveOverAxis: null this");

	return Body_getAxisSubjectToGravity(this->body);
}

void Particle_resume(Particle this)
{
	ASSERT(this, "Particle::resume: null this");

	Particle_addSprite(this);
	
	NM_ASSERT(this->objectSprite, "Particle::resume: null this");
}

void Particle_suspend(Particle this)
{
	ASSERT(this, "Particle::suspend: null this");

	__DELETE(this->objectSprite);
	
	this->objectSprite = NULL;
}
