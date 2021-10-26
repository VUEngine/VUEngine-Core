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

#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <SpriteManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Texture;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof						ObjectSprite
 * @public
 *
 * @param objectSpriteSpec	Sprite spec
 * @param owner						Owner
 */
void ObjectSprite::constructor(const ObjectSpriteSpec* objectSpriteSpec, Object owner)
{
	Base::constructor((SpriteSpec*)objectSpriteSpec, owner);

	this->head = objectSpriteSpec->display & __OBJECT_SPRITE_CHAR_SHOW_MASK;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;

	this->displacement = objectSpriteSpec->spriteSpec.displacement;
	this->halfWidth = 0;
	this->halfHeight = 0;

	ASSERT(objectSpriteSpec->spriteSpec.textureSpec, "ObjectSprite::constructor: null textureSpec");

	if(objectSpriteSpec->spriteSpec.textureSpec)
	{
		this->texture = Texture::safeCast(new ObjectTexture(objectSpriteSpec->spriteSpec.textureSpec, 0, this));
		NM_ASSERT(this->texture, "ObjectSprite::constructor: null texture");

		this->halfWidth = this->texture->textureSpec->cols << 2;
		this->halfHeight = this->texture->textureSpec->rows << 2;

		this->totalObjects = objectSpriteSpec->spriteSpec.textureSpec->cols * objectSpriteSpec->spriteSpec.textureSpec->rows;

		NM_ASSERT(this->texture, "ObjectSprite::constructor: null texture");
	}
	else
	{
		NM_ASSERT(this->texture, "ObjectSprite::constructor: null texture spec");
	}
}

void ObjectSprite::rewrite()
{	
	if(this->hidden || !this->positioned)
	{
		return;
	}

	if(__NO_RENDER_INDEX == this->index)
	{
		return;
	}

	NM_ASSERT(!isDeleted(this->texture), "ObjectSprite::rewrite: null texture");
	NM_ASSERT(!isDeleted(this->texture->charSet), "ObjectSprite::rewrite: null char set");

	int32 charLocation = CharSet::getOffset(this->texture->charSet);

	int16 halfWidth = this->halfWidth;
	int16 halfHeight = this->halfHeight;

	int16 cols = halfWidth >> 2;
	int16 rows = halfHeight >> 2;

	uint16 fourthWordValue = (this->head & 0x3000) | (this->texture->palette << 14);

	int16 jDisplacement = 0;

	BYTE* framePointer = this->texture->textureSpec->mapSpec + (this->texture->mapDisplacement << 1);

	ObjectAttributes* objectPointer = NULL;

	for(int16 i = 0; i < rows; i++, jDisplacement += cols)
	{
		int16 objectIndexStart = this->index + jDisplacement;

		for(int16 j = 0; j < cols; j++)
		{
			int16 objectIndex = objectIndexStart + j;
			objectPointer = &_objectAttributesCache[objectIndex];

			int32 charNumberIndex = (jDisplacement + j) << 1;
			uint16 charNumber = charLocation + (framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8));
			objectPointer->tile = fourthWordValue | charNumber;
		}
	}
}

/**
 * Class destructor
 *
 * @memberof						ObjectSprite
 * @public
 */
