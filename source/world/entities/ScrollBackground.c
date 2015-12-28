/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ScrollBackground.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>
#include <Game.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ScrollBackground
__CLASS_DEFINITION(ScrollBackground, Entity);

enum ScrollSprites
{
	kLeftSprite = 0,
	kRightSprite
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern const Optical* _optical;
extern const VBVec3D * _screenPosition;
extern const VBVec3D* _screenDisplacement;

static void ScrollBackground_updateScrolling(ScrollBackground this);
static void ScrollBackground_retrieveSprites(ScrollBackground this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ScrollBackground, ScrollBackgroundDefinition* backgroundDefinition, s16 id, const char* const name)
__CLASS_NEW_END(ScrollBackground, backgroundDefinition, id, name);

// class's constructor
void ScrollBackground_constructor(ScrollBackground this, ScrollBackgroundDefinition* scrollBackgroundDefinition, s16 id, const char* const name)
{
	ASSERT(this, "ScrollBackground::constructor: null this");
	ASSERT(scrollBackgroundDefinition, "ScrollBackground::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(scrollBackgroundDefinition, id, name);

	this->size.x = __SCREEN_WIDTH;
	this->size.y = __SCREEN_HEIGHT;
	this->size.z = 1;
}

// class's destructor
void ScrollBackground_destructor(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// initialize from definition
void ScrollBackground_initialize(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::initialize: null this");

	Entity_initialize(__SAFE_CAST(Entity, this));

	ASSERT(this->sprites, "ScrollBackground::constructor: null sprite list");

	ScrollBackground_retrieveSprites(this);

	BgmapSprite bSprite = this->scrollBgmapSprites[kRightSprite];
	
	if(bSprite)
	{
		BgmapTexture bTtexture = __SAFE_CAST(BgmapTexture, Sprite_getTexture(__SAFE_CAST(Sprite, bSprite)));

		if(bTtexture)
		{
			this->size.y = (u16)Texture_getRows(__SAFE_CAST(Texture, bTtexture)) << 3;
		}
	}
}

// get the sprites so I can manipulate them
static void ScrollBackground_retrieveSprites(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::retrieveSprites: null this");

	VirtualNode node = VirtualList_begin(this->sprites);
	int i = 0;

	for(; node && i <= kRightSprite ; node = VirtualNode_getNext(node), i++)
	{
		this->scrollBgmapSprites[i] = VirtualNode_getData(node);

		ASSERT(__SAFE_CAST(BgmapSprite, this->scrollBgmapSprites[i]), "ScrollBackground::constructor: no sprite added to list")
	}
}

// initial transform
void ScrollBackground_initialTransform(ScrollBackground this, Transformation* environmentTransform)
{
	ASSERT(this, "ScrollBackground::transform: null this");

	// call base class's transform method
	Entity_initialTransform(__SAFE_CAST(Entity, this), environmentTransform);

	ScrollBackground_updateScrolling(this);
}

// transform class
void ScrollBackground_transform(ScrollBackground this, const Transformation* environmentTransform)
{
	ASSERT(this, "ScrollBackground::transform: null this");

	// don't calling base class's transform method
	// will improve performance
#ifdef __STAGE_EDITOR
	if(Game_isInSpecialMode(Game_getInstance()))
	{
		Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);
	}
#endif

	if(_screenDisplacement->x || _screenDisplacement->y || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z)
	{
		ScrollBackground_updateScrolling(this);
	}
}

// calculate the scroll's screen position
static void ScrollBackground_updateScrolling(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::updateScrolling: null this");

	// TODO: add proper comments
	// TODO: this needs serious improvements
	DrawSpec drawSpec0 = BgmapSprite_getDrawSpec(this->scrollBgmapSprites[kRightSprite]);
	drawSpec0.position.z = this->transform.globalPosition.z;

	DrawSpec drawSpec1 = BgmapSprite_getDrawSpec(this->scrollBgmapSprites[kLeftSprite]);
	drawSpec1.position.z = this->transform.globalPosition.z;

	VBVec3D position3D = {_screenPosition->x, -_screenPosition->y + this->transform.globalPosition.y, this->transform.globalPosition.z};

	// get the screen's position
	VBVec2D screenPosition;

	int screens = 0;

	// axis to put one map o each side of it
	int axis = 0;
	int factor = 1;
	int displacement = 0;

	// project to 2d coordinates
	__OPTICS_PROJECT_TO_2D(position3D, screenPosition);

	// get the number of "screens" from the beginnig of the world
	// to the actual screen's position
	screens = FIX19_13TOI(screenPosition.x) / __SCREEN_WIDTH;

	// check if the number of screens is divisible by 2
	//if(!(screens & 2)1 == 0 && screens != 0){
	displacement = FIX19_13TOI(screenPosition.x);

	if(screens)
	{
		displacement -= ( screens - 1) * __SCREEN_WIDTH;

		if(!(screens & 1))
	{
			// if so,
			factor = 2;
		}
	}

	axis = __SCREEN_WIDTH * factor - displacement;

	if((unsigned)axis <= __SCREEN_WIDTH)
	{
		drawSpec0.position.x = ITOFIX19_13(axis - __SCREEN_WIDTH);

		drawSpec1.position.x = ITOFIX19_13(axis);
	}
	else
	{
		if(axis < 0)
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
	drawSpec0.position.y = drawSpec1.position.y = screenPosition.y - ITOFIX19_13(Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, this->scrollBgmapSprites[kLeftSprite]))) << 2);
	drawSpec0.position.parallax = drawSpec1.position.parallax = BgmapSprite_getDrawSpec(this->scrollBgmapSprites[kLeftSprite]).position.parallax;

	// set map's position
	ASSERT(this->scrollBgmapSprites[kLeftSprite], "ScrollBackground::updateScrolling: null kLeftSprite sprite");
	ASSERT(this->scrollBgmapSprites[kRightSprite], "ScrollBackground::updateScrolling: null kRightSprite sprite");

	BgmapSprite_setDrawSpec(this->scrollBgmapSprites[kRightSprite], &drawSpec0);
	Sprite_setRenderFlag(__SAFE_CAST(Sprite, this->scrollBgmapSprites[kRightSprite]), __UPDATE_G);

	BgmapSprite_setDrawSpec(this->scrollBgmapSprites[kLeftSprite], &drawSpec1);
	Sprite_setRenderFlag(__SAFE_CAST(Sprite, this->scrollBgmapSprites[kLeftSprite]), __UPDATE_G);
}

// whether it is visible
bool ScrollBackground_isVisible(ScrollBackground this, int pad)
{
	ASSERT(this, "ScrollBackground::isVisible: null this");

	return true;
}

// check if necessary to update sprite's position
bool ScrollBackground_updateSpritePosition(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::updateSpritePosition: null this");

	return false;
}

void ScrollBackground_suspend(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::suspend: null this");

	Entity_suspend(__SAFE_CAST(Entity, this));

	int i = 0;

	for(; i <= kRightSprite ; i++)
	{
		this->scrollBgmapSprites[i] = NULL;
	}
}

void ScrollBackground_resume(ScrollBackground this)
{
	ASSERT(this, "ScrollBackground::resume: null this");

	Entity_resume(__SAFE_CAST(Entity, this));

	ScrollBackground_retrieveSprites(this);
	
	Entity_translateSprites(__SAFE_CAST(Entity, this), true, true);
}
