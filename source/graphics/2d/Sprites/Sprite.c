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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <AnimationController.h>
#include <Camera.h>
#include <VIPManager.h>
#include <BgmapTexture.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	Sprite
 * @extends Object
 * @ingroup graphics-2d-sprites
 */
implements Sprite : Object;
friend class Texture;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Sprite::onTextureRewritten(Sprite this, Object eventFirer);


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
void Sprite::constructor(Sprite this, const SpriteDefinition* spriteDefinition __attribute__ ((unused)), Object owner __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::destructor: null this");

	Base::constructor();

	// clear values
	this->worldLayer = 0;
	this->head = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->position = (PixelVector){0, 0, 0, 0};
	this->displacement = (PixelVector){0, 0, 0, 0};
	this->hidden = false;
	this->transparent = spriteDefinition ? spriteDefinition->transparent : __TRANSPARENCY_NONE;
	this->visible = true;
	this->writeAnimationFrame = false;
	this->positioned = false;
}

/**
 * Class destructor
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite::destructor(Sprite this)
{
	ASSERT(this, "Sprite::destructor: null cast");

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
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
Scale Sprite::getScale(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::getScale: null this");

	Scale scale =
	{
		__1I_FIX7_9,
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
void Sprite::resize(Sprite this, Scale scale __attribute__ ((unused)), fix10_6 z __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::resize: null this");

	this->halfWidth = Texture::getCols(this->texture) << 2;
	this->halfHeight = Texture::getRows(this->texture) << 2;
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
Texture Sprite::getTexture(Sprite this)
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
void Sprite::show(Sprite this)
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
void Sprite::hide(Sprite this)
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
bool Sprite::isHidden(Sprite this)
{
	ASSERT(this, "Sprite::isHidden: null this");

	return this->hidden;
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
void Sprite::position(Sprite this __attribute__ ((unused)), const Vector3D* position)
{
	ASSERT(this, "Sprite::position: null this");

	this->position = Vector3D::projectToPixelVector(Vector3D::getRelativeToCamera(*position), this->position.parallax);

	this->positioned = true;
}

/**
 * Set position
 *
 * @memberof			Sprite
 * @public
 *
 * @param this			Function scope
 * @param position		Pixel position
 */
