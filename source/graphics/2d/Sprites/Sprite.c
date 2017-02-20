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

#include <Sprite.h>
#include <SpriteManager.h>
#include <AnimationController.h>
#include <VIPManager.h>
#include <BgmapTexture.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	Sprite
 * @extends Object
 * @ingroup graphics-2d-sprites
 */
__CLASS_DEFINITION(Sprite, Object);
__CLASS_FRIEND_DEFINITION(Texture);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

AnimationController Sprite_getAnimationController(Sprite this);
void Sprite_onTextureRewritten(Sprite this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (General)
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof				Sprite
 * @public
 *
 * @param this				Function scope
 * @param spriteDefinition	Definition of the Sprite
 * @param owner				Entity the Sprite belongs to
 */
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
	this->displacement = (VBVecWorld){0, 0, 0, 0};
	this->hidden = false;
	this->visible = true;
	this->transparent = spriteDefinition? spriteDefinition->transparent : false;
	this->writeAnimationFrame = false;
}

/**
 * Class destructor
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_destructor(Sprite this)
{
	ASSERT(this, "Sprite::destructor: null this");
	ASSERT(__SAFE_CAST(Sprite, this), "Sprite::destructor: null cast");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Get scale
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Scale struct
 */
Scale Sprite_getScale(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::getScale: null this");

	Scale scale =
	{
		__1I_FIX7_9,
		__1I_FIX7_9,
	};

	// return the scale
	return scale;
}

/**
 * Calculate zoom scaling factor
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 * @param scale	Scale struct to apply
 * @param z
 */
void Sprite_resize(Sprite this, Scale scale __attribute__ ((unused)), fix19_13 z __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::resize: null this");

	this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
	this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
}

/**
 * Retrieve the texture
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Texture struct
 */
Texture Sprite_getTexture(Sprite this)
{
	ASSERT(this, "Sprite::getTexture: null this");

	return this->texture;
}

/**
 * Show
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_show(Sprite this)
{
	ASSERT(this, "Sprite::show: null this");

	this->hidden = false;
}

/**
 * Hide
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_hide(Sprite this)
{
	ASSERT(this, "Sprite::hide: null this");

	this->hidden = true;
}

/**
 * Is the Sprite hidden?
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean telling whether the sprite is hidden
 */
bool Sprite_isHidden(Sprite this)
{
	ASSERT(this, "Sprite::isHidden: null this");

	return this->hidden;
}

/**
 * Set direction
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param axis			Axis to modify
 * @param direction		Direction value
 */
void Sprite_setDirection(Sprite this __attribute__ ((unused)), int axis __attribute__ ((unused)), int direction __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::setDirection: null this");
}

/**
 * Calculate 2D position
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param position		3D position
 */
void Sprite_position(Sprite this __attribute__ ((unused)), const VBVec3D* position __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::position: null this");
}

/**
 * Calculate parallax
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param z				Z coordinate to base on the calculation
 */
void Sprite_calculateParallax(Sprite this __attribute__ ((unused)), fix19_13 z __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::calculateParallax: null this");
}

/**
 * Retrieve animation controller
 *
 * @memberof	Sprite
 * @private
 *
 * @param this	Function scope
 *
 * @return		Sprite's AnimationController
 */
AnimationController Sprite_getAnimationController(Sprite this)
{
	ASSERT(this, "Sprite::getAnimationController: null this");

	return this->animationController;
}

/**
 * Set map's world layer
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param worldLayer	World layer
 */
void Sprite_setWorldLayer(Sprite this, u8 worldLayer)
{
	ASSERT(this, "Sprite::setWorldLayer: null this");

	this->worldLayer = worldLayer;
}

/**
 * Get WORLD layer
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return 		World layer
 */
u8 Sprite_getWorldLayer(Sprite this)
{
	ASSERT(this, "Sprite::getWorldLayer: null this");

	return this->worldLayer;
}

/**
 * Get sprite's render head
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
u16 Sprite_getHead(Sprite this)
{
	ASSERT(this, "Sprite::getHead: null this");

	return this->head;
}

/**
 * Get sprite's render mode
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Mode
 */
u16 Sprite_getMode(Sprite this)
{
	ASSERT(this, "Sprite::getMode: null this");

	return this->head & 0x3000;
}

/**
 * Get sprites's world head
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
u32 Sprite_getWorldHead(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHead: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->head;
}

/**
 * Get sprites's layer's gx
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite_getWorldX(Sprite this)
{
	ASSERT(this, "Sprite::getWorldX: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gx;
}

/**
 * Get sprites's layer's gy
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite_getWorldY(Sprite this)
{
	ASSERT(this, "Sprite::getWorldY: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gy;
}

/**
 * Get sprites's layer's width
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Width
 */
