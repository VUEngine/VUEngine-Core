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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <AnimationController.h>
#include <Camera.h>
#include <VIPManager.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <BgmapTexture.h>
#include <TimerManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Texture;


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (General)
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param spriteSpec	Spec of the Sprite
 * @param owner				Entity the Sprite belongs to
 */
void Sprite::constructor(const SpriteSpec* spriteSpec __attribute__ ((unused)), Object owner __attribute__ ((unused)))
{
	Base::constructor();

	// clear values
	this->index = __NO_RENDER_INDEX;
	this->head = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->position = (PixelVector){0, 0, 0, 0};
	this->displacement = (PixelVector){0, 0, 0, 0};
	this->hidden = false;
	this->transparent = spriteSpec ? spriteSpec->transparent : __TRANSPARENCY_NONE;
	this->visible = true;
	this->writeAnimationFrame = false;
	this->positioned = false;
	this->registered = false;
}

/**
 * Class destructor
 */
void Sprite::destructor()
{
	ASSERT(this, "Sprite::destructor: null cast");

	if(!isDeleted(this->animationController))
	{
		delete this->animationController;
		this->animationController = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Sprite::processEffects()
{
}

s16 Sprite::render(s16 index, bool evenFrame)
{
	this->index = __NO_RENDER_INDEX;
	this->visible = false;

	// If the client code makes these checks before calling this method,
	// it saves on method calls quite a bit when there are lots of
	// sprites. Don't uncomment.
/*
	if(this->hidden || !this->positioned)
	{
		return this->index;
	}
*/
	if(isDeleted(this->texture))
	{
		this->index = Sprite::doRender(this, index, evenFrame);

		this->visible = __NO_RENDER_INDEX != this->index;

		return this->index;
	}

	if(kTextureInvalid == this->texture->status)
	{
		return this->index;
	}

	if(kTextureWritten != this->texture->status)
	{
		if(!Texture::prepare(this->texture))
		{
			return this->index;
		}
	}

	// If the client code makes these checks before calling this method,
	// it saves on method calls quite a bit when there are lots of
	// sprites. Don't uncomment.
/*
	if(!(((this->transparent == __TRANSPARENCY_NONE) || (0x01 & (this->transparent ^ evenFrame))) && Sprite::isWithinScreenSpace(this)))
	{
		return this->index;
	}
*/
	if(!Sprite::isWithinScreenSpace(this))
	{
		return this->index;
	}

	this->index = Sprite::doRender(this, index, evenFrame);

	this->visible = __NO_RENDER_INDEX != this->index;

	if(this->visible && this->writeAnimationFrame)
	{
		Sprite::update(this);
	}

	return this->index;
}



/**
 * Add displacement to position
 *
 * @memberof				Sprite
 * @public
 *
 * @param displacement		2D position displacement
 */
void Sprite::addDisplacement(const PixelVector* displacement)
{
	this->position.x += displacement->x;
	this->position.y += displacement->y;
	this->position.z += displacement->z;
	this->position.parallax += displacement->parallax;
}

/**
 * Get scale
 *
 * @return		Scale struct
 */
Scale Sprite::getScale()
{
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
 * @param scale	Scale struct to apply
 * @param z
 */
void Sprite::resize(Scale scale __attribute__ ((unused)), fix10_6 z __attribute__ ((unused)))
{
	this->halfWidth = Texture::getCols(this->texture) << 2;
	this->halfHeight = Texture::getRows(this->texture) << 2;
}

/**
 * Retrieve the texture
 *
 * @return 		Texture struct
 */
Texture Sprite::getTexture()
{
	return this->texture;
}

/**
 * Show
 */
void Sprite::show()
{
	this->hidden = false;
}

/**
 * Hide
 */
void Sprite::hide()
{
	this->hidden = true;
}

/**
 * Is the Sprite hidden?
 *
 * @return		Boolean telling whether the sprite is hidden
 */
bool Sprite::isHidden()
{
	return this->hidden;
}

/**
 * Register
 *
 * @param position		
 */
void Sprite::registerWithManager()
{
	this->registered = true;
}

/**
 * Calculate 2D position
 *
 * @param position		3D position
 */
void Sprite::position(const Vector3D* position)
{
	this->position = Vector3D::projectRelativeToPixelVector(*position, this->position.parallax);
	this->positioned = true;

	if(!this->registered)
	{
		Sprite::registerWithManager(this);
	}
}

/**
 * Set position
 *
 * @param position		Pixel position
 */
void Sprite::setPosition(const PixelVector* position)
{
	this->position = *position;
	this->positioned = true;

	if(!this->registered)
	{
		Sprite::registerWithManager(this);
	}
}

/**
 * Calculate parallax
 *
 * @param z				Z coordinate to base on the calculation
 */
void Sprite::calculateParallax(fix10_6 z __attribute__ ((unused)))
{
	this->position.parallax = Optics::calculateParallax(__PIXELS_TO_METERS(this->position.x), z);
}

/**
 * Get position relative to the camera
 *
 * @return			Position relative to camera
 */
const PixelVector* Sprite::getPosition()
{
	return (const PixelVector*)&this->position;
}

/**
 * Get displaced position relative to the camera
 *
 * @return			Displaced position relative to camera
 */
PixelVector Sprite::getDisplacedPosition()
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
 * @private
 * @return		Sprite's AnimationController
 */
AnimationController Sprite::getAnimationController()
{
	return this->animationController;
}

/**
 * Get WORLD layer
 *
 * @return 		World layer
 */
u8 Sprite::getIndex()
{
	return this->index;
}

/**
 * Write textures
 *
 * @return			true it all textures are written
 */
bool Sprite::writeTextures()
{
	if(isDeleted(this->texture))
	{
		return true;
	}

	if(kTexturePendingWriting == this->texture->status)
	{
		Texture::write(this->texture);
	}

	return kTexturePendingWriting != this->texture->status;
}

/**
 * Get sprite's render head
 *
 * @return
 */
u16 Sprite::getHead()
{
	return this->head;
}

/**
 * Get sprite's render mode
 *
 * @return 		Mode
 */
u16 Sprite::getMode()
{
	return this->head & 0x3000;
}

/**
 * Get sprites's world head
 *
 * @return
 */
u32 Sprite::getWorldHead()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->head;
}

/**
 * Get sprites's layer's gx
 *
 * @return
 */
s16 Sprite::getWorldGX()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gx;
}

/**
 * Get sprites's layer's gy
 *
 * @return
 */
s16 Sprite::getWorldGY()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gy;
}

