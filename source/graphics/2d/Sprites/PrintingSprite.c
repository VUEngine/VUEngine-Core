/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <PrintingSprite.h>
#include <Affine.h>
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
void PrintingSprite::constructor(const PrintingSpriteSpec* printingSpriteSpec, ListenerObject owner)
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

void PrintingSprite::setMode(uint16 display __attribute__((unused)), uint16 mode __attribute__((unused)))
{}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		PrintingSprite
 * @public
 *
 * @param evenFrame
 */
int16 PrintingSprite::doRender(int16 index, bool evenFrame __attribute__((unused)))
{
	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	worldPointer->mx = this->textureSource.mx;
	worldPointer->mp = this->textureSource.mp;
	worldPointer->my = this->textureSource.my;
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

	this->textureSource.mx = __PRINTING_BGMAP_X_OFFSET;
	this->textureSource.my = __PRINTING_BGMAP_Y_OFFSET;
	this->textureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

	this->w = __SCREEN_WIDTH - 1;
	this->h = __SCREEN_HEIGHT - 1;
}

void PrintingSprite::setGValues(int16 gx, int16 gy, int16 gp)
{
	this->position.x = gx;
	this->position.y = gy;
	this->position.parallax = gp;
}

void PrintingSprite::setMValues(int16 mx, int16 my, int16 mp)
{
	this->textureSource.mx = mx;
	this->textureSource.my = my;
	this->textureSource.mp = mp;
}

void PrintingSprite::setSize(uint16 w, uint16 h)
{
	this->w = w;
	this->h = h;
}

int16 PrintingSprite::getGX()
{
	return this->position.x;
}

int16 PrintingSprite::getGY()
{
	return this->position.y;
}

int16 PrintingSprite::getGP()
{
	return this->position.parallax;
}