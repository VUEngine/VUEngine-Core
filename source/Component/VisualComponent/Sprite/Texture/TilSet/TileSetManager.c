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

#include <TileSet.h>
#include <Mem.h>
#include <Printer.h>
#include <Singleton.h>
#include <VirtualList.h>

#include "TileSetManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class TileSet;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TileSetManager::print(int32 x __attribute__((unused)), int32 y __attribute__((unused)))
{
#ifndef __SHIPPING
	TileSetManager charSetManager = TileSetManager::getInstance();
	Printer::text("TILE MEMORY USAGE", x, y++, NULL);

	Printer::text("Total TileSets:        ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(charSetManager->charSets), x + 18, y, NULL);
	Printer::text("Total used chars:      ", x, ++y, NULL);
	Printer::int32(TileSetManager::getTotalUsedChars(charSetManager), x + 18, y, NULL);
	Printer::text("Total free chars:      ", x, ++y, NULL);
	Printer::int32(TileSetManager::getTotalFreeChars(charSetManager), x + 18, y, NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::reset()
{
	if(this->charSets)
	{
		VirtualList::deleteData(this->charSets);
	}

	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::clearGraphicMemory()
{
	Mem::clear((uint8*) __TILE_SPACE_BASE_ADDRESS, 8192 * 4);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::loadTileSets(const TileSetSpec** charSetSpecs)
{
	if(NULL != charSetSpecs)
	{
		for(int16 i = 0; charSetSpecs[i]; i++)
		{
			if(charSetSpecs[i]->shared)
			{
				TileSetManager::getTileSet(this, (TileSetSpec*)charSetSpecs[i]);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::getTileSet(const TileSetSpec* charSetSpec)
{
	NM_ASSERT(NULL != charSetSpec, "TileSetManager::getTileSet: NULL charSetSpec");

	if(NULL == charSetSpec)
	{
		return NULL;
	}

	TileSet charSet = NULL;

	if(!charSetSpec->shared)
	{
		// Ask for allocation
		charSet = TileSetManager::allocateTileSet(this, charSetSpec);
	}
	else
	{
		// First try to find an already created tileSet
		charSet = TileSetManager::findTileSet(this, charSetSpec);

		if(NULL == charSet)
		{
			charSet = TileSetManager::allocateTileSet(this, charSetSpec);
		}
		else
		{
			TileSet::increaseUsageCount(charSet);
		}
	}

	return charSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool TileSetManager::releaseTileSet(TileSet charSet)
{
	if(isDeleted(charSet))
	{
		return false;
	}

	if(TileSet::decreaseUsageCount(charSet))
	{
		VirtualList::removeData(this->charSets, charSet);

		uint32 offset = TileSet::getOffset(charSet);

		if(1 == this->freedOffset || offset < this->freedOffset)
		{
			this->freedOffset = offset;
		}

		delete charSet;

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::defragment(bool deferred)
{
	if(1 < this->freedOffset)
	{
		do
		{
			VirtualNode node = this->charSets->head;

			for(; NULL != node; node = node->next)
			{
				TileSet charSet = TileSet::safeCast(node->data);

				NM_ASSERT(!isDeleted(charSet), "TileSetManager::defragment: deleted tileSet");

				uint32 offset = TileSet::getOffset(charSet);

				if(this->freedOffset < offset)
				{
					TileSet::setOffset(charSet, this->freedOffset);
					this->freedOffset += TileSet::getNumberOfChars(charSet);
					break;
				}
				else if(this->freedOffset == offset)
				{
					this->freedOffset += TileSet::getNumberOfChars(charSet);
				}
			}

			if(NULL == node)
			{
				this->freedOffset = 1;
			}
		}
		while(!deferred && 1 < this->freedOffset);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 TileSetManager::getTotalUsedChars()
{
	ASSERT(this->charSets, "TileSetManager::getTotalFreeChars: null charSets list");

	TileSet lastTileSet = VirtualList::back(this->charSets);
	return (int32)TileSet::getOffset(lastTileSet) + TileSet::getNumberOfChars(lastTileSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 TileSetManager::getTotalFreeChars()
{
	return __TILE_MEMORY_TOTAL_TILES - TileSetManager::getTotalUsedChars(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 TileSetManager::getTotalTileSets()
{
	return VirtualList::getCount(this->charSets);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSetManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->charSets = new VirtualList();
	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSetManager::destructor()
{
	TileSetManager::reset(this);

	delete this->charSets;
	this->charSets = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::findTileSet(const TileSetSpec* charSetSpec)
{
	CACHE_RESET;

	for(VirtualNode node = this->charSets->head; NULL != node; node = node->next)
	{
		TileSet charSet = TileSet::safeCast(node->data);

		if(!isDeleted(charSet) && charSet->charSetSpec->tiles == charSetSpec->tiles && charSet->charSetSpec->shared == charSetSpec->shared)
		{
			return charSet;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::allocateTileSet(const TileSetSpec* charSetSpec)
{
	NM_ASSERT(!isDeleted(this->charSets), "TileSetManager::allocateTileSet: null charSets");
	NM_ASSERT(charSetSpec, "TileSetManager::allocateTileSet: null charSetSpec");
	NM_ASSERT(charSetSpec->numberOfChars > 0, "TileSetManager::allocateTileSet: number of chars < 0");
	NM_ASSERT(charSetSpec->numberOfChars < __TILE_MEMORY_TOTAL_TILES, "TileSetManager::allocateTileSet: too many chars in spec");

	uint16 offset = NULL != this->charSets->head ? 0 : 1;

	if(NULL != this->charSets->head)
	{
		TileSet lastTileSet = TileSet::safeCast(VirtualList::back(this->charSets));
		offset += TileSet::getOffset(lastTileSet) + TileSet::getNumberOfChars(lastTileSet);
	}

	if((unsigned)offset + charSetSpec->numberOfChars < __TILE_MEMORY_TOTAL_TILES)
	{
		TileSet charSet = new TileSet(charSetSpec, offset);

		VirtualList::pushBack(this->charSets, charSet);

		return charSet;
	}

#ifdef __ALERT_TILE_MEMORY_DEPLETION
	Printer::setDebugMode();
	Printer::clear();

	NM_ASSERT(false, "TileSetManager::allocateTileSet: TILE mem depleted");
#endif

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::writeTileSets()
{
	TileSetManager::defragment(this, false);

	for(VirtualNode node = this->charSets->head; NULL != node; node = node->next)
	{
		TileSet::write(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
