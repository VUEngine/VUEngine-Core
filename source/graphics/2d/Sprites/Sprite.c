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

#include <Sprite.h>
#include <SpriteManager.h>
#include <AnimationController.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Sprite, Object);

__CLASS_FRIEND_DEFINITION(Texture);

//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition __attribute__ ((unused)), Object owner __attribute__ ((unused)))
{
	__CONSTRUCT_BASE(Object);

	// clear values
	this->worldLayer = 0;
	this->head = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->displacement = (VBVec3D){0, 0, 0};
	this->hidden = false;
    this->writeAnimationFrame = false;
}

// class's destructor
void Sprite_destructor(Sprite this)
{
	ASSERT(this, "Sprite::destructor: null this");
	ASSERT(__SAFE_CAST(Sprite, this), "Sprite::destructor: null cast");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

Scale Sprite_getScale(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::getScale: null this");

	Scale scale =
	{
			__1I_FIX7_9, __1I_FIX7_9
	};

	//  return the scale
	return scale;
}

// calculate zoom scaling factor
void Sprite_resize(Sprite this, Scale scale __attribute__ ((unused)), fix19_13 z __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::resize: null this");

	this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
	this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
}

// retrieve the texture
Texture Sprite_getTexture(Sprite this)
{
	ASSERT(this, "Sprite::getTexture: null this");

	return this->texture;
}

// show
void Sprite_show(Sprite this)
{
	ASSERT(this, "Sprite::show: null this");

	this->hidden = false;
}

// hide
void Sprite_hide(Sprite this)
{
	ASSERT(this, "Sprite::hide: null this");

	this->hidden = true;
}

bool Sprite_isHidden(Sprite this)
{
	ASSERT(this, "Sprite::isHidden: null this");

	return this->hidden;
}

// retrieve animation controller
AnimationController Sprite_getAnimationController(Sprite this)
{
	ASSERT(this, "Sprite::hide: getAnimationController this");

	return this->animationController;
}

// set map's world layer
void Sprite_setWorldLayer(Sprite this, u8 worldLayer)
{
	ASSERT(this, "Sprite::setWorldLayer: null this");

    this->worldLayer = worldLayer;
}

//get map's world layer
u8 Sprite_getWorldLayer(Sprite this)
{
	ASSERT(this, "Sprite::getWorldLayer: null this");

	return this->worldLayer;
}

// get sprite's render head
u16 Sprite_getHead(Sprite this)
{
	ASSERT(this, "Sprite::getHead: null this");

	return this->head;
}

// get sprite's render mode
u16 Sprite_getMode(Sprite this)
{
	ASSERT(this, "Sprite::getMode: null this");

	return this->head & 0x3000;
}

// get sprites's layer's width
u32 Sprite_getWorldHead(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHead: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->head;
}


// get sprites's layer's width
s16 Sprite_getWorldX(Sprite this)
{
	ASSERT(this, "Sprite::getWorldWidth: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gx;
}

// get sprites's layer's width
s16 Sprite_getWorldY(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHeight: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gy;
}

// get sprites's layer's width
u16 Sprite_getWorldWidth(Sprite this)
{
	ASSERT(this, "Sprite::getWorldWidth: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->w;
}

// get sprites's layer's width
u16 Sprite_getWorldHeight(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHeight: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->h;
}

// reload the sprite in bgmap memory
void Sprite_rewrite(Sprite this)
{
	ASSERT(this, "Sprite::reload: null this");

	if(this->texture)
	{
		// write it in graphical memory
		Texture_rewrite(this->texture);
	}
}

// process event
void Sprite_onTextureRewritten(Sprite this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::onTextureRewritten: null this");

	__VIRTUAL_CALL(Sprite, applyAffineTransformations, this);
	__VIRTUAL_CALL(Sprite, applyHbiasTransformations, this);
}

// get displacement
VBVec3D Sprite_getDisplacement(Sprite this)
{
	ASSERT(this, "Sprite::getRenderFlag: null this");

	return this->displacement;
}

void Sprite_rotate(Sprite this __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::rotate: null this");
}

int Sprite_getHalfWidth(Sprite this)
{
	ASSERT(this, "Sprite::getHalfWidth: null this");

    return FIX19_13TOI(this->halfWidth);
}

int Sprite_getHalfHeight(Sprite this)
{
	ASSERT(this, "Sprite::getHalfHeight: null this");

    return FIX19_13TOI(this->halfHeight);
}

//---------------------------------------------------------------------------------------------------------
// 										Animation
//---------------------------------------------------------------------------------------------------------

void Sprite_update(Sprite this)
{
	ASSERT(this, "Sprite::update: null this");

	if(this->animationController && this->texture->written)
	{
		// first animate the frame
		if(this->writeAnimationFrame)
		{
			__VIRTUAL_CALL(Sprite, writeAnimation, this);
        	this->writeAnimationFrame = false;
		}
	}
}

void Sprite_animate(Sprite this)
{
	ASSERT(this, "Sprite::animate: null this");

	if(this->animationController)
	{
		// first animate the frame
		this->writeAnimationFrame |= AnimationController_animate(this->animationController);
	}
}

void Sprite_pause(Sprite this, bool pause)
{
	ASSERT(this, "Sprite::pause: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_pause(this->animationController, pause);
	}
}

void Sprite_play(Sprite this, AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(this, "Sprite::play: null this");
	ASSERT(animationDescription, "Sprite::play: null animationDescription");
	ASSERT(functionName, "Sprite::play: null functionName");

	if(this->animationController)
	{
		this->writeAnimationFrame = AnimationController_play(this->animationController, animationDescription, functionName);
	}
}

bool Sprite_isPlaying(Sprite this)
{
	ASSERT(this, "Sprite::isPlaying: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_isPlaying(this->animationController);
	}

	return false;
}

bool Sprite_isPlayingFunction(Sprite this, char* functionName)
{
	ASSERT(this, "Sprite::isPlayingFunction: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_isPlayingFunction(this->animationController, functionName);
	}

	return false;
}

void Sprite_setFrameDelayDelta(Sprite this, u8 frameDelayDelta)
{
	ASSERT(this, "Sprite::setFrameDelayDelta: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setFrameDelayDelta(this->animationController, frameDelayDelta);
	}
}

s8 Sprite_getActualFrame(Sprite this)
{
	ASSERT(this, "Sprite::getActualFrame: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_getActualFrame(this->animationController);
	}

	return -1;
}

void Sprite_setActualFrame(Sprite this, s8 actualFrame)
{
	ASSERT(this, "Sprite::setActualFrame: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setActualFrame(this->animationController, actualFrame);
	}
}

s8 Sprite_getFrameDelay(Sprite this)
{
	ASSERT(this, "Sprite::getFrameDelay: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_getFrameDelay(this->animationController);
	}

	return -1;
}

void Sprite_setFrameDelay(Sprite this, u8 frameDelay)
{
	ASSERT(this, "Sprite::setFrameDelay: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setFrameDelay(this->animationController, frameDelay);
	}
}

void Sprite_writeAnimation(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::writeAnimation: null this");
}


//---------------------------------------------------------------------------------------------------------
// 										FXs
//---------------------------------------------------------------------------------------------------------

// write directly to texture
void Sprite_putChar(Sprite this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Sprite::putChar: null this");

	if(this->texture && newChar && texturePixel)
	{
		Texture_putChar(this->texture, texturePixel, newChar);
	}
}

// write directly to texture
void Sprite_putPixel(Sprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Sprite::putPixel: null this");

	if(this->texture)
	{
		Texture_putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
	}
}

/*
 * Affine FX
 */

void Sprite_applyAffineTransformations(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyAffineTransformations: null this");
}

void Sprite_applyHbiasTransformations(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyHbiasTransformations: null this");
}
