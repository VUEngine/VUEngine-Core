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

#define __FLIP_X_DISPLACEMENT	8
#define __FLIP_Y_DISPLACEMENT	8


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

	this->head = objectSpriteSpec->display & __OBJECT_CHAR_SHOW_MASK;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;

	this->displacement = objectSpriteSpec->spriteSpec.displacement;
	this->halfWidth = 0;
	this->halfHeight = 0;

	ASSERT(objectSpriteSpec->spriteSpec.textureSpec, "ObjectSprite::constructor: null textureSpec");

	if(objectSpriteSpec->spriteSpec.textureSpec)
	{
		this->texture = Texture::safeCast(new ObjectTexture(objectSpriteSpec->spriteSpec.textureSpec, 0));
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
	if(this->objectSpriteContainer)
	{
		ObjectSpriteContainer::unregisterSprite(this->objectSpriteContainer, this, this->totalObjects);
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

	if(!isDeleted(this->texture))
	{
		this->texture->written = false;
	}
}

/**
 * Set 2D position
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param position		New 2D position
 */
void ObjectSprite::setPosition(const PixelVector* position)
{
	Base::setPosition(this, position);

	ObjectSprite::checkForContainer(this);
}

/**
 * Calculate 2D position
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param position		3D position
 */
void ObjectSprite::position(const Vector3D* position)
{
	Base::position(this, position);

	ObjectSprite::checkForContainer(this);
}

/**
 * Check if assigned to a container
 *
 * @memberof			ObjectSprite
 * @private
 */
void ObjectSprite::checkForContainer()
{
	if(NULL == this->objectSpriteContainer && this->totalObjects)
	{
		this->objectSpriteContainer = SpriteManager::getObjectSpriteContainer(SpriteManager::getInstance(), this->totalObjects, this->position.z + this->displacement.z);
		ObjectSpriteContainer::registerSprite(this->objectSpriteContainer, this, this->totalObjects);
	}
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		ObjectSprite
 * @public
 *
 * @param evenFrame
 */
u16 ObjectSprite::doRender(u16 index, bool evenFrame __attribute__((unused)))
{
	NM_ASSERT(!isDeleted(this->texture), "ObjectSprite::doRender: null texture");

	int charLocation = CharSet::getOffset(this->texture->charSet);

	s16 xDisplacementIncrement = 8;
	s16 yDisplacementIncrement = 8;

	s16 halfWidth = this->halfWidth;
	s16 halfHeight = this->halfHeight;

	s16 cols = halfWidth >> 2;
	s16 rows = halfHeight >> 2;

	s16 xDisplacementDelta = 0;
	s16 yDisplacementDelta = 0;

	if(this->head & 0x2000)
	{
		xDisplacementIncrement = -8;
		halfWidth = -halfWidth;
		xDisplacementDelta = __FLIP_X_DISPLACEMENT;
	}

	if(this->head & 0x1000)
	{
		yDisplacementIncrement = -8;
		halfHeight = -halfHeight;
		yDisplacementDelta = __FLIP_Y_DISPLACEMENT;
	}

	s16 x = this->position.x - halfWidth + this->displacement.x - xDisplacementDelta;
	s16 y = this->position.y - halfHeight + this->displacement.y - yDisplacementDelta;

	u16 secondWordValue = this->head | (this->position.parallax + this->displacement.parallax);
	u16 fourthWordValue = (this->head & 0x3000) | (this->texture->palette << 14);

	s16 yDisplacement = 0;
	s16 jDisplacement = 0;

	BYTE* framePointer = this->texture->textureSpec->mapSpec + (this->texture->mapDisplacement << 1);
	u16 result = 0;

	ObjectAttributes* objectPointer = NULL;

	for(s16 i = 0; i < rows; i++, jDisplacement += cols, yDisplacement += yDisplacementIncrement)
	{
		s16 outputY = y + yDisplacement;

		s16 objectIndexStart = index + jDisplacement;

		if((unsigned)(outputY - _cameraFrustum->y0 + 4) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0))
		{
			s16 j = 0;
			for(; j < cols; j++)
			{
				s16 objectIndex = objectIndexStart + j;

				objectPointer = &_objectAttributesCache[objectIndex];
				objectPointer->head = __OBJECT_CHAR_HIDE_MASK;
			}
			continue;
		}

		s16 j = 0;
		s16 xDisplacement = 0;

		for(; j < cols; j++, xDisplacement += xDisplacementIncrement)
		{
			s16 objectIndex = objectIndexStart + j;
			objectPointer = &_objectAttributesCache[objectIndex];

			s16 outputX = x + xDisplacement;

			// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			{
				objectPointer->head = __OBJECT_CHAR_HIDE_MASK;
				continue;
			}

			objectPointer->jx = outputX;
			objectPointer->head = secondWordValue;
			objectPointer->jy = outputY;

			s32 charNumberIndex = (jDisplacement + j) << 1;
			u16 charNumber = charLocation + (framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8));
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
s16 ObjectSprite::getTotalObjects()
{
	ASSERT(0 < this->totalObjects, "ObjectSprite::getTotalObjects: null totalObjects");

	return this->totalObjects;
}

/**
 * Get WORLD layer
 *
 * @memberof	Sprite
 * @public
 *
 * @return 		World layer
 */
u8 ObjectSprite::getWorldLayer()
{
	ASSERT(this->objectSpriteContainer, "ObjectSprite::getWorldLayer: null objectSpriteContainer");

	return this->objectSpriteContainer ?  Sprite::getWorldLayer(this->objectSpriteContainer) : 0;
}

/**
 * Add displacement to position
 *
 * @memberof				ObjectSprite
 * @public
 *
 * @param displacement		2D position displacement
 */
void ObjectSprite::addDisplacement(const PixelVector* displacement)
{
	this->position.x += displacement->x;
	this->position.y += displacement->y;
	this->position.z += displacement->z;
	this->position.parallax += displacement->parallax;
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
void ObjectSprite::setMode(u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
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
