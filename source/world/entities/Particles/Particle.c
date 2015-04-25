/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Particle.h>
#include <PhysicalWorld.h>
#include <ObjectAnimatedSprite.h>
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
extern const VBVec3D* _screenPosition;
const extern VBVec3D* _screenDisplacement;
extern const Optical* _optical;

static Clock _gameClock = NULL;

static void Particle_addSprite(Particle this, const SpriteDefinition* spriteDefinition);


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

	this->lifeSpan = lifeSpan;
	this->body = PhysicalWorld_registerBody(PhysicalWorld_getInstance(), __UPCAST(SpatialObject, this), mass);

	this->objectSprite = NULL;
	Particle_addSprite(this, spriteDefinition);
	
	if(particleDefinition->initialAnimation && particleDefinition->animationDescription && __UPCAST(ObjectAnimatedSprite, this->objectSprite))
	{
		Sprite_play(__UPCAST(Sprite, this->objectSprite), particleDefinition->animationDescription, particleDefinition->initialAnimation);
	}
}

// class's destructor
void Particle_destructor(Particle this)
{
	ASSERT(this, "Particle::destructor: null this");

	// remove a body
	PhysicalWorld_unregisterBody(PhysicalWorld_getInstance(), __UPCAST(SpatialObject, this));
	
	__DELETE(this->objectSprite);
	
	// destroy the super Container
	__DESTROY_BASE;
}

// add sprite
static void Particle_addSprite(Particle this, const SpriteDefinition* spriteDefinition)
{
	ASSERT(this, "Particle::addSprite: null this");
	ASSERT(spriteDefinition->allocator, "Particle::load: no sprite allocator defined");

	if (spriteDefinition->allocator)
	{
		// call the appropiate allocator to support inheritance!
		this->objectSprite = __UPCAST(ObjectSprite, ((Sprite (*)(SpriteDefinition*, ...)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, this));
	}

	ASSERT(this->objectSprite, "Particle::addSprite: sprite not created");
}

void Particle_update(Particle this, u16 timeElapsed)
{
	ASSERT(this, "Particle::update: null this");

	this->lifeSpan -= timeElapsed;
	Sprite_update(__UPCAST(Sprite, this->objectSprite), _gameClock);
	
	if(0 > this->lifeSpan)
	{
		Object_fireEvent(__UPCAST(Object, this), __EVENT_PARTICLE_EXPIRED);
	}
}

// transform 
void Particle_transform(Particle this)
{
	ASSERT(this, "Particle::transform: null this");
	ASSERT(this->body, "Particle::transform: null body");

	if (Body_isAwake(this->body))
    {
		VBVec3D position = Body_getPosition(this->body);

		// calculate sprite's parallax
		__VIRTUAL_CALL(void, Sprite, calculateParallax, this->objectSprite, position.z);
		
		// update sprite's 2D position
		__VIRTUAL_CALL(void, Sprite, synchronizePosition, this->objectSprite, position);
    }
}

void Particle_setPosition(Particle this, VBVec3D* position)
{
	ASSERT(this, "Particle::position: null this");
	ASSERT(this->body, "Particle::position: null body");
	
	Body_setPosition(this->body, position, __UPCAST(SpatialObject, this));
	ObjectSprite_synchronizePosition(this->objectSprite, *position);
	// calculate sprite's parallax
	__VIRTUAL_CALL(void, Sprite, calculateParallax, this->objectSprite, position->z);

}

// retrieve position
const VBVec3D* Particle_getPosition(Particle this)
{
	ASSERT(this, "Particle::getPosition: null this");
	ASSERT(this->body, "Particle::getPosition: null body");

	static VBVec3D position;
	position = Body_getPosition(this->body);
	
	return &position;
}

// make it visible
void Particle_show(Particle this)
{
	ASSERT(this, "Particle::show: null this");
	ASSERT(this->objectSprite, "Particle::show: null objectSprite");

	ObjectSprite_show(this->objectSprite);
}

// make it invisible
void Particle_hide(Particle this)
{
	ASSERT(this, "Particle::hide: null this");
	ASSERT(this->objectSprite, "Particle::hide: null objectSprite");
	
	ObjectSprite_hide(this->objectSprite);
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

	return __XAXIS | __YAXIS | __ZAXIS;
}