/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <ObjectSpriteContainerManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __FLIP_X_DISPLACEMENT	6
#define __FLIP_Y_DISPLACEMENT	6


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ObjectSprite
 * @extends Sprite
 * @ingroup graphics-2d-sprites-object
 * @brief	Sprite which holds a texture and a drawing specification.
 */
__CLASS_DEFINITION(ObjectSprite, Sprite);
__CLASS_FRIEND_DEFINITION(Texture);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern Optical* _optical;

static void ObjectSprite_checkForContainer(ObjectSprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSprite, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner)
__CLASS_NEW_END(ObjectSprite, objectSpriteDefinition, owner);

/**
 * Class constructor
 *
 * @memberof						ObjectSprite
 * @public
 *
 * @param this						Function scope
 * @param objectSpriteDefinition	Sprite definition
 * @param owner						Owner
 */
void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner)
{
	ASSERT(this, "ObjectSprite::constructor: null this");

	__CONSTRUCT_BASE(Sprite, (SpriteDefinition*)objectSpriteDefinition, owner);

	this->head = objectSpriteDefinition->display;
	this->objectIndex = -1;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;

	// clear position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	this->position.parallax = 0;

	this->displacement = objectSpriteDefinition->spriteDefinition.displacement;

	ASSERT(objectSpriteDefinition->spriteDefinition.textureDefinition, "ObjectSprite::constructor: null textureDefinition");

	this->texture = __SAFE_CAST(Texture, __NEW(ObjectTexture, objectSpriteDefinition->spriteDefinition.textureDefinition, 0));
	this->halfWidth = ITOFIX19_13((int)this->texture->textureDefinition->cols << 2);
	this->halfHeight = ITOFIX19_13((int)this->texture->textureDefinition->rows << 2);
	this->totalObjects = objectSpriteDefinition->spriteDefinition.textureDefinition->cols * objectSpriteDefinition->spriteDefinition.textureDefinition->rows;
	ASSERT(this->texture, "ObjectSprite::constructor: null texture");
}

/**
 * Class destructor
 *
 * @memberof						ObjectSprite
 * @public
 *
 * @param this						Function scope
 */
void ObjectSprite_destructor(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::destructor: null this");

	// remove from sprite container before I become invalid
	// and the VPU triggers a new render cycle
	if(this->objectSpriteContainer)
	{
		ObjectSpriteContainer_removeObjectSprite(this->objectSpriteContainer, this, this->totalObjects);
	}

	if(this->texture)
	{
		__DELETE(this->texture);
		this->texture = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Set direction
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 * @param axis			Axis to modify
 * @param direction		Direction value
 */
void ObjectSprite_setDirection(ObjectSprite this, int axis, int direction)
{
	ASSERT(this, "ObjectSprite::setDirection: null this");

	switch(axis)
	{
		case __XAXIS:

			if(__LEFT == direction)
			{
				this->head |= 0x2000;
			}
			else if(__RIGHT == direction)
			{
				this->head &= 0xDFFF;
			}
			break;

		case __YAXIS:

			if(__UP == direction)
			{
				this->head |= 0x1000;
			}
			else if(__DOWN == direction)
			{
				this->head &= 0xEFFF;
			}
			break;
	}

	this->texture->written = false;
}

/**
 * Retrieve 2D position
 *
 * @memberof		ObjectSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			2D position
 */
VBVec2D ObjectSprite_getPosition(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getPosition: null this");

	return this->position;
}

/**
 * Set 2D position
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 * @param position		New 2D position
 */
void ObjectSprite_setPosition(ObjectSprite this, const VBVec2D* position)
{
	ASSERT(this, "ObjectSprite::setPosition: null this");

	this->position = *position;

	ObjectSprite_checkForContainer(this);
}

/**
 * Calculate 2D position
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 * @param position		3D position
 */
void ObjectSprite_position(ObjectSprite this, const VBVec3D* position)
{
	ASSERT(this, "ObjectSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "ObjectSprite::position: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->position);
	this->position.z = position->z;

	ObjectSprite_checkForContainer(this);
}

/**
 * Check if assigned to a container
 *
 * @memberof			ObjectSprite
 * @private
 *
 * @param this			Function scope
 */
static void ObjectSprite_checkForContainer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::checkForContainer: null this");

	if(0 > this->objectIndex)
	{
		this->objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager_getInstance(), this->totalObjects, this->position.z);
		ObjectSprite_setObjectIndex(this, ObjectSpriteContainer_addObjectSprite(this->objectSpriteContainer, this, this->totalObjects));
		ASSERT(0 <= this->objectIndex, "ObjectSprite::position: 0 > this->objectIndex");
	}
}

/**
 * Calculate parallax
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 * @param z				Z coordinate to base on the calculation
 */