/**
 * Get sprites's layer's gp
 *
 * @return
 */
s16 Sprite::getWorldGP()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gp;
}

/**
 * Get sprites's layer's mx
 *
 * @return
 */
s16 Sprite::getWorldMX()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mx;
}

/**
 * Get sprites's layer's my
 *
 * @return
 */
s16 Sprite::getWorldMY()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->my;
}

/**
 * Get sprites's layer's mp
 *
 * @return
 */
s16 Sprite::getWorldMP()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mp;
}

/**
 * Get sprites's layer's width
 *
 * @return 		Width
 */
u16 Sprite::getWorldWidth()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (s16)worldPointer->w ? 0 : worldPointer->w;
}

/**
 * Get sprites's layer's height
 *
 * @return 		Width
 */
u16 Sprite::getWorldHeight()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (s16)worldPointer->h ? 0 : worldPointer->h;
}

/**
 * Reload the sprite in BGMap memory
 */
void Sprite::rewrite()
{
	if(this->texture)
	{
		// write it in graphical memory
		Texture::rewrite(this->texture);
	}
}

/**
 * Get displacement
 *
 * @return
 */
const PixelVector* Sprite::getDisplacement()
{
	return &this->displacement;
}

/**
 * Set displacement
 *
 * @param displacement 	PixelVector
 */
void Sprite::setDisplacement(const PixelVector* displacement)
{
	this->displacement = *displacement;
}

/**
 * Rotate
 *
 * @param rotation	Rotation struct
 */
void Sprite::rotate(const Rotation* rotation __attribute__ ((unused)))
{}

/**
 * Get half width
 *
 * @return
 */
int Sprite::getHalfWidth()
{
	return this->halfWidth;
}

/**
 * Get half height
 *
 * @return
 */
int Sprite::getHalfHeight()
{
	return this->halfHeight;
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Animation)
//---------------------------------------------------------------------------------------------------------

/**
 * Update
 */
void Sprite::update()
{
	if(!isDeleted(this->animationController))
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
 * Is visible?
 *
 * @return visibility
 */
bool Sprite::isVisible()
{
	return this->visible && !this->hidden;
}

/**
 * Is within the screen space
 *
 * @return visibility
 */
bool Sprite::isWithinScreenSpace()
{
	if(!((unsigned)(this->position.x + this->displacement.x - (_cameraFrustum->x0 - this->halfWidth)) < (unsigned)(_cameraFrustum->x1 + this->halfWidth - (_cameraFrustum->x0 - this->halfWidth))))
	{
		return false;
	}

	if(!((unsigned)(this->position.y + this->displacement.y - (_cameraFrustum->y0 - this->halfHeight)) < (unsigned)(_cameraFrustum->y1 + this->halfHeight - (_cameraFrustum->y0 - this->halfHeight))))
	{
		return false;
	}
/*
	if(!((unsigned)(this->position.z + this->displacement.z - _cameraFrustum->z0) < (unsigned)(_cameraFrustum->z1 - _cameraFrustum->z0)))
	{
		return false;
	}
*/
	return true;
}

/**
 * Get Sprite's transparency mode
 *
 * @return		Transparency mode
 */
u8 Sprite::getTransparent()
{
	return this->transparent;
}

/**
 * Set Sprite transparent
 *
 * @param value	Transparency mode
 */
void Sprite::setTransparent(u8 value)
{
	this->transparent = value;
	this->visible = true;
}

/**
 * Animate the Sprite
 */
void Sprite::updateAnimation()
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		this->writeAnimationFrame |= AnimationController::updateAnimation(this->animationController);
	}
}

