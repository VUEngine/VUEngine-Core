/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <Clock.h>
#include <ObjectSprite.h>
#include <Optics.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <VIPManager.h>

#include "Sprite.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Texture;

#ifdef __RELEASE
friend class AnimationController;
#endif


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Sprite::constructor(SpatialObject owner, const SpriteSpec* spriteSpec)
{
	Base::constructor(owner, spriteSpec);

	// clear values
	this->index = __NO_RENDER_INDEX;
	this->head = 0;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->animationController = NULL;
	this->texture = NULL;
	this->transparent = spriteSpec ? spriteSpec->transparent : __TRANSPARENCY_NONE;
	this->updateAnimationFrame = false;
	this->checkIfWithinScreenSpace = true;
	this->position = (PixelVector){0, 0, 0, 0};
	this->rotation = Rotation::zero();
	this->scale = (PixelScale){__1I_FIX7_9, __1I_FIX7_9};
	this->transformed = false;
	this->displacement = PixelVector::zero();
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
int16 Sprite::render(int16 index, bool updateAnimation)
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
		this->index = Sprite::doRender(this, index);
		return this->index;
	}

	if(NULL != this->owner)
	{
		if(NULL != this->transformation && __NON_TRANSFORMED == this->transformation->invalid)
		{
			return __NO_RENDER_INDEX;
		}

		Sprite::position(this);
		Sprite::rotate(this);
		Sprite::scale(this);

		this->transformed = true;
	}

	if(kTextureInvalid == this->texture->status || NULL == this->texture->charSet)
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	if(kTexturePendingWriting == this->texture->status)
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	NM_ASSERT(!isDeleted(this->texture->charSet), "Sprite::render: null char set");

	// If the client code makes these checks before calling this method,
	// it saves on method calls quite a bit when there are lots of
	// sprites. Don't uncomment.
