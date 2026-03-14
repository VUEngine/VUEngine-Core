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
	TileSetManager tileSetManager = TileSetManager::getInstance();
	Printer::text("TILE MEMORY USAGE", x, y++, NULL);

	Printer::text("Total TileSets:        ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(tileSetManager->tileSets), x + 18, y, NULL);
	Printer::text("Total used chars:      ", x, ++y, NULL);
	Printer::int32(TileSetManager::getTotalUsedChars(tileSetManager), x + 18, y, NULL);
	Printer::text("Total free chars:      ", x, ++y, NULL);
	Printer::int32(TileSetManager::getTotalFreeChars(tileSetManager), x + 18, y, NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::reset()
{
	if(this->tileSets)
	{
		VirtualList::deleteData(this->tileSets);
	}

	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::clearGraphicMemory()
{
	Mem::clear((uint8*) __TILE_SPACE_BASE_ADDRESS, 8192 * 4);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TileSetManager::loadTileSets(const TileSetSpec** tileSetSpecs)
{
	if(NULL != tileSetSpecs)
	{
		for(int16 i = 0; tileSetSpecs[i]; i++)
		{
			if(tileSetSpecs[i]->shared)
			{
				TileSetManager::getTileSet(this, (TileSetSpec*)tileSetSpecs[i]);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::getTileSet(const TileSetSpec* tileSetSpec)
{
	NM_ASSERT(NULL != tileSetSpec, "TileSetManager::getTileSet: NULL tileSetSpec");

	if(NULL == tileSetSpec)
	{
		return NULL;
	}

	TileSet tileSet = NULL;

	if(!tileSetSpec->shared)
	{
		// Ask for allocation
		tileSet = TileSetManager::allocateTileSet(this, tileSetSpec);
	}
	else
	{
		// First try to find an already created tileSet
		tileSet = TileSetManager::findTileSet(this, tileSetSpec);

		if(NULL == tileSet)
		{
			tileSet = TileSetManager::allocateTileSet(this, tileSetSpec);
		}
		else
		{
			TileSet::increaseUsageCount(tileSet);
		}
	}

	return tileSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool TileSetManager::releaseTileSet(TileSet tileSet)
{
	if(isDeleted(tileSet))
	{
		return false;
	}

	if(TileSet::decreaseUsageCount(tileSet))
	{
		VirtualList::removeData(this->tileSets, tileSet);

		uint32 offset = TileSet::getOffset(tileSet);

		if(1 == this->freedOffset || offset < this->freedOffset)
		{
			this->freedOffset = offset;
		}

		delete tileSet;

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
			VirtualNode node = this->tileSets->head;

			for(; NULL != node; node = node->next)
			{
				TileSet tileSet = TileSet::safeCast(node->data);

				NM_ASSERT(!isDeleted(tileSet), "TileSetManager::defragment: deleted tileSet");

				uint32 offset = TileSet::getOffset(tileSet);

				if(this->freedOffset < offset)
				{
					TileSet::setOffset(tileSet, this->freedOffset);
					this->freedOffset += TileSet::getNumberOfChars(tileSet);
					break;
				}
				else if(this->freedOffset == offset)
				{
					this->freedOffset += TileSet::getNumberOfChars(tileSet);
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
	ASSERT(this->tileSets, "TileSetManager::getTotalFreeChars: null tileSets list");

	TileSet lastTileSet = VirtualList::back(this->tileSets);
	return (int32)TileSet::getOffset(lastTileSet) + TileSet::getNumberOfChars(lastTileSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 TileSetManager::getTotalFreeChars()
{
	return __TOTAL_TILES - TileSetManager::getTotalUsedChars(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 TileSetManager::getTotalTileSets()
{
	return VirtualList::getCount(this->tileSets);
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

	this->tileSets = new VirtualList();
	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSetManager::destructor()
{
	TileSetManager::reset(this);

	delete this->tileSets;
	this->tileSets = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::findTileSet(const TileSetSpec* tileSetSpec)
{
	CACHE_RESET;

	for(VirtualNode node = this->tileSets->head; NULL != node; node = node->next)
	{
		TileSet tileSet = TileSet::safeCast(node->data);

		if(!isDeleted(tileSet) && tileSet->tileSetSpec->tiles == tileSetSpec->tiles && tileSet->tileSetSpec->shared == tileSetSpec->shared)
		{
			return tileSet;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

TileSet TileSetManager::allocateTileSet(const TileSetSpec* tileSetSpec)
{
	NM_ASSERT(!isDeleted(this->tileSets), "TileSetManager::allocateTileSet: null tileSets");
	NM_ASSERT(tileSetSpec, "TileSetManager::allocateTileSet: null tileSetSpec");
	NM_ASSERT(tileSetSpec->numberOfChars > 0, "TileSetManager::allocateTileSet: number of chars < 0");
	NM_ASSERT(tileSetSpec->numberOfChars < __TOTAL_TILES, "TileSetManager::allocateTileSet: too many chars in spec");

	uint16 offset = NULL != this->tileSets->head ? 0 : 1;

	if(NULL != this->tileSets->head)
	{
		TileSet lastTileSet = TileSet::safeCast(VirtualList::back(this->tileSets));
		offset += TileSet::getOffset(lastTileSet) + TileSet::getNumberOfChars(lastTileSet);
	}

	if((unsigned)offset + tileSetSpec->numberOfChars < __TOTAL_TILES)
	{
		TileSet tileSet = new TileSet(tileSetSpec, offset);

		VirtualList::pushBack(this->tileSets, tileSet);

		return tileSet;
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

	for(VirtualNode node = this->tileSets->head; NULL != node; node = node->next)
	{
		TileSet::write(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
