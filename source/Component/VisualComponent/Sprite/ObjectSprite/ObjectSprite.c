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
#include <DebugConfig.h>
#include <ObjectTexture.h>

#include "ObjectSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::constructor(Entity owner, const ObjectSpriteSpec* objectSpriteSpec)
{
	NM_ASSERT(NULL != objectSpriteSpec, "ObjectSprite::constructor: NULL objectSpriteSpec");

	// Always explicitly call the base's constructor 
	Base::constructor(owner, (SpriteSpec*)objectSpriteSpec);

	this->head = objectSpriteSpec->display & __OBJECT_SPRITE_CHAR_SHOW_MASK;
	this->totalObjects = 0;

	this->displacement = objectSpriteSpec->spriteSpec.displacement;
	this->xDisplacementIncrement = 8;
	this->yDisplacementIncrement = 8;
	this->xDisplacementDelta = 0;
	this->yDisplacementDelta = 0;
	this->objectTextureSource.displacement = 0;
	this->cols = this->halfWidth >> 2;
	this->rows = this->halfHeight >> 2;

	this->fourthWordValue = (this->head & 0x3000) | (((SpriteSpec*)objectSpriteSpec)->textureSpec->palette << 14);

	ObjectSprite::loadTexture(this, typeofclass(ObjectTexture), true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::destructor()
{
	ObjectSprite::removeFromCache(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ObjectSprite::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventTextureRewritten:
		{
			ObjectSprite::rewrite(this);

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ClassPointer ObjectSprite::getBasicType()
{
	return typeofclass(ObjectSprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::loadTexture(ClassPointer textureClass __attribute__((unused)), bool listenForRewriting __attribute__((unused)))
{
	Base::loadTexture(this, typeofclass(ObjectTexture), true);

	NM_ASSERT(NULL != this->texture, "ObjectSprite::constructor: could not load texture");

	if(NULL != this->texture)
	{
		this->totalObjects = this->cols * this->rows;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 ObjectSprite::doRender(int16 index)
{
	NM_ASSERT(!isDeleted(this->texture), "ObjectSprite::doRender: null texture");
	NM_ASSERT(!isDeleted(this->texture->charSet), "ObjectSprite::doRender: null char set");

	int16 x = this->position.x - this->halfWidth + this->displacement.x - this->xDisplacementDelta;
	int16 y = this->position.y - this->halfHeight + this->displacement.y - this->yDisplacementDelta;

	uint16 secondWordValue = this->head | (this->position.parallax + this->displacement.parallax);
	uint16 fourthWordValue = 
		this->fourthWordValue | (CharSet::getOffset(this->texture->charSet) + this->objectTextureSource.displacement);

	int16 yDisplacement = 0;
	int16 jDisplacement = 0;

	uint16* framePointer = 
		(uint16*)(this->texture->textureSpec->map + this->texture->mapDisplacement);
	uint16 result = index;

	ObjectAttributes* objectPointer = NULL;

	for(int16 i = 0; i < this->rows; i++, jDisplacement += this->cols, yDisplacement += this->yDisplacementIncrement)
	{
		int16 outputY = y + yDisplacement;

		int16 objectIndexStart = index + jDisplacement;

		if((unsigned)(outputY - _cameraFrustum->y0 + 8) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0 + 8))
		{
			for(int16 j = 0; j < this->cols; j++)
			{
				int16 objectIndex = objectIndexStart + j;

				objectPointer = &_objectAttributesCache[objectIndex];
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			}

			continue;
		}

		for(int16 j = 0, xDisplacement = 0; j < this->cols; j++, xDisplacement += this->xDisplacementIncrement)
		{
			int16 objectIndex = objectIndexStart + j;
			objectPointer = &_objectAttributesCache[objectIndex];

			int16 outputX = x + xDisplacement;

			// Add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// Screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0 + 4))
			{
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
				continue;
			}

			objectPointer->jx = outputX;
			objectPointer->head = secondWordValue;
			objectPointer->jy = outputY;

			objectPointer->tile = fourthWordValue + framePointer[jDisplacement + j];

			result = index;
		}
	}

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::setMultiframe(uint16 frame)
{
	this->objectTextureSource.displacement = frame * this->rows * this->cols;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::setRotation(const Rotation* rotation)
{
	this->rotation = *rotation;

	NormalizedDirection normalizedDirection =
	{
		__QUARTER_ROTATION_DEGREES < __ABS(rotation->y) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z)  ? __LEFT : __RIGHT,
		__QUARTER_ROTATION_DEGREES < __ABS(rotation->x) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z) ? __UP : __DOWN,
		__FAR,
	};

	if(__LEFT == normalizedDirection.x)
	{
		this->head |= 0x2000;
	}
	else if(__RIGHT == normalizedDirection.x)
	{
		this->head &= 0xDFFF;
	}

	if(__UP == normalizedDirection.y)
	{
		this->head |= 0x1000;
	}
	else if(__DOWN == normalizedDirection.y)
	{
		this->head &= 0xEFFF;
	}

	this->xDisplacementIncrement = 8;
	this->yDisplacementIncrement = 8;
	this->xDisplacementDelta = 0;
	this->yDisplacementDelta = 0;

	this->halfWidth = this->texture->textureSpec->cols << 2;
	this->halfHeight = this->texture->textureSpec->rows << 2;

	this->cols = this->halfWidth >> 2;
	this->rows = this->halfHeight >> 2;

	if(this->head & 0x2000)
	{
		this->xDisplacementIncrement = -8;
		this->halfWidth = -this->halfWidth;
		this->xDisplacementDelta = __OBJECT_SPRITE_FLIP_X_DISPLACEMENT;
	}

	if(this->head & 0x1000)
	{
		this->yDisplacementIncrement = -8;
		this->halfHeight = -this->halfHeight;
		this->yDisplacementDelta = __OBJECT_SPRITE_FLIP_Y_DISPLACEMENT;
	}

	this->fourthWordValue = (this->head & 0x3000) | (this->texture->palette << 14);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 ObjectSprite::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return ObjectSprite::getTotalObjects(this) * 64;
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::resetTotalObjects()
{
	if(NULL == this->texture)
	{
		return;
	}

	this->totalObjects = Texture::getCols(this->texture) * Texture::getRows(this->texture);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 ObjectSprite::getTotalObjects()
{
	ASSERT(0 < this->totalObjects, "ObjectSprite::getTotalObjects: null totalObjects");

	return this->totalObjects;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::removeFromCache()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		int16 jDisplacement = 0;
		ObjectAttributes* objectPointer = NULL;

		for(int16 i = 0; i < this->rows; i++, jDisplacement += this->cols)
		{
			int16 objectIndexStart = this->index + jDisplacement;

			for(int16 j = 0; j < this->cols; j++)
			{
				int16 objectIndex = objectIndexStart + j;
				objectPointer = &_objectAttributesCache[objectIndex];
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::rewrite()
{
	if(__HIDE == this->show)
	{
		return;
	}

	if(__NO_RENDER_INDEX == this->index)
	{
		return;
	}

	NM_ASSERT(!isDeleted(this->texture), "ObjectSprite::rewrite: null texture");
	NM_ASSERT(!isDeleted(this->texture->charSet), "ObjectSprite::rewrite: null char set");

	uint16 fourthWordValue = 
		(this->head & 0x3000) | (this->texture->palette << 14) | 
		(CharSet::getOffset(this->texture->charSet) +  this->objectTextureSource.displacement);

	int16 jDisplacement = 0;

	uint16* framePointer = 
		(uint16*)(this->texture->textureSpec->map + this->texture->mapDisplacement + this->objectTextureSource.displacement);

	ObjectAttributes* objectPointer = NULL;

	for(int16 i = 0; i < this->rows; i++, jDisplacement += this->cols)
	{
		int16 objectIndexStart = this->index + jDisplacement;

		for(int16 j = 0; j < this->cols; j++)
		{
			int16 objectIndex = objectIndexStart + j;
			objectPointer = &_objectAttributesCache[objectIndex];

			objectPointer->tile = fourthWordValue + framePointer[jDisplacement + j];
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
