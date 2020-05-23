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

	this->head = objectSpriteSpec->display;
	this->objectIndex = __OBJECT_NO_INDEX;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;
	this->didHide = false;

	this->displacement = objectSpriteSpec->spriteSpec.displacement;
	this->halfWidth = 0;
	this->halfHeight = 0;

	ASSERT(objectSpriteSpec->spriteSpec.textureSpec, "ObjectSprite::constructor: null textureSpec");

	if(objectSpriteSpec->spriteSpec.textureSpec)
	{
		this->texture = Texture::safeCast(new ObjectTexture(objectSpriteSpec->spriteSpec.textureSpec, 0));
		Object::addEventListener(this->texture, Object::safeCast(this), (EventListener)ObjectSprite::onTextureRewritten, kEventTextureRewritten);

		this->halfWidth = this->texture->textureSpec->cols << 2;
		this->halfHeight = this->texture->textureSpec->rows << 2;

		this->totalObjects = objectSpriteSpec->spriteSpec.textureSpec->cols * objectSpriteSpec->spriteSpec.textureSpec->rows;

		NM_ASSERT(this->texture, "ObjectSprite::constructor: null texture");
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
		ObjectSpriteContainer::removeObjectSprite(this->objectSpriteContainer, this, this->totalObjects);
	}

	if(!isDeleted(this->texture))
	{
		Object::removeEventListener(this->texture, Object::safeCast(this), (EventListener)ObjectSprite::onTextureRewritten, kEventTextureRewritten);
		delete this->texture;
	}

	this->texture = NULL;
	this->objectIndex = __OBJECT_NO_INDEX;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void ObjectSprite::onTextureRewritten(Object eventFirer __attribute__ ((unused)))
{
	this->writeAnimationFrame = true;
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
	if(__OBJECT_NO_INDEX >= this->objectIndex && this->totalObjects)
	{
		this->objectSpriteContainer = SpriteManager::getObjectSpriteContainer(SpriteManager::getInstance(), this->totalObjects, this->position.z + this->displacement.z);
		this->objectIndex = ObjectSpriteContainer::addObjectSprite(this->objectSpriteContainer, this, this->totalObjects);
		ASSERT(__OBJECT_NO_INDEX < this->objectIndex, "ObjectSprite::position: 0 > this->objectIndex");
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
bool ObjectSprite::render(u8 worldLayer __attribute__((unused)))
{
	if(isDeleted(this->texture) | !this->positioned)
	{
		return false;
	}

	if(!this->texture->written)
	{
		ObjectTexture::setObjectIndex(this->texture, this->objectIndex, true);
	}

	s16 cols = this->texture->textureSpec->cols;
	s16 rows = this->texture->textureSpec->rows;

	s16 xDisplacementIncrement = 8;
	s16 yDisplacementIncrement = 8;

	s16 halfWith = this->halfWidth;
	s16 halfHeight = this->halfHeight;

	s16 xDisplacementDelta = 0;
	s16 yDisplacementDelta = 0;

	if(this->head & 0x2000)
	{
		xDisplacementIncrement = -8;
		halfWith = -halfWith;
		xDisplacementDelta = __FLIP_X_DISPLACEMENT;
	}

	if(this->head & 0x1000)
	{
		yDisplacementIncrement = -8;
		halfHeight = -halfHeight;
		yDisplacementDelta = __FLIP_Y_DISPLACEMENT;
	}

	s16 x = this->position.x - halfWith + this->displacement.x - xDisplacementDelta;
	s16 y = this->position.y - halfHeight + this->displacement.y - yDisplacementDelta;

	s16 i = 0;
	u16 secondWordValue = (this->head & __OBJECT_CHAR_SHOW_MASK) | ((this->position.parallax + this->displacement.parallax) & ~__OBJECT_CHAR_SHOW_MASK);
	u16 fourthWordValue = (this->head & 0x3000);

	s16 yDisplacement = 0;
	s16 jDisplacement = 0;

	for(; i < rows; i++, jDisplacement += cols, yDisplacement += yDisplacementIncrement)
	{
		s16 outputY = y + yDisplacement;

		if((unsigned)(outputY - _cameraFrustum->y0 + 4) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0))
		{
			s16 j = 0;
			for(; j < cols; j++)
			{
				s16 objectIndex = (this->objectIndex + jDisplacement + j) << 2;

				_objectAttributesBaseAddress[objectIndex + 1] = __OBJECT_CHAR_HIDE_MASK;
			}
			continue;
		}

		s16 j = 0;
		s16 xDisplacement = 0;

		for(; j < cols; j++, xDisplacement += xDisplacementIncrement)
		{
			s16 objectIndex = (this->objectIndex + jDisplacement + j) << 2;

			s16 outputX = x + xDisplacement;

			// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			{
				_objectAttributesBaseAddress[objectIndex + 1] = __OBJECT_CHAR_HIDE_MASK;
				continue;
			}

			_objectAttributesBaseAddress[objectIndex] = outputX;
			_objectAttributesBaseAddress[objectIndex + 1] = secondWordValue;
			_objectAttributesBaseAddress[objectIndex + 2] = outputY;
			_objectAttributesBaseAddress[objectIndex + 3] |= fourthWordValue;
		}
	}

	return true;
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
 * Retrieved the OBJECT index
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @return				Number of used OBJECTs
 */
s16 ObjectSprite::getObjectIndex()
{
	return this->objectIndex;
}

/**
 * Show
 *
 * @memberof			ObjectSprite
 * @public
 */
void ObjectSprite::show()
{
	Base::show(this);
	this->didHide = false;
}

/**
 * Hide
 *
 * @memberof			ObjectSprite
 * @public
 */
void ObjectSprite::hide()
{
	Base::hide(this);
	this->didHide = false;
}

/**
 * Set the OBJECT index
 *
 * @memberof				ObjectSprite
 * @public
 *
 * @param objectIndex		Set the OBJECT index
 */
void ObjectSprite::setObjectIndex(s16 objectIndex)
{
	int previousObjectIndex = this->objectIndex;
	this->objectIndex = objectIndex;

	if(__OBJECT_NO_INDEX < this->objectIndex)
	{
		// rewrite texture
		if(!isDeleted(this->texture))
		{
			ObjectTexture::setObjectIndex(this->texture, this->objectIndex, true);
		}

		if(__OBJECT_NO_INDEX < previousObjectIndex)
		{
			// hide the previously used objects
			int j = previousObjectIndex;
			for(; j < previousObjectIndex + this->totalObjects; j++)
			{
				_objectAttributesBaseAddress[(j << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
			}

			if(!this->hidden)
			{
				Sprite::show(this);

				// turn off previous OBJs' to avoid ghosting
				if(this->objectIndex < previousObjectIndex)
				{
					int counter = 0;
					int j = previousObjectIndex + this->totalObjects - 1;
					for(; j >= this->objectIndex + this->totalObjects && counter < this->totalObjects; j--, counter++)
					{
						_objectAttributesBaseAddress[(j << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
					}
				}
				else
				{
				}
			}
		}
	}
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