u16 Sprite_getWorldWidth(Sprite this)
{
	ASSERT(this, "Sprite::getWorldWidth: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->w;
}

/**
 * Get sprites's layer's height
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Width
 */
u16 Sprite_getWorldHeight(Sprite this)
{
	ASSERT(this, "Sprite::getWorldHeight: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->h;
}

/**
 * Reload the sprite in BGMap memory
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_rewrite(Sprite this)
{
	ASSERT(this, "Sprite::rewrite: null this");

	if(this->texture)
	{
		// write it in graphical memory
		__VIRTUAL_CALL(Texture, rewrite, this->texture);
	}
}

/**
 * Process event
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param eventFirer
 */
void Sprite_onTextureRewritten(Sprite this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::onTextureRewritten: null this");

	__VIRTUAL_CALL(Sprite, applyAffineTransformations, this);
	__VIRTUAL_CALL(Sprite, applyHbiasTransformations, this);
}

/**
 * Get displacement
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
VBVecWorld Sprite_getDisplacement(Sprite this)
{
	ASSERT(this, "Sprite::getDisplacement: null this");

	return this->displacement;
}

/**
 * Rotate
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 * @param rotation	Rotation struct
 */
void Sprite_rotate(Sprite this __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::rotate: null this");
}

/**
 * Get half width
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
int Sprite_getHalfWidth(Sprite this)
{
	ASSERT(this, "Sprite::getHalfWidth: null this");

	return FIX19_13TOI(this->halfWidth);
}

/**
 * Get half height
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
int Sprite_getHalfHeight(Sprite this)
{
	ASSERT(this, "Sprite::getHalfHeight: null this");

	return FIX19_13TOI(this->halfHeight);
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Animation)
//---------------------------------------------------------------------------------------------------------

/**
 * Update
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
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

	if(this->transparent)
	{
		this->visible = !this->visible;
	}
}

/**
 * Is Sprite transparent?
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether Sprite is transparent
 */
bool Sprite_isTransparent(Sprite this)
{
	ASSERT(this, "Sprite::isTransparent: null this");

	return this->transparent;
}

/**
 * Set Sprite transparent
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 * @param value	Boolean
 */
void Sprite_setTransparent(Sprite this, bool value)
{
	ASSERT(this, "Sprite::setTransparent: null this");

	this->transparent = value;

	if(!value)
	{
		this->visible = true;
	}
}

/**
 * Animate the Sprite
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_updateAnimation(Sprite this)
{
	ASSERT(this, "Sprite::updateAnimation: null this");

	if(this->animationController)
	{
		// first animate the frame
		this->writeAnimationFrame |= AnimationController_updateAnimation(this->animationController);
	}
}

/**
 * Pause animation
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 * @param pause	Boolean
 */
void Sprite_pause(Sprite this, bool pause)
{
	ASSERT(this, "Sprite::pause: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_pause(this->animationController, pause);
	}
}

/**
 * Play a given animation
 *
 * @memberof					Sprite
 * @public
 *
 * @param this					Function scope
 * @param animationDescription	AnimationDescription
 * @param functionName			Name of animation function to play
 */
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

/**
 * Is Sprite playing an animation?
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Boolean whether Sprite is playing an animation
 */
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

/**
 * Is Sprite playing a function?
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param functionName	Name of function to play
 *
 * @return				Boolean whether Sprite is playing a function
 */
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

/**
 * Set frame cycle decrement
 *
 * @memberof					Sprite
 * @public
 *
 * @param this					Function scope
 * @param frameCycleDecrement	Frame cycle decrement
 */
void Sprite_setFrameCycleDecrement(Sprite this, u8 frameCycleDecrement)
{
	ASSERT(this, "Sprite::setFrameCycleDecrement: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setFrameCycleDecrement(this->animationController, frameCycleDecrement);
	}
}

/**
 * Get actual frame
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Frame number
 */
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

/**
 * Set actual frame
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param actualFrame	Frame number
 */
void Sprite_setActualFrame(Sprite this, s8 actualFrame)
{
	ASSERT(this, "Sprite::setActualFrame: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setActualFrame(this->animationController, actualFrame);
	}
}

/**
 * Get frame delay
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Frame delay
 */
s8 Sprite_getFrameDuration(Sprite this)
{
	ASSERT(this, "Sprite::getFrameDuration: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController_getFrameDuration(this->animationController);
	}

	return -1;
}

/**
 * Set frame delay
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param frameDuration	Frame delay
 */
void Sprite_setFrameDuration(Sprite this, u8 frameDuration)
{
	ASSERT(this, "Sprite::setFrameDuration: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController_setFrameDuration(this->animationController, frameDuration);
	}
}

/**
 * Write animation
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_writeAnimation(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::writeAnimation: null this");
}

/**
 * Check if uses affine mode
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		True if it does
 */
bool Sprite_isAffine(Sprite this)
{
	ASSERT(this, "Sprite::isAffine: null this");

	return __WORLD_AFFINE == (this->head & __WORLD_AFFINE);
}

/**
 * Check if uses h-bias mode
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		True if it does
 */
bool Sprite_isHBias(Sprite this)
{
	ASSERT(this, "Sprite::isHBias: null this");

	return __WORLD_HBIAS == (this->head & __WORLD_HBIAS);
}

/**
 * Check if uses OBJECT mode
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		True if it does
 */
bool Sprite_isObject(Sprite this)
{
	ASSERT(this, "Sprite::isObject: null this");

	return __WORLD_OBJECT == (this->head & __WORLD_OBJECT);
}

/**
 * Print status
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 * @param x			Screen's x coordinate
 * @param y			Screen's y coordinate
 */
void Sprite_print(Sprite this, int x, int y)
{
	ASSERT(this, "Sprite::print: null this");

	Printing_text(Printing_getInstance(), "Layer: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->worldLayer, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "Class: ", x, ++y, NULL);
	Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "Head:                         ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), Sprite_getWorldHead(this), x + 14, y, 8, NULL);
	Printing_text(Printing_getInstance(), "Mode:", x, ++y, NULL);

	if(Sprite_isObject(this))
	{
		Printing_text(Printing_getInstance(), "OBJECT   ", x + 14, y, NULL);
	}
	else if(Sprite_isAffine(this))
	{
		Printing_text(Printing_getInstance(), "Affine   ", x + 14, y, NULL);
	}
	else if(Sprite_isHBias(this))
	{
		Printing_text(Printing_getInstance(), "H-bias   ", x + 14, y, NULL);
	}
	else
	{
		Printing_text(Printing_getInstance(), "BGMAP    ", x + 14, y, NULL);
	}

	Printing_text(Printing_getInstance(), "Transparent:                         ", x, ++y, NULL);
	Printing_text(Printing_getInstance(), Sprite_isTransparent(this) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 14, y, NULL);

	Printing_text(Printing_getInstance(), "Position:                         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, this).x), x + 14, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, this).y), x + 24, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(__VIRTUAL_CALL(Sprite, getPosition, this).z + Sprite_getDisplacement(this).z), x + 34, y, NULL);
	Printing_text(Printing_getInstance(), "WORLD (x, y):                         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), Sprite_getWorldX(this), x + 14, y, NULL);
	Printing_int(Printing_getInstance(), Sprite_getWorldY(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Size (w, h):                         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), Sprite_getWorldWidth(this), x + 14, y, NULL);
	Printing_int(Printing_getInstance(), Sprite_getWorldHeight(this), x + 24, y, NULL);

	if(Sprite_getTexture(this) && __GET_CAST(BgmapTexture, Sprite_getTexture(this)))
	{
		BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite_getTexture(this));

		Printing_text(Printing_getInstance(), "Texture (segment):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), BgmapTexture_getBgmapSegment(bgmapTexture), x + 24, y, NULL);
		Printing_text(Printing_getInstance(), "Texture (definition):                         ", x, ++y, NULL);
		Printing_hex(Printing_getInstance(), (int)Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture)), x + 24, y, 8, NULL);
		Printing_text(Printing_getInstance(), "Texture (written):                         ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), Texture_isWritten(__SAFE_CAST(Texture, bgmapTexture)) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 24, y, NULL);
		Printing_text(Printing_getInstance(), "Texture (rows rem.):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), BgmapTexture_getRemainingRowsToBeWritten(bgmapTexture), x + 24, y, NULL);
	}
}
//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Direct Draw)
//---------------------------------------------------------------------------------------------------------

/**
 * Write a char directly to the Sprite's Texture
 *
 * @memberof				Sprite
 * @public
 *
 * @param this				Function scope
 * @param texturePixel		Point that defines the position of the char in the Sprite's texture
 * @param newChar			Char to write
 */
void Sprite_putChar(Sprite this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Sprite::putChar: null this");

	if(this->texture && newChar && texturePixel)
	{
		Texture_putChar(this->texture, texturePixel, newChar);
	}
}

/**
 * Write a single pixel directly to the Sprite's Texture
 *
 * @memberof				Sprite
 * @public
 *
 * @param this				Function scope
 * @param texturePixel		Point that defines the position of the char in the Sprite's texture
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void Sprite_putPixel(Sprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Sprite::putPixel: null this");

	if(this->texture)
	{
		Texture_putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
	}
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Affine & HBias FX)
//---------------------------------------------------------------------------------------------------------

/**
 * Apply Affine transformations to Sprite
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_applyAffineTransformations(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyAffineTransformations: null this");
}

/**
 * Apply HBias transformations to Sprite
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite_applyHbiasTransformations(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyHbiasTransformations: null this");
}
