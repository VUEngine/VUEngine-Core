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
#include <VPUManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Sprite, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition, Object owner)
{
	__CONSTRUCT_BASE(Object);

	// clear values
	this->worldLayer = 0;
	this->head = 0;
	this->renderFlag = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->displacement = (VBVec3D){0, 0, 0};
	this->hidden = false;
	this->initialized = false;
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

Scale Sprite_getScale(Sprite this)
{
	ASSERT(this, "Sprite::getScale: null this");

	Scale scale =
	{
			ITOFIX7_9(1), ITOFIX7_9(1)
	};

	//  return the scale
	return scale;
}

// calculate zoom scaling factor
void Sprite_resize(Sprite this, Scale scale, fix19_13 z)
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

// set to true to allow render
void Sprite_setRenderFlag(Sprite this, bool renderFlag)
{
	ASSERT(this, "Sprite::setRenderFlag: null this");

	// do not override the whole world entry, or will be updated in the
	// next render
	if(__UPDATE_HEAD != this->renderFlag || !renderFlag)
	{
		this->renderFlag = !renderFlag ? 0 : this->renderFlag | renderFlag;
	}
}

// show
void Sprite_show(Sprite this)
{
	ASSERT(this, "Sprite::show: null this");

	this->renderFlag = __UPDATE_HEAD;
	this->hidden = false;

	// since I can have been moved after being hidden, I need to force
	// a complete update before showing up
	this->initialized = false;
}

// hide
void Sprite_hide(Sprite this)
{
	ASSERT(this, "Sprite::hide: null this");

	this->renderFlag = true;
	this->hidden = true;
}

bool Sprite_isHidden(Sprite this)
{
	ASSERT(this, "Sprite::isHidden: null this");

	return this->hidden;
}

// retrieve animation controller
const AnimationController const Sprite_getAnimationController(Sprite this)
{
	ASSERT(this, "Sprite::hide: getAnimationController this");

	return this->animationController;
}


// get render flag
u32 Sprite_getRenderFlag(Sprite this)
{
	ASSERT(this, "Sprite::getRenderFlag: null this");

	return this->renderFlag;
}

// set map's world layer
void Sprite_setWorldLayer(Sprite this, u8 worldLayer)
{
	ASSERT(this, "Sprite::setWorldLayer: null this");

	if(this->worldLayer != worldLayer)
	{
		this->worldLayer = worldLayer;

		// make sure everything is setup in the next render cycle
		this->renderFlag = __UPDATE_HEAD;
	}
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
u16 Sprite_getWorldWidth(Sprite this)
{
	ASSERT(this, "Sprite::getWorldWidth: null this");

	WORLD* worldPointer = &WA[this->worldLayer];
	return worldPointer->w;
}

// get sprites's layer's width
u16 Sprite_getWorldHeight(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHeight: null this");

	WORLD* worldPointer = &WA[this->worldLayer];
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

	// raise flag to render again
	this->renderFlag = __UPDATE_HEAD;
}

// process event
void Sprite_onTextureRewritten(Sprite this, Object eventFirer)
{
	ASSERT(this, "Sprite::onTextureRewritten: null this");

	__VIRTUAL_CALL(Sprite, applyAffineTransformations, this);
	__VIRTUAL_CALL(Sprite, applyHbiasTransformations, this);
	this->renderFlag = __UPDATE_HEAD;
}

// get displacement
VBVec3D Sprite_getDisplacement(Sprite this)
{
	ASSERT(this, "Sprite::getRenderFlag: null this");

	return this->displacement;
}

void Sprite_rotate(Sprite this, const Rotation* rotation)
{
	ASSERT(this, "Sprite::rotate: null this");
}


//---------------------------------------------------------------------------------------------------------
// 										Animation
//---------------------------------------------------------------------------------------------------------

void Sprite_update(Sprite this)
{
	ASSERT(this, "Sprite::update: null this");

	if(this->animationController)
	{
		// first animate the frame
		if(AnimationController_didAnimationFrameChanged(this->animationController))
		{
			__VIRTUAL_CALL(Sprite, writeAnimation, this);
		}
	}
}

void Sprite_animate(Sprite this)
{
	ASSERT(this, "Sprite::animate: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_animate(this->animationController);
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
		// first animate the frame
		AnimationController_play(this->animationController, animationDescription, functionName);
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

bool Sprite_isPlayingFunction(Sprite this, AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(this, "Sprite::isPlayingFunction: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_isPlayingFunction(this->animationController, animationDescription, functionName);
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

void Sprite_writeAnimation(Sprite this)
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

void Sprite_applyAffineTransformations(Sprite this)
{
	ASSERT(this, "Sprite::applyAffineTransformations: null this");
}

void Sprite_applyHbiasTransformations(Sprite this)
{
	ASSERT(this, "Sprite::applyHbiasTransformations: null this");
}