/**
 * Pause animation
 *
 * @param pause	Boolean
 */
void Sprite::pause(bool pause)
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		AnimationController::pause(this->animationController, pause);
		this->writeAnimationFrame |= !pause;
	}
}

/**
 * Play a given animation
 *
 * @param animationDescription	AnimationDescription
 * @param functionName			Name of animation function to play
 */
void Sprite::play(const AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(animationDescription, "Sprite::play: null animationDescription");
	ASSERT(functionName, "Sprite::play: null functionName");

	if(!isDeleted(this->animationController))
	{
		this->writeAnimationFrame |= AnimationController::play(this->animationController, animationDescription, functionName);
	}
}

/**
 * Stop any playing animation
 *
 */
void Sprite::stop()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::stop(this->animationController);
	}
}

/**
 * Replay the last animation
 *
 * @param animationDescription	AnimationDescription
 */
bool Sprite::replay(const AnimationDescription* animationDescription)
{
	ASSERT(animationDescription, "Sprite::replay: null animationDescription");

	if(!isDeleted(this->animationController))
	{
		this->writeAnimationFrame |= AnimationController::replay(this->animationController, animationDescription);

		return this->writeAnimationFrame;
	}

	return false;
}

/**
 * Is Sprite playing an animation?
 *
 * @return		Boolean whether Sprite is playing an animation
 */
bool Sprite::isPlaying()
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		return AnimationController::isPlaying(this->animationController);
	}

	return false;
}

/**
 * Is Sprite playing a function?
 *
 * @param functionName	Name of function to play
 * @return				Boolean whether Sprite is playing a function
 */
bool Sprite::isPlayingFunction(char* functionName)
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::isPlayingFunction(this->animationController, functionName);
	}

	return false;
}

/**
 * Set frame cycle decrement
 *
 * @param frameCycleDecrement	Frame cycle decrement
 */
void Sprite::setFrameCycleDecrement(u8 frameCycleDecrement)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameCycleDecrement(this->animationController, frameCycleDecrement);
	}
}

/**
 * Get actual frame
 *
 * @return		Frame number
 */
s16 Sprite::getActualFrame()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getActualFrame(this->animationController);
	}

	return -1;
}

/**
 * Set actual frame
 *
 * @param actualFrame	Frame number
 */
void Sprite::setActualFrame(s16 actualFrame)
{
	if(!isDeleted(this->animationController))
	{
		this->writeAnimationFrame |= AnimationController::setActualFrame(this->animationController, actualFrame);
	}
	else if(!isDeleted(this->texture))
	{
		Texture::setFrame(this->texture, actualFrame);
	}
}

/**
 * Skip to next frame
 */
void Sprite::nextFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::nextFrame(this->animationController);
		this->writeAnimationFrame = true;
	}
}

/**
 * Rewind to previous frame
 */
void Sprite::previousFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::previousFrame(this->animationController);
		this->writeAnimationFrame = true;
	}
}

/**
 * Get frame delay
 *
 * @return		Frame delay
 */
u8 Sprite::getFrameDuration()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getFrameDuration(this->animationController);
	}

	return -1;
}

/**
 * Set frame delay
 *
 * @param frameDuration	Frame delay
 */
void Sprite::setFrameDuration(u8 frameDuration)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameDuration(this->animationController, frameDuration);
	}
}

/**
 * Write animation
 */
void Sprite::writeAnimation()
{}

/**
 * Check if uses affine mode
 *
 * @return		True if it does
 */
bool Sprite::isAffine()
{
	return __WORLD_AFFINE == (this->head & __WORLD_AFFINE);
}

/**
 * Check if uses h-bias mode
 *
 * @return		True if it does
 */
bool Sprite::isHBias()
{
	return __WORLD_HBIAS == (this->head & __WORLD_HBIAS);
}

/**
 * Check if uses OBJECT mode
 *
 * @return		True if it does
 */
bool Sprite::isObject()
{
	return __WORLD_OBJECT == (this->head & __WORLD_OBJECT);
}

/**
 * Print status
 *
 * @param x			Camera's x coordinate
 * @param y			Camera's y coordinate
 */
