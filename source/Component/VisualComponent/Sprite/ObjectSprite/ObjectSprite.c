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

friend class CharSet;
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

	ObjectSprite::loadTexture(this, typeofclass(ObjectTexture));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::destructor()
{
	ObjectSprite::removeFromCache(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ClassPointer ObjectSprite::getBasicType()
{
	return typeofclass(ObjectSprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSprite::loadTexture(ClassPointer textureClass __attribute__((unused)))
{
	Base::loadTexture(this, typeofclass(ObjectTexture));

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

	// Cache reused references
	int16 cameraFrustumX0 = _cameraFrustum->x0, cameraFrustumX1 = _cameraFrustum->x1;
	int16 cameraFrustumY0 = _cameraFrustum->y0, cameraFrustumY1 = _cameraFrustum->y1;
	Texture texture = this->texture;

	const int16 xDeltaIncrement = this->xDisplacementIncrement;
	const int16 yDeltaIncrement = this->yDisplacementIncrement;
	const int16 cols = this->cols;
	const int16 rows = this->rows;

	int16 x = this->position.x - this->halfWidth + this->displacement.x - this->xDisplacementDelta;
	int16 y = this->position.y - this->halfHeight + this->displacement.y - this->yDisplacementDelta;

	uint16 secondWordValue = this->head | (this->position.parallax + this->displacement.parallax);
	uint16 fourthWordValue = 
		this->fourthWordValue | (texture->charSet->offset + this->objectTextureSource.displacement);

	uint16* framePointer = (uint16*)(texture->textureSpec->map + texture->mapDisplacement);
	ObjectAttributes* objectAttributesCache = &_objectAttributesCache[index];

	uint16 yLimit = cameraFrustumY0 - 8;
	uint16 xLimit = cameraFrustumX0 - 4;

	int16 yDisplacement = 0;
	int16 jDisplacement = 0;

	for (int16 i = 0; i < rows; i++, jDisplacement += cols, yDisplacement += yDeltaIncrement)
	{
		int16 outputY = y + yDisplacement;

		if((unsigned)(outputY - yLimit) > (unsigned)(cameraFrustumY1 - yLimit))
		{
			ObjectAttributes* object = objectAttributesCache + jDisplacement;
			
			for (int16 j = 0; j < cols; j++, object++)
			{				
				object->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			}
			continue;
		}

		uint16* frameRow = framePointer + jDisplacement;
		ObjectAttributes* object = objectAttributesCache + jDisplacement;
		
		for (int16 j = 0, xDisplacement = 0; j < cols; j++, xDisplacement += xDeltaIncrement, object++)
		{
			int16 outputX = x + xDisplacement;

			if((unsigned)(outputX - xLimit) > (unsigned)(cameraFrustumX1 - xLimit))
			{
				object->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
				continue;
			}

			object->jx = outputX;
			object->jy = outputY;
			object->head = secondWordValue;
			object->tile = fourthWordValue + frameRow[j];
		}
	}

	return index;
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