/*
	if(!(((this->transparent == __TRANSPARENCY_NONE) || (0x01 & (this->transparent ^ evenFrame))) && Sprite::isWithinScreenSpace(this)))
	{
		return this->index;
	}
*/

	// Do not remove this check, it prevents sprites from looping
	if(this->checkIfWithinScreenSpace && !Sprite::isWithinScreenSpace(this))
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	if(updateAnimation)	
	{
		Sprite::update(this);
	}

	if(!this->rendered || this->index != index)
	{
		this->rendered = true;

 		this->index = Sprite::doRender(this, index);

#ifdef __SHOW_SPRITES_PROFILING
		extern int32 _renderedSprites;
		_renderedSprites++;
#endif		
	}

	return this->index;
}
//---------------------------------------------------------------------------------------------------------
Texture Sprite::getTexture()
{
	return this->texture;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getIndex()
{
	return this->index;
}
//---------------------------------------------------------------------------------------------------------
uint16 Sprite::getHead()
{
	return this->head;
}
//---------------------------------------------------------------------------------------------------------
int32 Sprite::getHalfWidth()
{
	return this->halfWidth;
}
//---------------------------------------------------------------------------------------------------------
int32 Sprite::getHalfHeight()
{
	return this->halfHeight;
}
//---------------------------------------------------------------------------------------------------------
uint32 Sprite::getEffectiveHead()
{
	if(Sprite::isObject(this))
	{
		return this->head;
	}
	
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->head;
}
//---------------------------------------------------------------------------------------------------------
uint16 Sprite::getEffectiveWidth()
{
	if(Sprite::isObject(this))
	{
		return this->halfWidth << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->w ? 0 : worldPointer->w;
}
//---------------------------------------------------------------------------------------------------------
uint16 Sprite::getEffectiveHeight()
{
	if(Sprite::isObject(this))
	{
		return this->halfHeight << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->h ? 0 : worldPointer->h;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveX()
{
	if(Sprite::isObject(this))
	{
		return this->position.x;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gx;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveY()
{
	if(Sprite::isObject(this))
	{
		return this->position.y;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gy;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveP()
{
	if(Sprite::isObject(this))
	{
		return this->position.parallax;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gp;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveMX()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mx;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveMY()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->my;
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getEffectiveMP()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mp;
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isVisible()
{
	return __NO_RENDER_INDEX != this->index && __SHOW == this->show;
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isHidden()
{
	return __HIDE == this->show;
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isBgmap()
{
	return (__WORLD_BGMAP == (this->head & __WORLD_BGMAP)) || Sprite::isAffine(this) || Sprite::isHBias(this);
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isObject()
{
	return NULL != __GET_CAST(ObjectSprite, this);
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isAffine()
{
	return __WORLD_AFFINE == (this->head & __WORLD_AFFINE);
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isHBias()
{
	return __WORLD_HBIAS == (this->head & __WORLD_HBIAS);
}
//---------------------------------------------------------------------------------------------------------
void Sprite::createAnimationController()
{
    this->animationController = new AnimationController();

	if(isDeleted(this->animationController))
	{
		this->animationController = NULL;
		return;
	}
	
	if(!isDeleted(this->texture) && Texture::isSingleFrame(this->texture) && Texture::isShared(this->texture))
	{
		AnimationController::setAnimationCoordinator
		(
			this->animationController,
			AnimationCoordinatorFactory::getCoordinator
			(
				AnimationCoordinatorFactory::getInstance(),
				this->animationController, 
				ListenerObject::safeCast(this->owner), 
				Texture::getSpec(this->texture)->charSetSpec
			)
		);
	}
}
//---------------------------------------------------------------------------------------------------------
AnimationController Sprite::getAnimationController()
{
	return this->animationController;
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::play(const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope)
{
	ASSERT(NULL != animationFunctions, "Sprite::play: null animationFunctions");
	ASSERT(NULL != animationName, "Sprite::play: null animationName");

	bool playBackStarted = false;

	if(!isDeleted(this->animationController))
	{
		playBackStarted = AnimationController::play(this->animationController, animationFunctions, animationName, scope);
		this->rendered = this->rendered && !this->updateAnimationFrame;
	}

	return playBackStarted;
}
bool Sprite::replay(const AnimationFunction* animationFunctions[])
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::replay(this->animationController, animationFunctions);
		this->rendered = this->rendered && !this->updateAnimationFrame;

		return this->updateAnimationFrame;
	}

	return false;
}

void Sprite::pause(bool pause)
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		AnimationController::pause(this->animationController, pause);
	}
}
void Sprite::stop()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::stop(this->animationController);
	}
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isPlaying()
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		return AnimationController::isPlaying(this->animationController);
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::isPlayingAnimation(char* animationName)
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::isPlayingFunction(this->animationController, animationName);
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::nextFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::nextFrame(this->animationController);
		this->updateAnimationFrame = true;
	}
}
//---------------------------------------------------------------------------------------------------------
void Sprite::previousFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::previousFrame(this->animationController);
		this->updateAnimationFrame = true;
	}
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setActualFrame(int16 actualFrame)
{
	if(!isDeleted(this->animationController))
	{
		this->updateAnimationFrame = this->updateAnimationFrame || AnimationController::setActualFrame(this->animationController, actualFrame);
	}
	else if(!isDeleted(this->texture))
	{
		Texture::setFrame(this->texture, actualFrame);
	}
}
//---------------------------------------------------------------------------------------------------------
int16 Sprite::getActualFrame()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getActualFrame(this->animationController);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setFrameDuration(uint8 frameDuration)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameDuration(this->animationController, frameDuration);
	}
}
//---------------------------------------------------------------------------------------------------------
uint8 Sprite::getFrameDuration()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getFrameDuration(this->animationController);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setFrameDurationDecrement(uint8 frameDurationDecrement)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameDurationDecrement(this->animationController, frameDurationDecrement);
	}
}
//---------------------------------------------------------------------------------------------------------
const char* Sprite::getPlayingAnimationName()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getPlayingAnimationName(this->animationController);
	}

	return "None";
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setPosition(const PixelVector* position)
{
	if(NULL == position)
	{
		return;
	}

	this->position = *position;
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
const PixelVector* Sprite::getPosition()
{
	return (const PixelVector*)&this->position;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setDisplacement(const PixelVector* displacement)
{
	this->displacement = *displacement;
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
const PixelVector* Sprite::getDisplacement()
{
	return &this->displacement;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void Sprite::addChar(const Point* texturePoint, const uint32* newChar)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::addChar(this->texture, texturePoint, newChar);
}
//---------------------------------------------------------------------------------------------------------
void Sprite::putChar(const Point* texturePoint, const uint32* newChar)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::putChar(this->texture, texturePoint, newChar);
}
//---------------------------------------------------------------------------------------------------------
void Sprite::putPixel(const Point* texturePixel, const Pixel* charSetPixel, BYTE newPixelColor)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
}
//---------------------------------------------------------------------------------------------------------
bool Sprite::hasSpecialEffects()
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::invalidateRendering()
{
	this->transformed = false;
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::updateAnimation()
{}
//---------------------------------------------------------------------------------------------------------
void Sprite::processEffects()
{}
//---------------------------------------------------------------------------------------------------------
void Sprite::setMultiframe(uint16 frame __attribute__((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void Sprite::forceShow()
{
	this->show = __SHOW;

	Sprite::setPosition(this, &this->position);
}
//---------------------------------------------------------------------------------------------------------
void Sprite::forceHide()
{
	this->show = __HIDE;

	Sprite::setPosition(this, &this->position);
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setRotation(const Rotation* rotation __attribute__((unused)))
{
	this->rotation = *rotation;
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
void Sprite::setScale(const PixelScale* scale __attribute__((unused)))
{
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
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
	Printing::int32(Printing::getInstance(), this->position.z, x + 30, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "Displ. (x,y,z,p):                    ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.x, x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.y, x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.z, x + 30, y, NULL);
	Printing::int32(Printing::getInstance(), this->displacement.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "FPos. (x,y,z,p):                      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->position.x + this->displacement.x, x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.y + this->displacement.y, x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.z + this->displacement.z, x + 30, y, NULL);
	Printing::int32(Printing::getInstance(), this->position.parallax + this->displacement.parallax, x + 36, y, NULL);
	Printing::text(Printing::getInstance(), "G (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveX(this), x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveY(this), x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveP(this), x + 30, y, NULL);
	Printing::text(Printing::getInstance(), "M (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveMX(this), x + 18, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveMY(this), x + 24, y, NULL);
	Printing::int32(Printing::getInstance(), Sprite::getEffectiveMP(this), x + 30, y, NULL);
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
		Printing::hex(Printing::getInstance(), (int32)Texture::getSpec(Sprite::getTexture(this)), x + 18, y, 8, NULL);
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

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Sprite::position()
{
	if(NULL == this->transformation)
	{
		return;
	}

#ifdef __SPRITE_ROTATE_IN_3D
	PixelVector position = PixelVector::transformVector3D(this->transformation->position);

	if(position.z != this->position.z)
	{
		this->scale.x = this->scale.y = 0;
	}

#else
	PixelVector position = PixelVector::projectVector3D(Vector3D::sub(this->transformation->position, *_cameraPosition), this->position.parallax);

	if(position.z != this->position.z)
	{
		position.parallax = Optics::calculateParallax(this->transformation->position.z - _cameraPosition->z);

		this->scale.x = this->scale.y = 0;
	}
#endif

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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
		0 >= this->scale.x
		||
		0 >= this->scale.y
		||
		this->scale.x != this->transformation->scale.x
		||
		this->scale.y != this->transformation->scale.y
	)
	{
		this->scale.x = this->transformation->scale.x;
		this->scale.y = this->transformation->scale.y;

		this->rendered = false;

		if(Sprite::overrides(this, setScale))
		{
			PixelScale scale = this->scale;

			NM_ASSERT(0 < scale.x, "Sprite::scale: 0 scale x");
			NM_ASSERT(0 < scale.y, "Sprite::scale: 0 scale y");

			fix7_9 ratio = __FIXED_TO_FIX7_9(Vector3D::getScale(this->transformation->position.z, true));

			ratio = 0 > ratio? __1I_FIX7_9 : ratio;
			ratio = __I_TO_FIX7_9(__MAXIMUM_SCALE) < ratio? __I_TO_FIX7_9(__MAXIMUM_SCALE) : ratio;

			scale.x = __FIX7_9_MULT(scale.x, ratio);
			scale.y = __FIX7_9_MULT(scale.y, ratio);

			Sprite::setScale(this, &scale);
		}		
	}
}
//---------------------------------------------------------------------------------------------------------
void Sprite::update()
{
	if(NULL == this->animationController)
	{
		return;
	}

#ifdef __RELEASE
	if(!this->updateAnimationFrame && this->animationController->playing)
#else
	if(!this->updateAnimationFrame)
#endif
	{
		this->updateAnimationFrame = AnimationController::updateAnimation(this->animationController);
	}
	
	if(this->updateAnimationFrame)
	{
		Sprite::updateAnimation(this);
		this->updateAnimationFrame = false;
		this->rendered = false;
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