void Sprite::print(int x, int y)
{
	// Allow normal rendering once for WORLD values to populate properly
	u8 transparent = this->transparent;
	this->transparent = __TRANSPARENCY_NONE;

	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), SpriteManager::getSpritePosition(SpriteManager::getInstance(), this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getWorldHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);

	if(Sprite::isObject(this))
	{
		Printing::text(Printing::getInstance(), "OBJECT   ", x + 18, y, NULL);
	}
	else if(Sprite::isAffine(this))
	{
		Printing::text(Printing::getInstance(), "AFFINE   ", x + 18, y, NULL);
	}
	else if(Sprite::isHBias(this))
	{
		Printing::text(Printing::getInstance(), "H-BIAS   ", x + 18, y, NULL);
	}
	else
	{
		Printing::text(Printing::getInstance(), "BGMAP    ", x + 18, y, NULL);
	}

	Printing::text(Printing::getInstance(), "Transparent:                         ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), (transparent > 0) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), (transparent == 1) ? "(Even)" : (transparent == 2) ? "(Odd)" : "", x + 20, y, NULL);
	Printing::text(Printing::getInstance(), "Hidden:                         ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), (this->hidden > 0) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);

	Printing::text(Printing::getInstance(), "Pos. (x,y,z,p):                      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->position.x, x + 18, y, NULL);
	Printing::int(Printing::getInstance(), this->position.y, x + 24, y, NULL);
	Printing::int(Printing::getInstance(), this->position.z + Sprite::getDisplacement(this)->z, x + 30, y, NULL);
	Printing::int(Printing::getInstance(), this->position.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "Displ. (x,y,z,p):                    ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.x, x + 18, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.y, x + 24, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.z, x + 30, y, NULL);
	Printing::int(Printing::getInstance(), this->displacement.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "G (x,y,p):                           ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGX(this), x + 18, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGY(this), x + 24, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldGP(this), x + 30, y, NULL);
	Printing::text(Printing::getInstance(), "M (x,y,p):                           ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMX(this), x + 18, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMY(this), x + 24, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldMP(this), x + 30, y, NULL);
	Printing::text(Printing::getInstance(), "Size (w,h):                          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldWidth(this), x + 18, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getWorldHeight(this), x + 24, y++, NULL);
	Printing::text(Printing::getInstance(), "Pixels:                      ", x, y, NULL);
	Printing::int(Printing::getInstance(), Sprite::getTotalPixels(this), x + 18, y++, NULL);

	if(Sprite::getTexture(this) && __GET_CAST(BgmapTexture, Sprite::getTexture(this)))
	{
		BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite::getTexture(this));

		Printing::text(Printing::getInstance(), "TEXTURE                          ", x, ++y, NULL);
		y++;
		Printing::text(Printing::getInstance(), "Segment:                         ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), BgmapTexture::getSegment(bgmapTexture), x + 18, y, NULL);
		Printing::text(Printing::getInstance(), "Spec:                      ", x, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int)Texture::getTextureSpec(bgmapTexture), x + 18, y, 8, NULL);
		Printing::text(Printing::getInstance(), "Written:                         ", x, ++y, NULL);
		Printing::text(Printing::getInstance(), Texture::isWritten(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
		Printing::text(Printing::getInstance(), "Rows remaining:                  ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), BgmapTexture::getRemainingRowsToBeWritten(bgmapTexture), x + 18, y, NULL);
		Printing::text(Printing::getInstance(), "Size (w,h):                      ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), this->halfWidth * 2, x + 18, y, NULL);
		Printing::int(Printing::getInstance(), this->halfHeight * 2, x + 24, y, NULL);
	}

	this->transparent = transparent;
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Direct Draw)
//---------------------------------------------------------------------------------------------------------

/**
 * Write a char directly to the Sprite's Texture
 *
 * @param texturePixel		Point that defines the position of the char in the Sprite's texture
 * @param newChar			Char to write
 */
void Sprite::putChar(Point* texturePixel, BYTE* newChar)
{
	if(this->texture && newChar && texturePixel)
	{
		Texture::putChar(this->texture, texturePixel, newChar);
	}
}

/**
 * Write a single pixel directly to the Sprite's Texture
 *
 * @param texturePixel		Point that defines the position of the char in the Sprite's texture
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void Sprite::putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor)
{
	if(this->texture)
	{
		Texture::putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
	}
}

/**
 * Get the total amount of pixels displayed by the sprite
 *
 * @return		Total pixels
 */
int Sprite::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return Sprite::getWorldWidth(this) * Sprite::getWorldHeight(this);
	}

	return 0;
}


//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (Affine & HBias FX)
//---------------------------------------------------------------------------------------------------------

/**
 * Apply Affine transformations to Sprite
 */
void Sprite::applyAffineTransformations()
{}

/**
 * Apply HBias effects to Sprite
 */
void Sprite::applyHbiasEffects()
{}
