/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackground.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the MBackground
__CLASS_DEFINITION(MBackground);

enum ScrollSprites
{
	kLeftSprite = 0,
	kRightSprite
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern const VBVec3D * _screenPosition;
extern const VBVec3D* _screenDisplacement;

static void MBackground_updateScrolling(MBackground this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBackground, __PARAMETERS(MBackgroundDefinition* mBackgroundDefinition, s16 id))
__CLASS_NEW_END(MBackground, __ARGUMENTS(mBackgroundDefinition, id));

// class's constructor
void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id)
{
	ASSERT(this, "MBackground::constructor: null this");
	ASSERT(mBackgroundDefinition, "MBackground::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(Entity, __ARGUMENTS((EntityDefinition*)mBackgroundDefinition, id));
	
	ASSERT(this->sprites, "MBackground::constructor: null sprite list");
}

// class's destructor
void MBackground_destructor(MBackground this)
{
	ASSERT(this, "MBackground::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Entity);
}

// initial transform
void MBackground_initialTransform(MBackground this, Transformation* environmentTransform)
{
	ASSERT(this, "MBackground::transform: null this");

	// call base class's transform method
	Entity_transform((Entity)this, environmentTransform);

	//MBackground_updateScrolling(this);
}

// transform class
void MBackground_transform(MBackground this, Transformation* environmentTransform)
{
	ASSERT(this, "MBackground::transform: null this");

	// don't calling base class's transform method
	// will improve performance
#ifdef __STAGE_EDITOR
	if (Game_isInSpecialMode(Game_getInstance()))
	{
		Entity_transform((Entity)this, environmentTransform);
	}
#endif

	if (_screenDisplacement->x || _screenDisplacement->y || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z)
	{
		//MBackground_updateScrolling(this);
	}
}

// calculate the scroll's screen position
static void MBackground_updateScrolling(MBackground this)
{
	ASSERT(this, "MBackground::updateScrolling: null this");

	CACHE_ENABLE;
	// TODO: add proper comments
	// TODO: this needs serious improvements
	DrawSpec drawSpec0 =
	{
			{0, 0, this->transform.globalPosition.z},
			{1, 1}
	};

	DrawSpec drawSpec1 = drawSpec0;

	VBVec3D position3D = {_screenPosition->x, -_screenPosition->y + this->transform.globalPosition.y, this->transform.globalPosition.z};

	// get the screen's position
	VBVec2D screenPosition;

	int screens = 0;

	// axis to put one map o each side of it
	int axis = 0;
	int factor = 1;
	int displacement = 0;

	// project position to 2D
	Optics_projectTo2D(&screenPosition, &position3D);

	// get the number of "screens" from the beginnig of the world
	// to the actual screen's position
	screens = FIX19_13TOI(screenPosition.x) / __SCREEN_WIDTH;

	// check if the number of screens is divisible by 2
	//if (!(screens & 2)1 == 0 && screens != 0){
	displacement = FIX19_13TOI(screenPosition.x);

	if (screens)
	{
		displacement -= ( screens - 1) * __SCREEN_WIDTH;

		if (!(screens & 1))
	{
			// if so,
			factor = 2;
		}
	}

	axis = __SCREEN_WIDTH * factor - displacement;

	if ((unsigned)axis <= __SCREEN_WIDTH)
	{
		drawSpec0.position.x = ITOFIX19_13(axis - __SCREEN_WIDTH);

		drawSpec1.position.x = ITOFIX19_13(axis);
	}
	else
	{
		if (axis < 0)
    	{
			drawSpec1.position.x = ITOFIX19_13(axis);

			drawSpec0.position.x = drawSpec1.position.x + ITOFIX19_13(__SCREEN_WIDTH);
		}
		else
	    {
			drawSpec0.position.x = ITOFIX19_13(axis - __SCREEN_WIDTH - 1);

			drawSpec1.position.x = drawSpec0.position.x - ITOFIX19_13(__SCREEN_WIDTH - 1);
		}
	}

	// now move the drawspec in order to render the texture in the center
	//drawSpec0.position.y = drawSpec1.position.y = screenPosition.y - ITOFIX19_13(Texture_getRows(Sprite_getTexture(this->scrollSprites[kLeftSprite])) << 2);
	//drawSpec0.position.parallax = drawSpec1.position.parallax = Sprite_getDrawSpec(this->scrollSprites[kLeftSprite]).position.parallax;

	// set map's position
	
	CACHE_DISABLE;
}

// whether it is visible
int MBackground_isVisible(MBackground this, int pad)
{
	ASSERT(this, "MBackground::isVisible: null this");

	return true;
}

// check if must update sprite's position
int MBackground_updateSpritePosition(MBackground this)
{
	ASSERT(this, "MBackground::updateSpritePosition: null this");

	return false;
}
