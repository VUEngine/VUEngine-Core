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

#include <SolidParticle.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the SolidParticle
__CLASS_DEFINITION(SolidParticle, Particle);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SolidParticle, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
__CLASS_NEW_END(SolidParticle, solidParticleDefinition, spriteDefinition, lifeSpan, mass);

// class's constructor
void SolidParticle_constructor(SolidParticle this, const SolidParticleDefinition* solidParticleDefinition, const SpriteDefinition* spriteDefinition, int lifeSpan, fix19_13 mass)
{
	ASSERT(this, "SolidParticle::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(solidParticleDefinition->particleDefinition);

	this->solidParticleDefinition = solidParticleDefinition;

	// register a shape for collision detection
	this->shape = CollisionManager_registerShape(CollisionManager_getInstance(), __GET_CAST(SpatialObject, this), solidParticleDefinition->shapeType);
}

// class's destructor
void SolidParticle_destructor(SolidParticle this)
{
	ASSERT(this, "SolidParticle::destructor: null this");

	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);

	this->shape = NULL;
	
	// destroy the super Container
	__DESTROY_BASE;
}

void SolidParticle_update(SolidParticle this, u16 timeElapsed, void (* behavior)(Particle particle))
{
	ASSERT(this, "SolidParticle::update: null this");

	Particle_update(__GET_CAST(Particle, this), timeElapsed, behavior);
}

// retrieve shape
Shape SolidParticle_getShape(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getShape: null this");

	return this->shape;
}

// get width
u16 SolidParticle_getWidth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getWidth: null this");

	return this->solidParticleDefinition->width;
}

// get height
u16 SolidParticle_getHeight(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getHeight: null this");

	return this->solidParticleDefinition->height;
}

// get depth
u16 SolidParticle_getDepth(SolidParticle this)
{
	ASSERT(this, "SolidParticle::getDepth: null this");

	// must calculate based on the scale because not affine object must be enlarged
	return this->solidParticleDefinition->depth;
}

// process a telegram
bool SolidParticle_handleMessage(SolidParticle this, Telegram telegram)
{
	ASSERT(this, "SolidParticle::handleMessage: null this");
/*
	switch(Telegram_getMessage(telegram))
    {
		case kCollision:

			SolidParticle_resolveCollision(this, __GET_CAST(VirtualList, Telegram_getExtraInfo(telegram)));
			return true;
			break;
			
		case kBodyStartedMoving:

			CollisionManager_shapeStartedMoving(CollisionManager_getInstance(), this->shape);
			SolidParticle_updateCollisionStatus(this, *(u8*)Telegram_getExtraInfo(telegram));
			return true;
			break;

		case kBodyStoped:

			if(!Body_isMoving(this->body))
            {
				CollisionManager_shapeStopedMoving(CollisionManager_getInstance(), this->shape);
			}
			break;

		case kBodyBounced:

			SolidParticle_changeDirectionOnAxis(this, *(int*)Telegram_getExtraInfo(telegram));
			return true;
			break;
	}
*/
	return false;
}