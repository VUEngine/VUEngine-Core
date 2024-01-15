/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <DebugUtilities.h>
#include <ObjectSprite.h>
#include <Optics.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <VIPManager.h>

#include "Sprite.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Texture;

#ifdef __RELEASE
friend class AnimationController;
#endif

//---------------------------------------------------------------------------------------------------------
//										CLASS'S METHODS (General)
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param spriteSpec	Spec of the Sprite
 * @param owner				Entity the Sprite belongs to
 */
void Sprite::constructor(SpatialObject owner, const SpriteSpec* spriteSpec)
{
	Base::constructor(owner, &spriteSpec);

	// clear values
	this->index = __NO_RENDER_INDEX;
	this->head = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->transparent = spriteSpec ? spriteSpec->transparent : __TRANSPARENCY_NONE;
	this->writeAnimationFrame = false;
	this->checkIfWithinScreenSpace = true;
	this->position = (PixelVector){0, 0, 0, 0};
	this->rotation = Rotation::zero();
	this->scale = Scale::unit();
	this->transformed = false;
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

void Sprite::createAnimationController(CharSetSpec* charSetSpec)
{
    this->animationController = new AnimationController();

	if(isDeleted(this->animationController))
	{
		this->animationController = NULL;
		return;
	}
	
	if(!isDeleted(this->texture) && Texture::isSingleFrame(this->texture))
	{
		AnimationController::setAnimationCoordinator(this->animationController,
			AnimationCoordinatorFactory::getCoordinator
			(
				AnimationCoordinatorFactory::getInstance(),
				this->animationController, 
				ListenerObject::safeCast(this->owner), 
				charSetSpec
			)
		);
	}
}

void Sprite::processEffects()
{
}

int16 Sprite::render(int16 index, bool evenFrame)
{
	// If the client code makes these checks before calling this method,
	// it saves on method calls quite a bit when there are lots of
	// sprites. Don't uncomment.
/*
	if(__HIDE == this->show)
	{
		return __NO_RENDER_INDEX;
	}
*/

	if(isDeleted(this->texture))
	{
		this->index = Sprite::doRender(this, index, evenFrame);
		return this->index;
	}

	if(kTextureInvalid == this->texture->status)
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	if(kTexturePendingWriting == this->texture->status)
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
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

	if(NULL != this->owner)
	{
		Sprite::position(this);
		Sprite::rotate(this);
		Sprite::scale(this);

		this->transformed = true;
	}

	// Do not remove this check, it prevents sprites from looping
	if(this->checkIfWithinScreenSpace && !Sprite::isWithinScreenSpace(this))
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	Sprite::update(this);

	if(!this->rendered || this->index != index)
	{
		this->rendered = true;

 		this->index = Sprite::doRender(this, index, evenFrame);

#ifdef __SHOW_SPRITES_PROFILING
		extern int32 _renderedSprites;
		_renderedSprites++;
#endif		
	}

	return this->index;
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
void Sprite::forceShow()
{
	this->show = __SHOW;

	Sprite::setPosition(this, &this->position);
}

/**
 * Hide
 */
void Sprite::hideForDebug()
{
	this->show = __HIDE;

	Sprite::setPosition(this, &this->position);
}

/**
 * Is the Sprite show?
 *
 * @return		Boolean telling whether the sprite is show
 */
bool Sprite::isHidden()
{
	return __HIDE == this->show;
}

/**
 * Calculate 2D position
 *
 * @param position		3D position
 */
void Sprite::position()
{
	if(NULL == this->transformation)
	{
		return;
	}

	PixelVector position = Vector3D::transformToPixelVector(this->transformation->position);

	if
	(
		!this->transformed 
		||
		this->position.x != position.x
		||
		this->position.y != position.y
		||
		this->position.z != position.z
	)
	{
		Sprite::setPosition(this, &position);
	}
}

/**
 * Set position
 *
 * @param position		Pixel position
 */
void Sprite::setPosition(const PixelVector* position)
{
	if(NULL == position)
	{
		return;
	}

	this->position = *position;
	this->rendered = false;
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
	this->rendered = false;
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
 * Rotate
 *
 * @param rotation	Rotation struct
 */
void Sprite::rotate()
{
	if(NULL == this->transformation)
	{
		return;
	}

	if
	(
		!this->transformed
		||
		this->rotation.x != this->transformation->rotation.x
		||
		this->rotation.y != this->transformation->rotation.y
		||
		this->rotation.z != this->transformation->rotation.z
	)
	{
		Sprite::setRotation(this, &this->transformation->rotation);
		this->rotation = this->transformation->rotation;
		this->rendered = false;
	}
}

void Sprite::setRotation(const Rotation* rotation __attribute__((unused)))
{
	this->rotation = *rotation;
	this->rendered = false;
}

const Rotation* Sprite::getRotation()
{
	return &this->transformation->rotation;
}

/**
 * Calculate zoom scaling factor
 *
 * @param scale	Scale struct to apply
 * @param z
 */
void Sprite::scale()
{
	if(NULL == this->transformation)
	{
		return;
	}

	if
	(
		!this->transformed
		||
		this->scale.x != this->transformation->scale.x
		||
		this->scale.y != this->transformation->scale.y
		||
		this->scale.z != this->transformation->scale.z
	)
	{
		this->scale = this->transformation->scale;
		this->rendered = false;

		if(Sprite::overrides(this, setScale))
		{
			Scale scale = this->scale;

			NM_ASSERT(0 < scale.x, "Sprite::scale: 0 scale x");
			NM_ASSERT(0 < scale.y, "Sprite::scale: 0 scale y");

			fix7_9 ratio = __FIXED_TO_FIX7_9(Vector3D::getScale(this->transformation->position.z, true));

			ratio = 0 > ratio? __1I_FIX7_9 : ratio;
			ratio = __I_TO_FIX7_9(__MAXIMUM_SCALE) < ratio? __I_TO_FIX7_9(__MAXIMUM_SCALE) : ratio;

			scale.x = __FIX7_9_MULT(scale.x, ratio);
			scale.y = __FIX7_9_MULT(scale.y, ratio);

			NM_ASSERT(0 < scale.x, "Sprite::scale: null scale x");
			NM_ASSERT(0 < scale.y, "Sprite::scale: null scale y");

			Sprite::setScale(this, &scale);
		}		
	}
}

void Sprite::setScale(const Scale* scale __attribute__((unused)))
{
	this->rendered = false;
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
 * Calculate parallax
 *
 * @param z				Z coordinate to base on the calculation
 */
void Sprite::calculateParallax(fixed_t z __attribute__ ((unused)))
{
	this->position.parallax = Optics::calculateParallax(z);
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
int16 Sprite::getIndex()
{
	return this->index;
}

/**
 * Write textures
 *
 * @return			true it all textures are written
 */
bool Sprite::writeTextures(int16 maximumTextureRowsToWrite)
{
	if(isDeleted(this->texture))
	{
		return true;
	}

	if(kTexturePendingWriting == this->texture->status)
	{
		Texture::write(this->texture, maximumTextureRowsToWrite);
	}

	return kTexturePendingWriting != this->texture->status;
}

/**
 * Get sprite's render head
 *
 * @return
 */
uint16 Sprite::getHead()
{
	return this->head;
}

/**
 * Get sprite's render mode
 *
 * @return 		Mode
 */
uint16 Sprite::getMode()
{
	return this->head & 0x3000;
}

/**
 * Get sprites's world head
 *
 * @return
 */
uint32 Sprite::getEffectiveHead()
{
	if(Sprite::isObject(this))
	{
		return this->head;
	}
	
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->head;
}

/**
 * Get sprites's layer's gx
 *
 * @return
 */
int16 Sprite::getEffectiveX()
{
	if(Sprite::isObject(this))
	{
		return this->position.x;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gx;
}

/**
 * Get sprites's layer's gy
 *
 * @return
 */
int16 Sprite::getEffectiveY()
{
	if(Sprite::isObject(this))
	{
		return this->position.y;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gy;
}

/**
 * Get sprites's layer's gp
 *
 * @return
 */
int16 Sprite::getEffectiveP()
{
	if(Sprite::isObject(this))
	{
		return this->position.parallax;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gp;
}

/**
 * Get sprites's layer's mx
 *
 * @return
 */
int16 Sprite::getWorldMX()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mx;
}

/**
 * Get sprites's layer's my
 *
 * @return
 */
int16 Sprite::getWorldMY()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->my;
}

/**
 * Get sprites's layer's mp
 *
 * @return
 */
int16 Sprite::getWorldMP()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mp;
}

/**
 * Get sprites's layer's width
 *
 * @return 		Width
 */
uint16 Sprite::getEffectiveWidth()
{
	if(Sprite::isObject(this))
	{
		return this->halfWidth << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->w ? 0 : worldPointer->w;
}

/**
 * Get sprites's layer's height
 *
 * @return 		Width
 */
uint16 Sprite::getEffectiveHeight()
{
	if(Sprite::isObject(this))
	{
		return this->halfHeight << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->h ? 0 : worldPointer->h;
}

/**
 * Get half width
 *
 * @return
 */
int32 Sprite::getHalfWidth()
{
	return this->halfWidth;
}

/**
 * Get half height
 *
 * @return
 */
int32 Sprite::getHalfHeight()
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
	if(NULL == this->animationController)
	{
		return;
	}

#ifdef __RELEASE
	if(!this->writeAnimationFrame && this->animationController->playing)
#else
	if(!this->writeAnimationFrame)
#endif
	{
		this->writeAnimationFrame = AnimationController::updateAnimation(this->animationController);
	}
	
	if(this->writeAnimationFrame)
	{
		Sprite::writeAnimation(this);
		this->writeAnimationFrame = false;
	}
}

/**
 * Is visible?
 *
 * @return visibility
 */
bool Sprite::isVisible()
{
	return __NO_RENDER_INDEX != this->index && __SHOW == this->show;
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
	}
}

/**
 * Play a given animation
 *
 * @param animationFunctions	AnimationFunction*
 * @param functionName			Name of animation function to play
 */
bool Sprite::play(const AnimationFunction* animationFunctions[], const char* functionName, ListenerObject scope)
{
	ASSERT(NULL != animationFunctions, "Sprite::play: null animationFunctions");
	ASSERT(NULL != functionName, "Sprite::play: null functionName");

	bool playBackStarted = false;

	if(!isDeleted(this->animationController))
	{
		playBackStarted = AnimationController::play(this->animationController, animationFunctions, functionName, scope);
		this->rendered = this->rendered && !this->writeAnimationFrame;
	}

	return playBackStarted;
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
 * @param animationFunctions	AnimationFunction
 */
bool Sprite::replay(const AnimationFunction* animationFunctions[])
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::replay(this->animationController, animationFunctions);
		this->rendered = this->rendered && !this->writeAnimationFrame;

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
 * Retrieve the name of the current animation
 *
 * @return				Name of the animation that is playing
 */
const char* Sprite::getPlayingAnimationName()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getPlayingAnimationName(this->animationController);
	}

	return "None";
}

/**
 * Set frame cycle decrement
 *
 * @param frameCycleDecrement	Frame cycle decrement
 */
void Sprite::setFrameCycleDecrement(uint8 frameCycleDecrement)
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
int16 Sprite::getActualFrame()
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
void Sprite::setActualFrame(int16 actualFrame)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setActualFrame(this->animationController, actualFrame);
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
uint8 Sprite::getFrameDuration()
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
void Sprite::setFrameDuration(uint8 frameDuration)
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
bool Sprite::isBgmap()
{
	return (__WORLD_BGMAP == (this->head & __WORLD_BGMAP)) || Sprite::isAffine(this) || Sprite::isHBias(this);
}

/**
 * Check if uses OBJECT mode
 *
 * @return		True if it does
 */
bool Sprite::isObject()
{
	return NULL != __GET_CAST(ObjectSprite, this);
}

/**
 * Print status
 *
 * @param x			Camera's x coordinate
 * @param y			Camera's y coordinate
 */
void Sprite::print(int32 x, int32 y)
{
	// Allow normal rendering once for WORLD values to populate properly
	uint8 transparent = this->transparent;
	this->transparent = __TRANSPARENCY_NONE;

	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 18, y, NULL);
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
	else if(Sprite::isBgmap(this))
	{
		Printing::text(Printing::getInstance(), "BGMAP    ", x + 18, y, NULL);
	}

	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->index, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getEffectiveHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Transparent:                         ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), (transparent > 0) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), (transparent == 1) ? "(Even)" : (transparent == 2) ? "(Odd)" : "", x + 20, y, NULL);
	Printing::text(Printing::getInstance(), "Shown:                         ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), (__HIDE != this->show) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);

	Printing::text(Printing::getInstance(), "Pos. (x,y,z,p):                      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->position.x, x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.y, x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.z + Sprite::getDisplacement(this)->z, x + 30, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "Displ. (x,y,z,p):                    ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.x, x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.y, x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.z, x + 30, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "G (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveX(this), x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveY(this), x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveP(this), x + 30, y, NULL);
	Printing::text(Printing::getInstance(), "M (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getWorldMX(this), x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getWorldMY(this), x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getWorldMP(this), x + 30, y, NULL);
	Printing::text(Printing::getInstance(), "Size (w,h):                          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveWidth(this), x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveHeight(this), x + 24, y++, NULL);
	Printing::text(Printing::getInstance(), "Pixels:                      ", x, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getTotalPixels(this), x + 18, y++, NULL);

	if(NULL != Sprite::getTexture(this))
	{
		y++;
		Printing::text(Printing::getInstance(), "TEXTURE                          ", x, ++y, NULL);
		y++;
		Printing::text(Printing::getInstance(), "Spec:                      ", x, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int32)Texture::getTextureSpec(Sprite::getTexture(this)), x + 18, y, 8, NULL);
		Printing::text(Printing::getInstance(), "Size (w,h):                      ", x, ++y, NULL);
		Printing::int32(Printing::getInstance(), this->halfWidth * 2, x + 18, y, NULL);
		Printing::int32(Printing::getInstance(), this->halfHeight * 2, x + 24, y, NULL);

		if(Sprite::getTexture(this) && __GET_CAST(BgmapTexture, Sprite::getTexture(this)))
		{
			BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite::getTexture(this));

			Printing::text(Printing::getInstance(), "Segment:                         ", x, ++y, NULL);
			Printing::int32(Printing::getInstance(), BgmapTexture::getSegment(bgmapTexture), x + 18, y, NULL);
			Printing::text(Printing::getInstance(), "Written:                         ", x, ++y, NULL);
			Printing::text(Printing::getInstance(), Texture::isWritten(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
			Printing::text(Printing::getInstance(), "Rows remaining:                  ", x, ++y, NULL);
			Printing::int32(Printing::getInstance(), BgmapTexture::getRemainingRowsToBeWritten(bgmapTexture), x + 18, y, NULL);
		}
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
void Sprite::putChar(Point* texturePixel, uint32* newChar)
{
	if(isDeleted(this->texture) || NULL == newChar || NULL == texturePixel)
	{
		return;
	}

	Texture::putChar(this->texture, texturePixel, newChar);
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
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
}

/**
 * Invalidate render flag
 *
 */
void Sprite::invalidateRenderFlag()
{
	this->rendered = false;
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