void ObjectSprite::destructor()
{
	// remove from sprite container before I become invalid
	// and the VPU triggers a new render cycle
	if(this->registered && this->objectSpriteContainer)
	{
		ObjectSpriteContainer::unregisterSprite(this->objectSpriteContainer, this);
	}

	if(!isDeleted(this->texture))
	{
		delete this->texture;
	}

	this->texture = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Set rotation
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param rotation		The rotation
 */
void ObjectSprite::rotate(const Rotation* rotation)
{
	Direction direction =
	{
		(__QUARTER_ROTATION_DEGREES) < __ABS(rotation->y) || (__QUARTER_ROTATION_DEGREES) < __ABS(rotation->z)  ? __LEFT : __RIGHT,
		(__QUARTER_ROTATION_DEGREES) < __ABS(rotation->x) || (__QUARTER_ROTATION_DEGREES) < __ABS(rotation->z) ? __UP : __DOWN,
		__FAR,
	};

	if(__LEFT == direction.x)
	{
		this->head |= 0x2000;
	}
	else if(__RIGHT == direction.x)
	{
		this->head &= 0xDFFF;
	}

	if(__UP == direction.y)
	{
		this->head |= 0x1000;
	}
	else if(__DOWN == direction.y)
	{
		this->head &= 0xEFFF;
	}
}

/**
 * Check if assigned to a container
 *
 * @memberof			ObjectSprite
 * @private
 */
void ObjectSprite::registerWithManager()
{
	if(!this->registered && NULL == this->objectSpriteContainer && this->totalObjects)
	{
		this->objectSpriteContainer = SpriteManager::getObjectSpriteContainer(SpriteManager::getInstance(), this->position.z + this->displacement.z);
		ObjectSpriteContainer::registerSprite(this->objectSpriteContainer, this);
	}

	Base::registerWithManager(this);
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		ObjectSprite
 * @public
 *
 * @param evenFrame
 */
int16 ObjectSprite::doRender(int16 index, bool evenFrame __attribute__((unused)))
{
	NM_ASSERT(!isDeleted(this->texture), "ObjectSprite::doRender: null texture");
	NM_ASSERT(!isDeleted(this->texture->charSet), "ObjectSprite::doRender: null char set");

	int32 charLocation = CharSet::getOffset(this->texture->charSet);

	int16 xDisplacementIncrement = 8;
	int16 yDisplacementIncrement = 8;

	int16 halfWidth = this->halfWidth;
	int16 halfHeight = this->halfHeight;

	int16 cols = halfWidth >> 2;
	int16 rows = halfHeight >> 2;

	int16 xDisplacementDelta = 0;
	int16 yDisplacementDelta = 0;

	if(this->head & 0x2000)
	{
		xDisplacementIncrement = -8;
		halfWidth = -halfWidth;
		xDisplacementDelta = __OBJECT_SPRITE_FLIP_X_DISPLACEMENT;
	}

	if(this->head & 0x1000)
	{
		yDisplacementIncrement = -8;
		halfHeight = -halfHeight;
		yDisplacementDelta = __OBJECT_SPRITE_FLIP_Y_DISPLACEMENT;
	}

	int16 x = this->position.x - halfWidth + this->displacement.x - xDisplacementDelta;
	int16 y = this->position.y - halfHeight + this->displacement.y - yDisplacementDelta;

	uint16 secondWordValue = this->head | (this->position.parallax + this->displacement.parallax);
	uint16 fourthWordValue = (this->head & 0x3000) | (this->texture->palette << 14);

	int16 yDisplacement = 0;
	int16 jDisplacement = 0;

	BYTE* framePointer = this->texture->textureSpec->mapSpec + (this->texture->mapDisplacement << 1);
	uint16 result = 0;

	ObjectAttributes* objectPointer = NULL;

	index -= this->totalObjects;

	if(0 > index)
	{
		return __NO_RENDER_INDEX;
	}

	for(int16 i = 0; i < rows; i++, jDisplacement += cols, yDisplacement += yDisplacementIncrement)
	{
		int16 outputY = y + yDisplacement;

		int16 objectIndexStart = index + jDisplacement;

		if((unsigned)(outputY - _cameraFrustum->y0 + 4) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0))
		{
			int16 j = 0;
			for(; j < cols; j++)
			{
				int16 objectIndex = objectIndexStart + j;

				objectPointer = &_objectAttributesCache[objectIndex];
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			}
			continue;
		}

		int16 j = 0;
		int16 xDisplacement = 0;

		for(; j < cols; j++, xDisplacement += xDisplacementIncrement)
		{
			int16 objectIndex = objectIndexStart + j;
			objectPointer = &_objectAttributesCache[objectIndex];

			int16 outputX = x + xDisplacement;

			// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			{
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
				continue;
			}

			objectPointer->jx = outputX;
			objectPointer->head = secondWordValue;
			objectPointer->jy = outputY;

			int32 charNumberIndex = (jDisplacement + j) << 1;
			uint16 charNumber = charLocation + (framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8));
			objectPointer->tile = fourthWordValue | charNumber;

			result = index;
		}
	}

	return result;
}

/**
 * Retrieved number of OBJECTs
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @return				Number of used OBJECTs
 */
int16 ObjectSprite::getTotalObjects()
{
	ASSERT(0 < this->totalObjects, "ObjectSprite::getTotalObjects: null totalObjects");

	return this->totalObjects;
}

/**
 * Set Sprite's render mode
 *
 * @memberof		ObjectSprite
 * @public
 *
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSprite::setMode(uint16 display __attribute__ ((unused)), uint16 mode __attribute__ ((unused)))
{}

/**
 * Set ObjectSpriteContainer to NULL
 *
 * @memberof				ObjectSprite
 * @public
 */
void ObjectSprite::invalidateObjectSpriteContainer()
{
	this->objectSpriteContainer = NULL;
}
