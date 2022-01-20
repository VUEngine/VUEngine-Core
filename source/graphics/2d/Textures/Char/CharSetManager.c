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

#include <CharSetManager.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Mem.h>
#include <HardwareManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

#ifndef __RELEASE
friend class CharSet;
#endif

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CharSetManager::getInstance()
 * @memberof	CharSetManager
 * @public
 * @return		CharSetManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void CharSetManager::constructor()
{
	Base::constructor();

	this->charSets = new VirtualList();
	this->charSetsPendingWriting = new VirtualList();
	this->freedOffset = 1;
	this->preventDefragmentation = false;
}

/**
 * Class destructor
 */
void CharSetManager::destructor()
{
	CharSetManager::reset(this);

	delete this->charSets;
	this->charSets = NULL;

	delete this->charSetsPendingWriting;
	this->charSetsPendingWriting = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset manager's state
 */
void CharSetManager::reset()
{
	this->preventDefragmentation = true;

	if(this->charSets)
	{
		VirtualNode node = this->charSets->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		VirtualList::clear(this->charSets);
	}

	VirtualList::clear(this->charSetsPendingWriting);

	this->freedOffset = 1;
	this->preventDefragmentation = false;
}

/**
 * Find a previously registered CharSet with the given spec
 *
 * @private
 * @param charSetSpec		CharSet spec
 */
CharSet CharSetManager::findCharSet(CharSetSpec* charSetSpec)
{
	// try to find a charset with the same char spec
	VirtualNode node = this->charSets->head;

	for(; node; node = node->next)
	{
		CharSet charSet = CharSet::safeCast(node->data);

		if(charSet && CharSet::getCharSetSpec(charSet)->tiles == charSetSpec->tiles && CharSet::getAllocationType(charSet) == charSetSpec->allocationType)
		{
			return charSet;
		}
	}

	return NULL;
}

/**
 * Retrieve a CharSet
 *
 * @private
 * @param charSetSpec				CharSet spec to find o allocate a CharSet
 * @return 								Allocated CharSet
 */
CharSet CharSetManager::getCharSet(CharSetSpec* charSetSpec)
{
	CharSet charSet = NULL;

	switch(charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:

			// ask for allocation
			charSet = CharSetManager::allocateCharSet(CharSetManager::getInstance(), charSetSpec);
			break;

		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
		case __ANIMATED_MULTI:
		case __NOT_ANIMATED:

			// first try to find an already created charset
			charSet = CharSetManager::findCharSet(this, charSetSpec);

			if(charSet)
			{
				CharSet::increaseUsageCount(charSet);
			}
			else
			{
				charSet = CharSetManager::allocateCharSet(this, charSetSpec);
			}

			break;

		default:

			ASSERT(false, "CharSet::write: with no allocation type");
			break;
	}

	return charSet;
}

/**
 * Release a previously allocated CharSet
 *
 * @param charSet			CharSet to release
 */
bool CharSetManager::releaseCharSet(CharSet charSet)
{
	if(isDeleted(charSet))
	{
		return true;
	}

	if(CharSet::decreaseUsageCount(charSet))
	{
		this->preventDefragmentation = true;

		VirtualList::removeElement(this->charSets, charSet);
		VirtualList::removeElement(this->charSetsPendingWriting, charSet);

		uint32 offset = CharSet::getOffset(charSet);

		if(1 == this->freedOffset || offset < this->freedOffset)
		{
			this->freedOffset = offset;
		}

		delete charSet;

		this->preventDefragmentation = false;

		return true;
	}

	return false;
}

/**
 * Try to allocate a CHAR memory space for a new CharSet
 *
 * @private
 * @param charSetSpec		CharSet spec to allocate space for
 * @return 						Allocated CharSet
 */
CharSet CharSetManager::allocateCharSet(CharSetSpec* charSetSpec)
{
	NM_ASSERT(this->charSets, "CharSetManager::allocateCharSet: null this");
	NM_ASSERT(charSetSpec, "CharSetManager::allocateCharSet: null charSetSpec");
	NM_ASSERT(charSetSpec->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");
	NM_ASSERT(charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS, "CharSetManager::allocateCharSet: too many chars in spec");

	this->preventDefragmentation = true;

	uint16 offset = NULL != this->charSets->head ? 0 : 1;

	if(this->charSets->head)
	{
		CharSet lastCharSet = CharSet::safeCast(VirtualList::back(this->charSets));
		offset += CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
	}

	if((unsigned)offset + charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS)
	{
		CharSet charSet = new CharSet(charSetSpec, offset);

		VirtualList::pushBack(this->charSets, charSet);
		VirtualList::pushBack(this->charSetsPendingWriting, charSet);

		this->preventDefragmentation = false;
		return charSet;
	}

// TODO: implement __CHAR_FORCE_LOADING in config.h file
#ifdef __CHAR_FORCE_LOADING
	else
	{
		CharSet charSet = new CharSet(charSetSpec, __CHAR_MEMORY_TOTAL_CHARS - charSetSpec->numberOfChars);

		VirtualList::pushBack(this->charSets, charSet);
		VirtualList::pushBack(this->charSetsPendingWriting, charSet);

		this->preventDefragmentation = false;
		return charSet;		
	}
#endif

	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	CharSetManager::print(this, 1, 10);
	Printing::text(Printing::getInstance(), "CharSet ", 1, 19, NULL);
	Printing::text(Printing::getInstance(), "    Address: ", 1, 21, NULL);
	Printing::hex(Printing::getInstance(), (WORD)&charSetSpec, 14, 21, 8, NULL);
	Printing::text(Printing::getInstance(), "    Size: ", 1, 22, NULL);
	Printing::int32(Printing::getInstance(), charSetSpec->numberOfChars, 14, 22, NULL);

	// if there isn't enough memory thrown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: CHAR mem depleted");

	return NULL;
}

/**
 * Write char sets pending writing
 */
void CharSetManager::writeCharSets()
{
	this->preventDefragmentation = true;

	CharSetManager::defragment(this);

	VirtualNode node = this->charSetsPendingWriting->head;

	for(; node; node = node->next)
	{
		CharSet::write(node->data);
	}

	VirtualList::clear(this->charSetsPendingWriting);

	this->preventDefragmentation = false;
}

/**
 * Write char sets pending writing
 */
bool CharSetManager::writeCharSetsProgressively()
{
	if(this->preventDefragmentation)
	{
		return false;
	}

	CharSet charSet = VirtualList::front(this->charSetsPendingWriting);

	if(!isDeleted(charSet))
	{
		CharSet::write(charSet);
		VirtualList::popFront(this->charSetsPendingWriting);
		return true;
	}
	else
	{
		NM_ASSERT(0 == VirtualList::front(this->charSetsPendingWriting), "CharSetManager::writeCharSetsProgressively: null charset in list");
	}

	// do some defragmenting
    return CharSetManager::defragmentProgressively(this);
}

/**
 * Deframent CHAR memory
 */
void CharSetManager::defragment()
{
	this->preventDefragmentation = true;
	
	while(1 < this->freedOffset)
	{
		CharSetManager::defragmentProgressively(this);
	}

	this->preventDefragmentation = false;
}

/**
 * Deframent CHAR memory progressively
 */
bool CharSetManager::defragmentProgressively()
{
	if(1 < this->freedOffset)
	{
		VirtualNode node = this->charSets->head;

		for(; node; node = node->next)
		{
			CharSet charSet = CharSet::safeCast(node->data);

			if(!isDeleted(charSet))
			{
				uint32 offset = CharSet::getOffset(charSet);

				if(this->freedOffset < offset)
				{
	#ifndef __RELEASE
					for(WORD* x = (WORD*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)charSet->offset) << 4)); x < (WORD*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)charSet->offset) << 4)) + __BYTES_PER_CHARS(charSet->charSetSpec->numberOfChars) / sizeof(WORD); x++)
					{
						*x = 0;
					}
	#endif
					CharSet::setOffset(charSet, this->freedOffset);

					//write to CHAR memory
					CharSet::rewrite(charSet);
					this->freedOffset += CharSet::getNumberOfChars(charSet);

					VirtualList::removeNode(this->charSetsPendingWriting, node);
					return true;
				}
				else if(this->freedOffset == offset)
				{
					this->freedOffset += CharSet::getNumberOfChars(charSet);
				}
			}
		}

		this->freedOffset = 1;
	}

	return false;
}

/**
 * Retrieve the total number of used CHARs
 *
 * @return 				Total number of used CHARs
 */
int32 CharSetManager::getTotalUsedChars()
{
	ASSERT(this->charSets, "CharSetManager::getTotalFreeChars: null charSets list");

	CharSet lastCharSet = VirtualList::back(this->charSets);
	return (int32)CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
}

/**
 * Retrieve the total number of free CHARs
 *
 * @return 				Total number of free CHARs
 */
int32 CharSetManager::getTotalFreeChars()
{
	return __CHAR_MEMORY_TOTAL_CHARS - CharSetManager::getTotalUsedChars(this);
}

/**
 * Retrieve the total number of registered char sets
 *
 * @return 				Total number of registered char sets
 */
int32 CharSetManager::getTotalCharSets()
{
	return VirtualList::getSize(this->charSets);
}

/**
 * Print manager's state
 *
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void CharSetManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "CHAR MEMORY USAGE", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Total CharSets:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->charSets), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Total used chars:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CharSetManager::getTotalUsedChars(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Total free chars:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CharSetManager::getTotalFreeChars(this), x + 18, y, NULL);
}