void Sprite::setPosition(Sprite this, const PixelVector* position)
{
	ASSERT(this, "Sprite::setPosition: null this");

	this->positioned = true;

	this->position = *position;
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
void Sprite::calculateParallax(Sprite this __attribute__ ((unused)), fix10_6 z __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::calculateParallax: null this");
}

/**
 * Get position relative to the camera
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			Position relative to camera
 */
PixelVector Sprite::getPosition(Sprite this)
{
	return this->position;
}


/**
 * Get displaced position relative to the camera
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			Displaced position relative to camera
 */
PixelVector Sprite::getDisplacedPosition(Sprite this)
{
	PixelVector position =
	{
		this->position.x + this->displacement.x,
		this->position.y + this->displacement.y,
		this->position.z + this->displacement.z,
		this->position.parallax + this->displacement.parallax
	};

	return position;
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
AnimationController Sprite::getAnimationController(Sprite this)
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
void Sprite::setWorldLayer(Sprite this, u8 worldLayer)
{
	ASSERT(this, "Sprite::setWorldLayer: null this");

	this->worldLayer = worldLayer;

	if(0 <= (s8)this->worldLayer)
	{
		_worldAttributesBaseAddress[this->worldLayer].head &= ~__WORLD_END;
	}
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
u8 Sprite::getWorldLayer(Sprite this)
{
	ASSERT(this, "Sprite::getWorldLayer: null this");

	return this->worldLayer;
}

/**
 * Write textures
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			true it all textures are written
 */
bool Sprite::writeTextures(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::writeTextures: null this");

	if(!this->texture)
	{
		return true;
	}

	if(!this->texture->written)
	{
		 Texture::write(this->texture);
	}

	return this->texture->written;
}

/**
 * Check if all textures are written
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			true it all textures are written
 */
bool Sprite::areTexturesWritten(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::areTexturesWritten: null this");

	return !this->texture ? true : this->texture->written;
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
u16 Sprite::getHead(Sprite this)
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
u16 Sprite::getMode(Sprite this)
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
u32 Sprite::getWorldHead(Sprite this)
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
s16 Sprite::getWorldGX(Sprite this)
{
	ASSERT(this, "Sprite::getWorldGX: null this");

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
s16 Sprite::getWorldGY(Sprite this)
{
	ASSERT(this, "Sprite::getWorldGY: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gy;
}

/**
 * Get sprites's layer's gp
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite::getWorldGP(Sprite this)
{
	ASSERT(this, "Sprite::getWorldGP: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->gp;
}

/**
 * Get sprites's layer's mx
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite::getWorldMX(Sprite this)
{
	ASSERT(this, "Sprite::getWorldMX: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->mx;
}

/**
 * Get sprites's layer's my
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite::getWorldMY(Sprite this)
{
	ASSERT(this, "Sprite::getWorldMY: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->my;
}

/**
 * Get sprites's layer's mp
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return
 */
s16 Sprite::getWorldMP(Sprite this)
{
	ASSERT(this, "Sprite::getWorldMP: null this");

	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[this->worldLayer];
	return worldPointer->mp;
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
u16 Sprite::getWorldWidth(Sprite this)
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
u16 Sprite::getWorldHeight(Sprite this)
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
void Sprite::rewrite(Sprite this)
{
	ASSERT(this, "Sprite::rewrite: null this");

	if(this->texture)
	{
		// write it in graphical memory
		 Texture::rewrite(this->texture);
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
void Sprite::onTextureRewritten(Sprite this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::onTextureRewritten: null this");

	 Sprite::applyAffineTransformations(this);
	 Sprite::applyHbiasEffects(this);
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
PixelVector Sprite::getDisplacement(Sprite this)
{
	ASSERT(this, "Sprite::getDisplacement: null this");

	return this->displacement;
}

/**
 * Set displacement
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 * @param displacement 	PixelVector
 */
void Sprite::setDisplacement(Sprite this, PixelVector displacement)
{
	ASSERT(this, "Sprite::setDisplacement: null this");

	this->displacement = displacement;
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
void Sprite::rotate(Sprite this __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)))
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
int Sprite::getHalfWidth(Sprite this)
{
	ASSERT(this, "Sprite::getHalfWidth: null this");

	return this->halfWidth;
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
int Sprite::getHalfHeight(Sprite this)
{
	ASSERT(this, "Sprite::getHalfHeight: null this");

	return this->halfHeight;
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Animation)
//---------------------------------------------------------------------------------------------------------

/**
 * Update
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 */
void Sprite::update(Sprite this)
{
	ASSERT(this, "Sprite::update: null this");

	if(this->animationController && this->texture)
	{
		// first animate the frame
		if(this->writeAnimationFrame)
		{
			 Sprite::writeAnimation(this);
			this->writeAnimationFrame = false;
		}
	}
}

/**
 * Render
 *
 * @memberof		Sprite
 * @public
 *
 * @param this		Function scope
 * @param evenFrame
 */
void Sprite::render(Sprite this, bool evenFrame)
{
	ASSERT(this, "Sprite::update: null this");

	this->visible = (this->transparent == __TRANSPARENCY_NONE) ||
					((this->transparent == __TRANSPARENCY_EVEN) && evenFrame) ||
					((this->transparent == __TRANSPARENCY_ODD) && !evenFrame);
}

/**
 * Get Sprite's transparency mode
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 *
 * @return		Transparency mode
 */
u8 Sprite::getTransparent(Sprite this)
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
 * @param value	Transparency mode
 */
void Sprite::setTransparent(Sprite this, u8 value)
{
	ASSERT(this, "Sprite::setTransparent: null this");

	this->transparent = value;
	this->visible = true;
}

/**
 * Animate the Sprite
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite::updateAnimation(Sprite this)
{
	ASSERT(this, "Sprite::updateAnimation: null this");

	if(this->animationController)
	{
		// first animate the frame
		this->writeAnimationFrame |= AnimationController::updateAnimation(this->animationController);
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
void Sprite::pause(Sprite this, bool pause)
{
	ASSERT(this, "Sprite::pause: null this");

	if(this->animationController)
	{
		// first animate the frame
		AnimationController::pause(this->animationController, pause);
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
void Sprite::play(Sprite this, AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(this, "Sprite::play: null this");
	ASSERT(animationDescription, "Sprite::play: null animationDescription");
	ASSERT(functionName, "Sprite::play: null functionName");

	if(this->animationController)
	{
		this->writeAnimationFrame = AnimationController::play(this->animationController, animationDescription, functionName);
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
bool Sprite::isPlaying(Sprite this)
{
	ASSERT(this, "Sprite::isPlaying: null this");

	if(this->animationController)
	{
		// first animate the frame
		return AnimationController::isPlaying(this->animationController);
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
bool Sprite::isPlayingFunction(Sprite this, char* functionName)
{
	ASSERT(this, "Sprite::isPlayingFunction: null this");

	if(this->animationController)
	{
		return AnimationController::isPlayingFunction(this->animationController, functionName);
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
void Sprite::setFrameCycleDecrement(Sprite this, u8 frameCycleDecrement)
{
	ASSERT(this, "Sprite::setFrameCycleDecrement: null this");

	if(this->animationController)
	{
		AnimationController::setFrameCycleDecrement(this->animationController, frameCycleDecrement);
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
s8 Sprite::getActualFrame(Sprite this)
{
	ASSERT(this, "Sprite::getActualFrame: null this");

	if(this->animationController)
	{
		return AnimationController::getActualFrame(this->animationController);
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
void Sprite::setActualFrame(Sprite this, s8 actualFrame)
{
	ASSERT(this, "Sprite::setActualFrame: null this");

	if(this->animationController)
	{
		AnimationController::setActualFrame(this->animationController, actualFrame);
	}
}

/**
 * Skip to next frame
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite::nextFrame(Sprite this)
{
	ASSERT(this, "Sprite::nextFrame: null this");

	if(this->animationController)
	{
		AnimationController::nextFrame(this->animationController);
		this->writeAnimationFrame = true;
	}
}

/**
 * Rewind to previous frame
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite::previousFrame(Sprite this)
{
	ASSERT(this, "Sprite::previousFrame: null this");

	if(this->animationController)
	{
		AnimationController::previousFrame(this->animationController);
		this->writeAnimationFrame = true;
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
s8 Sprite::getFrameDuration(Sprite this)
{
	ASSERT(this, "Sprite::getFrameDuration: null this");

	if(this->animationController)
	{
		return AnimationController::getFrameDuration(this->animationController);
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
void Sprite::setFrameDuration(Sprite this, u8 frameDuration)
{
	ASSERT(this, "Sprite::setFrameDuration: null this");

	if(this->animationController)
	{
		AnimationController::setFrameDuration(this->animationController, frameDuration);
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
void Sprite::writeAnimation(Sprite this __attribute__ ((unused)))
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
bool Sprite::isAffine(Sprite this)
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
bool Sprite::isHBias(Sprite this)
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
bool Sprite::isObject(Sprite this)
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
 * @param x			Camera's x coordinate
 * @param y			Camera's y coordinate
 */
void Sprite::print(Sprite this, int x, int y)
{
	ASSERT(this, "Sprite::print: null this");

	Printing::text(Printing::getInstance(), "Layer: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->worldLayer, x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getWorldHead(this), x + 14, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);

	if(Sprite::isObject(this))
	{
		Printing::text(Printing::getInstance(), "OBJECT   ", x + 14, y, NULL);
	}
	else if(Sprite::isAffine(this))
	{
		Printing::text(Printing::getInstance(), "Affine   ", x + 14, y, NULL);
	}
	else if(Sprite::isHBias(this))
	{
		Printing::text(Printing::getInstance(), "H-bias   ", x + 14, y, NULL);
	}
	else
	{
		Printing::text(Printing::getInstance(), "BGMAP    ", x + 14, y, NULL);
	}

	Printing::text(Printing::getInstance(), "Transparent:                         ", x, ++y, NULL);
	u8 spriteTransparency = Sprite::getTransparent(this);
	Printing::text(Printing::getInstance(), (spriteTransparency > 0) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 14, y, NULL);
	Printing::text(Printing::getInstance(), (spriteTransparency == 1) ? "(Even)" : (spriteTransparency == 2) ? "(Odd)" : "", x + 16, y, NULL);

	Printing::text(Printing::getInstance(), "Position:                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->position.x, x + 14, y, NULL);
	Printing::int(Printing::getInstance(), this->position.y, x + 22, y, NULL);
	Printing::int(Printing::getInstance(), this->position.z + Sprite::getDisplacement(this).z, x + 30, y, NULL);
	Printing::int(Printing::getInstance(), this->position.parallax, x + 38, y, NULL);
	Printing::text(Printing::getInstance(), "Texture size:                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->halfWidth * 2, x + 14, y, NULL);
	Printing::int(Printing::getInstance(), this->halfHeight * 2, x + 22, y, NULL);
	Printing::text(Printing::getInstance(), "Displacement:                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.x, x + 14, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.y, x + 22, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.z, x + 30, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.parallax, x + 38, y, NULL);
	Printing::text(Printing::getInstance(), "G (x, y, p):                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGX(this), x + 14, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGY(this), x + 24, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGP(this), x + 34, y, NULL);
	Printing::text(Printing::getInstance(), "M (x, y, p):                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMX(this), x + 14, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMY(this), x + 24, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMP(this), x + 34, y, NULL);
	Printing::text(Printing::getInstance(), "Size (w, h):                         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldWidth(this), x + 14, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldHeight(this), x + 24, y, NULL);

	if(Sprite::getTexture(this) && __GET_CAST(BgmapTexture, Sprite::getTexture(this)))
	{
		BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite::getTexture(this));

		Printing::text(Printing::getInstance(), "Texture (segment):                         ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), BgmapTexture::getSegment(bgmapTexture), x + 24, y, NULL);
		Printing::text(Printing::getInstance(), "Texture (definition):                         ", x, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int)Texture::getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture)), x + 24, y, 8, NULL);
		Printing::text(Printing::getInstance(), "Texture (written):                         ", x, ++y, NULL);
		Printing::text(Printing::getInstance(), Texture::isWritten(__SAFE_CAST(Texture, bgmapTexture)) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 24, y, NULL);
		Printing::text(Printing::getInstance(), "Texture (rows rem.):                         ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), BgmapTexture::getRemainingRowsToBeWritten(bgmapTexture), x + 24, y, NULL);
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
void Sprite::putChar(Sprite this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Sprite::putChar: null this");

	if(this->texture && newChar && texturePixel)
	{
		Texture::putChar(this->texture, texturePixel, newChar);
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
void Sprite::putPixel(Sprite this, Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Sprite::putPixel: null this");

	if(this->texture)
	{
		Texture::putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
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
void Sprite::applyAffineTransformations(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyAffineTransformations: null this");
}

/**
 * Apply HBias effects to Sprite
 *
 * @memberof	Sprite
 * @public
 *
 * @param this	Function scope
 */
void Sprite::applyHbiasEffects(Sprite this __attribute__ ((unused)))
{
	ASSERT(this, "Sprite::applyHbiasEffects: null this");
}