void ObjectSprite_calculateParallax(ObjectSprite this, fix19_13 z)
{
	ASSERT(this, "ObjectSprite::calculateParallax: null this");

	this->position.z = z - _screenPosition->z;
	this->position.parallax = Optics_calculateParallax(this->position.x, z);
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		ObjectSprite
 * @public
 *
 * @param this		Function scope
 */
void ObjectSprite_render(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::render: null this");
	ASSERT(this->texture, "ObjectSprite::render: null texture");
//	ASSERT(Texture_getCharSet(this->texture), "ObjectSprite::render: null charSet");

	if(0 > this->objectIndex)
	{
		this->objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager_getInstance(), this->totalObjects, this->position.z);
		ObjectSprite_setObjectIndex(this, ObjectSpriteContainer_addObjectSprite(this->objectSpriteContainer, this, this->totalObjects));
		ASSERT(0 <= this->objectIndex, "ObjectSprite::position: 0 > this->objectIndex");
	}

	if(this->hidden || !this->visible)
	{
		if(0 <= this->objectIndex)
		{
			int i = 0;
			for(; i < this->totalObjects; i++)
			{
				_objectAttributesBaseAddress[((this->objectIndex + i) << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
			}
		}

		return;
	}

	if(!this->texture->written)
	{
		ObjectTexture_write(__SAFE_CAST(ObjectTexture, this->texture));
	}

	int cols = this->texture->textureDefinition->cols;
	int rows = this->texture->textureDefinition->rows;

	int xDirection = this->head & 0x2000 ? -1 : 1;
	int yDirection = this->head & 0x1000 ? -1 : 1;

	int x = FIX19_13TOI(this->position.x - this->halfWidth * xDirection + this->displacement.x + __0_5F_FIX19_13) - (__LEFT == xDirection? __FLIP_X_DISPLACEMENT : 0);
	int y = FIX19_13TOI(this->position.y - this->halfHeight * yDirection + this->displacement.y + __0_5F_FIX19_13) - (__UP == yDirection? __FLIP_Y_DISPLACEMENT : 0);

	int i = 0;
	u16 secondWordValue = (this->head & __OBJECT_CHAR_SHOW_MASK) | ((this->position.parallax + FIX19_13TOI(this->displacement.z + this->displacement.p)) & __OBJECT_CHAR_HIDE_MASK);
	u16 fourthWordValue = (this->head & 0x3000);

	for(; i < rows; i++)
	{
		int j = 0;
		for(; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			int outputX = x + (j << 3) * xDirection;

			// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// screen's bounds
			if((unsigned)(outputX + 8) > __SCREEN_WIDTH + 8)
			{
				_objectAttributesBaseAddress[(objectIndex << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
				continue;
			}

			int outputY = y + (i << 3) * yDirection;

			if((unsigned)(outputY + 8) > __SCREEN_HEIGHT + 8)
			{
				_objectAttributesBaseAddress[(objectIndex << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
				continue;
			}

			_objectAttributesBaseAddress[(objectIndex << 2)] = outputX;
			_objectAttributesBaseAddress[(objectIndex << 2) + 1] = secondWordValue;
			_objectAttributesBaseAddress[(objectIndex << 2) + 2] = outputY;
			_objectAttributesBaseAddress[(objectIndex << 2) + 3] |= fourthWordValue;
		}
	}
}

/**
 * Retrieved number of OBJECTs
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 * @return				Number of used OBJECTs
 */
s16 ObjectSprite_getTotalObjects(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getTotalObjects: null this");
	ASSERT(0 < this->totalObjects, "ObjectSprite::getTotalObjects: null totalObjects");

	return this->totalObjects;
}

/**
 * Retrieved the OBJECT index
 *
 * @memberof			ObjectSprite
 * @public
 *
 * @param this			Function scope
 *
 * @return				Number of used OBJECTs
 */
s16 ObjectSprite_getObjectIndex(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getObjectIndex: null this");

	return this->objectIndex;
}

/**
 * Set the OBJECT index
 *
 * @memberof				ObjectSprite
 * @public
 *
 * @param this				Function scope
 * @param objectIndex		Set the OBJECT index
 */
void ObjectSprite_setObjectIndex(ObjectSprite this, s16 objectIndex)
{
	ASSERT(this, "ObjectSprite::setObjectIndex: null this");
	ASSERT(this->texture, "ObjectSprite::setObjectIndex: null texture");

	int previousObjectIndex = this->objectIndex;
	this->objectIndex = objectIndex;

	if(0 <= this->objectIndex)
	{
		// rewrite texture
		ObjectTexture_setObjectIndex(__SAFE_CAST(ObjectTexture, this->texture), this->objectIndex);

		if(0 <= previousObjectIndex)
		{
			// hide the previously used objects
			int j = previousObjectIndex;
			for(; j < previousObjectIndex + this->totalObjects; j++)
			{
				_objectAttributesBaseAddress[(j << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
			}

			if(!this->hidden)
			{
				__VIRTUAL_CALL(Sprite, show, this);

				// turn off previous OBJs' to avoid ghosting
				if(this->objectIndex < previousObjectIndex)
				{
					int counter = 0;
					int j = previousObjectIndex + this->totalObjects - 1;
					for(; j >= this->objectIndex + this->totalObjects && counter < this->totalObjects; j--, counter++)
					{
						_objectAttributesBaseAddress[(j << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
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
 * @param this	Function scope
 *
 * @return 		World layer
 */
u8 ObjectSprite_getWorldLayer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getWorldLayer: null this");
	ASSERT(this->objectSpriteContainer, "ObjectSprite::getWorldLayer: null objectSpriteContainer");

	return this->objectSpriteContainer ? __VIRTUAL_CALL(Sprite, getWorldLayer, __SAFE_CAST(Sprite, this->objectSpriteContainer)) : 0;
}

/**
 * Add displacement to position
 *
 * @memberof				ObjectSprite
 * @public
 *
 * @param this				Function scope
 * @param displacement		2D position displacement
 */
void ObjectSprite_addDisplacement(ObjectSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "ObjectSprite::addDisplacement: null this");

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
 * @param this		Function scope
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSprite_setMode(ObjectSprite this __attribute__ ((unused)), u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
{
	ASSERT(this, "ObjectSprite::setMode: null this");
}

/**
 * Set ObjectSpriteContainer to NULL
 *
 * @memberof				ObjectSprite
 * @public
 *
 * @param this				Function scope
 */
void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::invalidateObjectSpriteContainer: null this");

	this->objectSpriteContainer = NULL;
}
