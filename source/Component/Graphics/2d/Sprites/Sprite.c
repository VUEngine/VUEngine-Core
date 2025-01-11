/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <Clock.h>
#include <ObjectSprite.h>
#include <Optics.h>
#include <Printing.h>
#include <Texture.h>
#include <VIPManager.h>

#include "Sprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;

#ifdef __RELEASE
friend class AnimationController;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define SPRITE_DEPTH						16
#define SPRITE_HALF_DEPTH					(SPRITE_DEPTH >> 1)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::constructor(Entity owner, const SpriteSpec* spriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, (const VisualComponentSpec*)&spriteSpec->visualComponentSpec);

	// Clear values
	this->index = __NO_RENDER_INDEX;
	this->head = 0;
	this->texture = NULL;
	this->halfWidth = 0;
	this->halfHeight = 0;
	this->transparency = __TRANSPARENCY_NONE;
	this->checkIfWithinScreenSpace = true;
	this->position = (PixelVector){0, 0, 0, 0};
	this->rotation = (Rotation){0, 0, 0};
	this->scale = (PixelScale){__1I_FIX7_9, __1I_FIX7_9};
	this->displacement = PixelVector::zero();
	this->hasTextures = true;

	if(NULL != spriteSpec)
	{
		this->transparency = spriteSpec->transparency;
		
		if(NULL != spriteSpec->textureSpec)
		{
			this->halfWidth = spriteSpec->textureSpec->cols << 2;
			this->halfHeight = spriteSpec->textureSpec->rows << 2;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

RightBox Sprite::getRightBox()
{
#ifndef __RELEASE
	if(0 == this->halfWidth || 0 == this->halfHeight)
	{
		Printing::setDebugMode();
		Printing::clear();
		PRINT_HEX((uint32)this->componentSpec, 1, 27);
		Error::triggerException("Sprite::getRightBox: 0 size sprite", NULL);
	}
#endif

	return (RightBox) 
	{
		__PIXELS_TO_METERS(-this->halfWidth + this->displacement.x),
		__PIXELS_TO_METERS(-this->halfHeight + this->displacement.y),
		__PIXELS_TO_METERS(-SPRITE_HALF_DEPTH), 
		__PIXELS_TO_METERS(this->halfWidth + this->displacement.x),
		__PIXELS_TO_METERS(this->halfHeight + this->displacement.y),
		__PIXELS_TO_METERS(SPRITE_HALF_DEPTH),
	};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
				this->animationController, 
				ListenerObject::safeCast(this->owner), 
				Texture::getSpec(this->texture)->charSetSpec
			)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::forceChangeOfFrame(int16 actualFrame)
{
	if(!isDeleted(this->texture))
	{
		Texture::setFrame(this->texture, actualFrame);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void Sprite::transform()
{
	if(NULL != this->owner)
	{
		Sprite::position(this);
		Sprite::rotate(this);
		Sprite::scale(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::render(int16 index, bool updateAnimation)
{
	// If the client code makes these checks before calling this method,
	// It saves on method calls quite a bit when there are lots of
	// Sprites. Don't uncomment.
/*
	if(__HIDE == this->show)
	{
		return __NO_RENDER_INDEX;
	}
*/

	if(!this->hasTextures)
	{
		this->index = Sprite::doRender(this, index);
		return this->index;
	}

	if(kTextureInvalid == this->texture->status || isDeleted(this->texture->charSet))
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	if(kTexturePendingWriting == this->texture->status)
	{
		this->index = __NO_RENDER_INDEX;
		return this->index;
	}

	if(NULL != this->owner)
	{
		if(NULL == this->transformation)
		{
			return __NO_RENDER_INDEX;
		}
		else if(__NON_TRANSFORMED == this->transformation->invalid)
		{
			return __NO_RENDER_INDEX;
		}

		Sprite::position(this);

		if
		(
			this->rotation.x != this->transformation->rotation.x
			||
			this->rotation.y != this->transformation->rotation.y
			||
			this->rotation.z != this->transformation->rotation.z
		)
		{
			Sprite::rotate(this);
		}

		if
		(
			this->scale.x != this->transformation->scale.x
			||
			this->scale.y != this->transformation->scale.y
		)
		{
			Sprite::scale(this);
		}
	}

	// If the client code makes these checks before calling this method,
	// It saves on method calls quite a bit when there are lots of
	// Sprites. Don't uncomment.
/*
	if
	(
		!(((this->transparency == __TRANSPARENCY_NONE) || (0x01 & (this->transparency ^ evenFrame))) 
		&& 
		Sprite::isWithinScreenSpace(this))
	)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Texture Sprite::getTexture()
{
	return this->texture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getIndex()
{
	return this->index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Sprite::getHead()
{
	return this->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 Sprite::getHalfWidth()
{
	return this->halfWidth;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 Sprite::getHalfHeight()
{
	return this->halfHeight;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Sprite::getEffectiveHead()
{
	if(Sprite::isObject(this))
	{
		return this->head;
	}
	
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Sprite::getEffectiveWidth()
{
	if(Sprite::isObject(this))
	{
		return this->halfWidth << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->w ? 0 : worldPointer->w;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Sprite::getEffectiveHeight()
{
	if(Sprite::isObject(this))
	{
		return this->halfHeight << 1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->h ? 0 : worldPointer->h;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveX()
{
	if(Sprite::isObject(this))
	{
		return this->position.x;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gx;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveY()
{
	if(Sprite::isObject(this))
	{
		return this->position.y;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gy;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveP()
{
	if(Sprite::isObject(this))
	{
		return this->position.parallax;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gp;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveMX()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mx;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveMY()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->my;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Sprite::getEffectiveMP()
{
	if(Sprite::isObject(this))
	{
		return -1;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mp;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isVisible()
{
	return __NO_RENDER_INDEX != this->index && __SHOW == this->show;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isHidden()
{
	return __HIDE == this->show;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isBgmap()
{
	return (__WORLD_BGMAP == (this->head & __WORLD_BGMAP)) || Sprite::isAffine(this) || Sprite::isHBias(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isObject()
{
	return NULL != __GET_CAST(ObjectSprite, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isAffine()
{
	return __WORLD_AFFINE == (this->head & __WORLD_AFFINE);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isHBias()
{
	return __WORLD_HBIAS == (this->head & __WORLD_HBIAS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::setPosition(const PixelVector* position)
{
	if(NULL == position)
	{
		return;
	}

	this->position = *position;
	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const PixelVector* Sprite::getPosition()
{
	return (const PixelVector*)&this->position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::setDisplacement(const PixelVector* displacement)
{
	this->displacement = *displacement;
	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const PixelVector* Sprite::getDisplacement()
{
	return &this->displacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::addChar(const Point* texturePoint, const uint32* newChar)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::addChar(this->texture, texturePoint, newChar);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::putChar(const Point* texturePoint, const uint32* newChar)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::putChar(this->texture, texturePoint, newChar);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::putPixel(const Point* texturePixel, const Pixel* charSetPixel, BYTE newPixelColor)
{
	if(isDeleted(this->texture))
	{
		return;
	}

	Texture::putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::invalidateRendering()
{
	this->rendered = false;
	Sprite::transform(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void* Sprite::getManager()
{
	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::hasSpecialEffects()
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::updateAnimation()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::processEffects(int32 maximumParamTableRowsToComputePerCall __attribute__((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::setMultiframe(uint16 frame __attribute__((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::forceShow()
{
	this->show = __SHOW;

	Sprite::setPosition(this, &this->position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::forceHide()
{
	this->show = __HIDE;

	Sprite::setPosition(this, &this->position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::setRotation(const Rotation* rotation __attribute__((unused)))
{
	if(NULL == rotation)
	{
		return;
	}

	this->rotation = *rotation;
	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::setScale(const PixelScale* scale __attribute__((unused)))
{
	if(NULL == scale)
	{
		return;
	}

	this->scale = *scale;
	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::print(int32 x, int32 y)
{
	// Allow normal rendering once for WORLD values to populate properly
	uint8 transparency = this->transparency;
	this->transparency = __TRANSPARENCY_NONE;

	Printing::text("SPRITE ", x, y++, NULL);
	Printing::text("Class: ", x, ++y, NULL);
	Printing::text(__GET_CLASS_NAME(this), x + 18, y, NULL);
	Printing::text("Mode:", x, ++y, NULL);

	if(Sprite::isObject(this))
	{
		Printing::text("OBJECT   ", x + 18, y, NULL);
	}
	else if(Sprite::isAffine(this))
	{
		Printing::text("AFFINE   ", x + 18, y, NULL);
	}
	else if(Sprite::isHBias(this))
	{
		Printing::text("H-BIAS   ", x + 18, y, NULL);
	}
	else if(Sprite::isBgmap(this))
	{
		Printing::text("BGMAP    ", x + 18, y, NULL);
	}

	Printing::text("Index: ", x, ++y, NULL);
	Printing::int32(this->index, x + 18, y, NULL);
	Printing::text("Head:                         ", x, ++y, NULL);
	Printing::hex(Sprite::getEffectiveHead(this), x + 18, y, 8, NULL);
	Printing::text("Transparent:                         ", x, ++y, NULL);
	Printing::text(transparency > 0 ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
	Printing::text(transparency == 1 ? "(Even)" : (transparency == 2) ? "(Odd)" : "", x + 20, y, NULL);
	Printing::text("Shown:                         ", x, ++y, NULL);
	Printing::text(__HIDE != this->show ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);

	Printing::text("Pos. (x,y,z,p):                      ", x, ++y, NULL);
	Printing::int32(this->position.x, x + 18, y, NULL);
	Printing::int32(this->position.y, x + 24, y, NULL);
	Printing::int32(this->position.z, x + 30, y, NULL);
	Printing::int32(this->position.parallax, x + 36, y, NULL);
	Printing::text("Displ. (x,y,z,p):                    ", x, ++y, NULL);
	Printing::int32(this->displacement.x, x + 18, y, NULL);
	Printing::int32(this->displacement.y, x + 24, y, NULL);
	Printing::int32(this->displacement.z, x + 30, y, NULL);
	Printing::int32(this->displacement.parallax, x + 36, y, NULL);
	Printing::text("FPos. (x,y,z,p):                      ", x, ++y, NULL);
	Printing::int32(this->position.x + this->displacement.x, x + 18, y, NULL);
	Printing::int32(this->position.y + this->displacement.y, x + 24, y, NULL);
	Printing::int32(this->position.z + this->displacement.z, x + 30, y, NULL);
	Printing::int32(this->position.parallax + this->displacement.parallax, x + 36, y, NULL);
	Printing::text("G (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Sprite::getEffectiveX(this), x + 18, y, NULL);
	Printing::int32(Sprite::getEffectiveY(this), x + 24, y, NULL);
	Printing::int32(Sprite::getEffectiveP(this), x + 30, y, NULL);
	Printing::text("M (x,y,p):                           ", x, ++y, NULL);
	Printing::int32(Sprite::getEffectiveMX(this), x + 18, y, NULL);
	Printing::int32(Sprite::getEffectiveMY(this), x + 24, y, NULL);
	Printing::int32(Sprite::getEffectiveMP(this), x + 30, y, NULL);
	Printing::text("Size (w,h):                          ", x, ++y, NULL);
	Printing::int32(Sprite::getEffectiveWidth(this), x + 18, y, NULL);
	Printing::int32(Sprite::getEffectiveHeight(this), x + 24, y++, NULL);
	Printing::text("Pixels:                      ", x, y, NULL);
	Printing::int32(Sprite::getTotalPixels(this), x + 18, y++, NULL);

	if(NULL != Sprite::getTexture(this))
	{
		y++;
		Printing::text("TEXTURE                          ", x, ++y, NULL);
		y++;
		Printing::text("Spec:                      ", x, ++y, NULL);
		Printing::hex((int32)Texture::getSpec(Sprite::getTexture(this)), x + 18, y, 8, NULL);
		Printing::text("Size (w,h):                      ", x, ++y, NULL);
		Printing::int32(this->halfWidth * 2, x + 18, y, NULL);
		Printing::int32(this->halfHeight * 2, x + 24, y, NULL);

		if(Sprite::getTexture(this) && __GET_CAST(BgmapTexture, Sprite::getTexture(this)))
		{
			BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite::getTexture(this));

			Printing::text("Segment:                         ", x, ++y, NULL);
			Printing::int32(BgmapTexture::getSegment(bgmapTexture), x + 18, y, NULL);
			Printing::text("Written:                         ", x, ++y, NULL);
			Printing::text
			(
				
				Texture::isWritten(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL
			);

			Printing::text("Rows remaining:                  ", x, ++y, NULL);
			Printing::int32(BgmapTexture::getRemainingRowsToBeWritten(bgmapTexture), x + 18, y, NULL);
		}
	}

	this->transparency = transparency;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::position()
{
#ifdef __SPRITE_ROTATE_IN_3D
	PixelVector position = PixelVector::transformVector3D(this->transformation->position);

	if(position.z != this->position.z)
	{
		this->scale.x = this->scale.y = 0;
	}

#else
	PixelVector position = 
		PixelVector::projectVector3D(Vector3D::sub(this->transformation->position, *_cameraPosition), this->position.parallax);

	if(position.z != this->position.z)
	{
		position.parallax = Optics::calculateParallax(this->transformation->position.z - _cameraPosition->z);

		this->scale.x = this->scale.y = 0;
	}
#endif

	Sprite::setPosition(this, &position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::rotate()
{
	this->rendered = false;

	if(Sprite::overrides(this, setRotation))
	{
		Sprite::setRotation(this, &this->transformation->rotation);
	}
	else
	{
		this->rotation = this->transformation->rotation;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sprite::scale()
{
	this->rendered = false;

	PixelScale scale = 
	{
		this->transformation->scale.x,
		this->transformation->scale.y
	};

	if(Sprite::overrides(this, setScale))
	{
		Sprite::setScale(this, &scale);
	}
	else
	{
		this->scale = scale;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sprite::isWithinScreenSpace()
{
	if
	(
		!(
			(unsigned)(this->position.x + this->displacement.x - (_cameraFrustum->x0 - this->halfWidth)) 
			< 
			(unsigned)(_cameraFrustum->x1 + this->halfWidth - (_cameraFrustum->x0 - this->halfWidth))
		)
	)
	{
		return false;
	}

	if
	(
		!(
			(unsigned)(this->position.y + this->displacement.y - (_cameraFrustum->y0 - this->halfHeight)) 
			< 
			(unsigned)(_cameraFrustum->y1 + this->halfHeight - (_cameraFrustum->y0 - this->halfHeight))
		)
	)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
