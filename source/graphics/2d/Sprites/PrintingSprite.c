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

#include <PrintingSprite.h>
#include <Affine.h>
#include <Game.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <Camera.h>
#include <VIPManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof						PrintingSprite
 * @public
 *
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void PrintingSprite::constructor(const PrintingSpriteSpec* printingSpriteSpec, Object owner)
{
	Base::constructor(&printingSpriteSpec->bgmapSpriteSpec, owner);

	PrintingSprite::reset(this);

	PrintingSprite::registerWithManager(this);
}

/**
 * Class denstructor
 *
 * @memberof			PrintingSprite
 * @public
 */
void PrintingSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void PrintingSprite::setMode(u16 display __attribute__((unused)), u16 mode __attribute__((unused)))
{}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		PrintingSprite
 * @public
 *
 * @param evenFrame
 */
u16 PrintingSprite::doRender(s16 index, bool evenFrame __attribute__((unused)))
{
	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	worldPointer->mx = this->drawSpec.textureSource.mx;
	worldPointer->mp = this->drawSpec.textureSource.mp;
	worldPointer->my = this->drawSpec.textureSource.my;
	worldPointer->gx = this->position.x;
	worldPointer->gp = this->position.parallax;
	worldPointer->gy = this->position.y;
	worldPointer->w = this->w;
	worldPointer->h = this->h;
	worldPointer->head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	return index;
}

void PrintingSprite::reset()
{
	this->positioned = true;

	this->position.x = 0;
	this->position.y = 0;
	this->position.parallax = 0;

	this->drawSpec.textureSource.mx = __PRINTING_BGMAP_X_OFFSET;
	this->drawSpec.textureSource.my = __PRINTING_BGMAP_Y_OFFSET;
	this->drawSpec.textureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

	this->w = __SCREEN_WIDTH - 1;
	this->h = __SCREEN_HEIGHT - 1;
}

void PrintingSprite::setGValues(s16 gx, s16 gy, s16 gp)
{
	this->position.x = gx;
	this->position.y = gy;
	this->position.parallax = gp;
}

void PrintingSprite::setMValues(s16 mx, s16 my, s16 mp)
{
	this->drawSpec.textureSource.mx = mx;
	this->drawSpec.textureSource.my = my;
	this->drawSpec.textureSource.mp = mp;
}

void PrintingSprite::setSize(u16 w, u16 h)
{
	this->w = w;
	this->h = h;
}

s16 PrintingSprite::getGX()
{
	return this->position.x;
}

s16 PrintingSprite::getGY()
{
	return this->position.y;
}

s16 PrintingSprite::getGP()
{
	return this->position.parallax;
}