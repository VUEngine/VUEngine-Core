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

#include <string.h>

#include <BgmapTextureManager.h>
#include <CharSet.h>
#include <CharSetManager.h>
#include <ComponentManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <ParamTableManager.h>
#include <PrintingSprite.h>
#include <Singleton.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>

#include "Printer.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern FontData _fontData[];
extern FontROMSpec DefaultFontSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define VUENGINE_DEBUG_FONT_CHARSET_OFFSET		(__CHAR_MEMORY_TOTAL_CHARS - VUENGINE_DEBUG_FONT_SIZE)
#define VUENGINE_DEBUG_FONT_SIZE				160

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

FontData VUENGINE_DEBUG_FONT_DATA =
{
	// Font spec
	(FontSpec*)&DefaultFontSpec,

	// CharSet
	NULL,
};

const uint32 DefaultPrintingTiles[] __attribute__((aligned(4))) =
{
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

const uint16 DefaultPrintingMap[] __attribute__((aligned(4))) =
{
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

CharSetROMSpec DefaultPrintingCharsetSpec =
{
	// Number of CHARs in function of the number of frames to load at the same time
	1,

	// Whether it is shared or not
	true,

	// Whether the tiles are optimized or not
	false,

	// Tiles array
	(uint32*)DefaultPrintingTiles,

	// Frame offsets array
	NULL
};

const TextureROMSpec DefaultPrintingTextureSpec =
{
	// Pointer to the char spec that the texture uses
	(CharSetSpec*)&DefaultPrintingCharsetSpec,

	// Pointer to the map array that defines how to use the tiles from the char set
	(uint16*)DefaultPrintingMap,

	// Horizontal size in tiles of the texture (max. 64)
	64,

	// Vertical size in tiles of the texture (max. 64)
	28,

	// padding for affine/hbias transformations
	{0, 0},

	// Number of frames that the texture supports
	1,

	// Palette index to use by the graphical data (0 - 3)
	1,

	// Flag to recycle the texture with a different map
	false,

	// Flag to vertically flip the image
	false,

	// Flag to horizontally flip the image
	false
};

const PrintingSpriteSpec DefaultPrintingSpriteSpec =
{
	{
		{
			// VisualComponent
			{
				// Component
				{
					// Allocator
					__TYPE(PrintingSprite),

					// Component type
					kSpriteComponent
				},

				// Array of function animations
				(const AnimationFunction**)NULL
			},

			// Spec for the texture to display
			(TextureSpec*)&DefaultPrintingTextureSpec,

			// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
			__TRANSPARENCY_NONE,

			// Displacement added to the sprite's position
			{
				0, // x
				0, // y
				0, // z
				0, // parallax
			},
		},

		// Flag to indicate in which display to show the texture (__WORLD_ON, __WORLD_LON or __WORLD_RON)
		__WORLD_ON,

		// The display mode (__WORLD_BGMAP, __WORLD_AFFINE or __WORLD_HBIAS)
		__WORLD_BGMAP,

		// Pointer to affine/hbias manipulation function
		NULL
	}
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setDebugMode()
{
	Printer printing = Printer::getInstance();

	if(__PRINTING_MODE_DEBUG == printing->mode)
	{
		return;
	}

	printing->mode = __PRINTING_MODE_DEBUG;
	printing->lastUsedFontData = NULL;
	printing->lastUsedFont = NULL;
	Printer::loadDebugFont();
	Printer::clear();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::loadFonts(FontSpec** fontSpecs)
{
	Printer printing = Printer::getInstance();

	// Since fonts' charsets will be released, there is no reason to keep
	// Anything in the printing area
	Printer::clear();

	// Empty list of registered fonts
	Printer::releaseFonts();

	// Prevent VIP's interrupt from calling render during printing process
	HardwareManager::suspendInterrupts();

	/// Must force CHAR defragmentation
	CharSetManager::writeCharSets(CharSetManager::getInstance());

	// Iterate over all defined fonts and add to internal list
	// Preload charset for font if in list of fonts to preload
	if(NULL != fontSpecs)
	{
		for(int16 i = 0; NULL != fontSpecs[i]; i++)
		{
			// Find defined font in list of fonts to preload
			for(int16 j = 0; NULL != _fontData[j].fontSpec; j++)
			{
				// Preload charset and save charset reference, if font was found
				if(fontSpecs[i] == _fontData[j].fontSpec)
				{
					_fontData[j].charSet = CharSet::get(_fontData[j].fontSpec->charSetSpec);

					if(NULL != _fontData[j].charSet)
					{
						CharSet::addEventListener(_fontData[j].charSet, ListenerObject::safeCast(printing), kEventCharSetChangedOffset);
					}
				}
			}
		}
	}

	if(NULL == printing->activePrintingSprite)
	{
		Printer::addSprite(printing);
	}

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::releaseFonts()
{
	Printer printing = Printer::getInstance();

	Printer::removeEventListeners(printing, kEventFontRewritten);

	for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
	{
		// Preload charset and save charset reference, if font was found
		if(!isDeleted(_fontData[i].charSet))
		{
			CharSet::removeEventListener(_fontData[i].charSet, ListenerObject::safeCast(printing), kEventCharSetChangedOffset);

			while(!CharSet::release(_fontData[i].charSet));
		}

		_fontData[i].charSet = NULL;
	}

	printing->lastUsedFont = NULL;
	printing->lastUsedFontData = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::clear()
{
	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::clear(printing->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::clearRow(uint16 row)
{
	// TODO: implement something more elegant and performant
	Printer::text("                                                ", 0, row, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::text(const char* string, int32 x, int32 y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printer::out(x, y, Utilities::toUppercase(string), font);
#else
	Printer::out(x, y, string, font);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::int32(int32 value, uint8 x, uint8 y, const char* font)
{
	Printer::number(value, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::hex(WORD value, uint8 x, uint8 y, uint8 length, const char* font)
{
	Printer::out(x, y, Utilities::itoa((int32)(value), 16, length), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::float(float value, uint8 x, uint8 y, int32 precision, const char* font)
{
	if(1 > precision)
	{
		precision = 1;
	}
	else if(10 < precision)
	{
		precision = 10;
	}

	int32 decMultiplier = 1;

	int32 decimals = 0;

	for(; decimals < precision; decimals++)
	{
		decMultiplier *= 10;
	}

	// Round last precision digit
	value += (0.5f / (decMultiplier));

	char string[48] = "\0";

	int32 i = 0;

	// Handle negatives
	if(0 > value)
	{
		string[i] = '-';
		i++;

		value *= -1;
	}

	// Get integral part
	int32 floorValue = ((int32)(value * 10)) / 10;
	char* integer = Utilities::itoa(floorValue, 10, Math::getDigitsCount(floorValue));

	// Save it right away
	for(int32 j = 0; integer[j];)
	{
		string[i++] = integer[j++];
	}

	// Start decimal part
	string[i++] = '.';

	// Get decimal part
	float decimalValue = value - floorValue;

	// Promote to integral all the decimals up to precision
	decimalValue *= decMultiplier; 

	int32 zeros = 0;
	int32 flooredDecimalValue = (int32)Math::floor(decimalValue);

	while(10 <= decMultiplier)
	{
		decMultiplier /= 10;

		if(0 != (flooredDecimalValue / decMultiplier))
		{
			break;
		}

		string[i++] = '0';
		zeros++;
	}

	if(decimals <= precision && zeros < precision)
	{
		long roundedDecimalValue = (int32)(decimalValue * 10) / 10;

		if(0 == roundedDecimalValue)
		{
			string[i] = 0;
		}
		else
		{
			int32 totalDecimalDigits = Math::getDigitsCount(roundedDecimalValue);

			char* decimalString = Utilities::itoa((int32)(decimalValue * 10) / 10, 10, totalDecimalDigits);

			int32 j = 0;

			for(; j < totalDecimalDigits; j++)
			{
				string[i + j] = decimalString[j];
			}

			string[i + j] = 0;
		}
	}
	else
	{
		string[i] = 0;
	}

	Printer::text(string, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setFontPage(const char* font, uint16 page)
{
	FontData* fontData = Printer::getFontByName(font);

	if(NULL == fontData || isDeleted(fontData->charSet))
	{
		return;
	}

	CharSet::setFrame(fontData->charSet, page);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setOrientation(uint8 value)
{
	Printer printing = Printer::getInstance();

	printing->orientation = value;

	switch(printing->orientation)
	{
		case kPrintingOrientationHorizontal:
		case kPrintingOrientationVertical:
		{
			break;
		}

		default:
		{
			printing->orientation = kPrintingOrientationHorizontal;
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setTextDirection(uint8 value)
{
	Printer printing = Printer::getInstance();

	printing->direction = value;

	switch(printing->direction)
	{
		case kPrintingDirectionLTR:
		case kPrintingDirectionRTL:
		{
			break;
		}

		default:
		{
			printing->direction = kPrintingDirectionLTR;
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::addSprite()
{
	Printer printing = Printer::getInstance();
	printing->activePrintingSprite = 
		PrintingSprite::safeCast(Printer::addComponent(printing, (ComponentSpec*)&DefaultPrintingSpriteSpec));

	PixelVector position = 
	{
		0, 0, 0, 0
	};

	PrintingSprite::setPosition(printing->activePrintingSprite, &position);

	Printer::clearComponentLists(printing, kSpriteComponent);

	Printer::fireEvent(printing, kEventFontRewritten);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool Printer::setActiveSprite(uint16 printingSpriteIndex)
{
	Printer printing = Printer::getInstance();

	printing->activePrintingSprite = 
		PrintingSprite::safeCast(Printer::getComponentAtIndex(printing, kSpriteComponent, printingSpriteIndex));

	bool result = NULL != printing->activePrintingSprite;

	if(NULL == printing->activePrintingSprite)
	{
		printing->activePrintingSprite = PrintingSprite::safeCast(Printer::getComponentAtIndex(printing, kSpriteComponent, 0));
	}

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getPrintingBgmapSegment()
{
	Printer printing = Printer::getInstance();

	if(isDeleted(printing->activePrintingSprite))
	{
		return 0;
	}

	BgmapTexture texture = BgmapTexture::safeCast(PrintingSprite::getTexture(printing->activePrintingSprite));

	if(isDeleted(texture))
	{
		return 0;
	}

	return BgmapTexture::getSegment(texture);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getPrintingBgmapXOffset()
{
	Printer printing = Printer::getInstance();

	if(isDeleted(printing->activePrintingSprite))
	{
		return 0;
	}

	BgmapTexture texture = BgmapTexture::safeCast(PrintingSprite::getTexture(printing->activePrintingSprite));

	if(isDeleted(texture))
	{
		return 0;
	}

	return BgmapTexture::getXOffset(texture);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getPrintingBgmapYOffset()
{
	Printer printing = Printer::getInstance();

	if(isDeleted(printing->activePrintingSprite))
	{
		return 0;
	}

	BgmapTexture texture = BgmapTexture::safeCast(PrintingSprite::getTexture(printing->activePrintingSprite));

	if(isDeleted(texture))
	{
		return 0;
	}

	return BgmapTexture::getYOffset(texture);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16* Printer::getPrintingBgmapAddress()
{
	return 
		(uint16*)__BGMAP_SEGMENT(Printer::getPrintingBgmapSegment()) + 
		Printer::getPrintingBgmapXOffset() + 
		(Printer::getPrintingBgmapYOffset() << 6);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::printSprite(int16 x, int16 y)
{
	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::print(printing->activePrintingSprite, x, y);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer::setWorldCoordinates(x, y, z, parallax);
	Printer::setBgmapCoordinates(x, y, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setWorldCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PixelVector position = 
		{
			x <= __SCREEN_WIDTH ? x : 0, 
			y <= __SCREEN_HEIGHT ? y : 0, 
			z, 
			parallax
		};

		PrintingSprite::setPosition(printing->activePrintingSprite, &position);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setBgmapCoordinates(int16 mx, int16 my, int8 mp)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setMValues
		(
			printing->activePrintingSprite, 
			mx <= 64 * 8 ? mx : 0, 
			my <= 64 * 8 ? my : 0,
			mp
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setWorldSize(uint16 w, uint16 h)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setSize
		(
			printing->activePrintingSprite, w < __SCREEN_WIDTH ? w : __SCREEN_WIDTH, h < __SCREEN_HEIGHT ? h : __SCREEN_HEIGHT
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setTransparency(uint8 transparency)
{
	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		Sprite::setTransparency(printing->activePrintingSprite, transparency);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setPalette(uint8 palette)
{
	Printer printing = Printer::getInstance();

	if(4 > palette)
	{
		printing->palette = palette;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::resetCoordinates()
{
	Printer printing = Printer::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::reset(printing->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getWorldCoordinatesX()
{
	Printer printing = Printer::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveX(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getWorldCoordinatesY()
{
	Printer printing = Printer::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveY(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printer::getWorldCoordinatesP()
{
	Printer printing = Printer::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveP(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static PixelVector Printer::getActiveSpritePosition()
{
	Printer printing = Printer::getInstance();

	return 
		!isDeleted(printing->activePrintingSprite) ? 
			PrintingSprite::getDisplacedPosition(printing->activePrintingSprite) 
			: 
			(PixelVector){0, 0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontData* Printer::getFontByName(const char* font)
{
	Printer printing = Printer::getInstance();

	FontData* result = NULL;

	if(printing->mode == __PRINTING_MODE_DEBUG)
	{
		result = (FontData*)&VUENGINE_DEBUG_FONT_DATA;

		if(NULL == result->charSet)
		{
			result->charSet = CharSet::get(result->fontSpec->charSetSpec);
			CharSet::write(result->charSet);
		}
	}
	else
	{
		// Set first defined font as default
		result = &_fontData[0];

		if(NULL != font)
		{
			for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
			{
				if(0 == strcmp(_fontData[i].fontSpec->name, font))
				{
					result = &_fontData[i];
					break;
				}
			}
		}

		if(NULL == result->charSet && NULL != result->fontSpec->charSetSpec)
		{
			result->charSet = CharSet::get(result->fontSpec->charSetSpec);

			NM_ASSERT(!isDeleted(result->charSet), "Printer::getFontByName: cannot load charset");

			if(NULL != result->charSet)
			{
				CharSet::addEventListener(result->charSet, ListenerObject::safeCast(printing), kEventCharSetChangedOffset);
			}
		}
	}
	
	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontSize Printer::getTextSize(const char* string, const char* font)
{
	FontSize fontSize = {0, 0};
	uint16 i = 0, currentLineLength = 0;

	FontData* fontData = Printer::getFontByName(font);

	if(NULL == fontData)
	{
		// Just to make sure that no client code does a 0 division with these results
		fontSize = (FontSize){8, 8};
		return fontSize;
	}

	fontSize.y = fontData->fontSpec->fontSize.y;

	while('\0' != string[i])
	{
		switch(string[i])
		{
			// Line feed
			case 13:
			{
				break;
			}

			// Tab
			case 9:
			{
				currentLineLength += (currentLineLength / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				break;
			}

			// Carriage return
			case 10:
			{
				fontSize.y += fontData->fontSpec->fontSize.y;
				currentLineLength = 0;
				break;
			}

			default:
			{
				currentLineLength += fontData->fontSpec->fontSize.x;
				if(currentLineLength >= 64)
				{
					fontSize.y += fontData->fontSpec->fontSize.y;
					currentLineLength = 0;
				}

				break;
			}
		}

		if(currentLineLength > fontSize.x)
		{
			fontSize.x = currentLineLength;
		}

		i++;
	}

	return fontSize;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::loadDebugFont()
{
	Mem::copyWORD
	(
		(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)VUENGINE_DEBUG_FONT_CHARSET_OFFSET) << 4)),
		VUENGINE_DEBUG_FONT_DATA.fontSpec->charSetSpec->tiles + 1,
		__UINT32S_PER_CHARS(VUENGINE_DEBUG_FONT_SIZE)
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::number(int32 value, uint8 x, uint8 y, const char* font)
{
	if(value < 0)
	{
		value = -value;
		Printer::out(x++, y, "-", font);
	}

	Printer::out(x, y, Utilities::itoa((int32)(value), 10, 0), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::out(uint8 x, uint8 y, const char* string, const char* font)
{
	Printer printing = Printer::getInstance();

#ifdef __DEFAULT_FONT
	if(NULL == font)
	{
		font = __DEFAULT_FONT;
	}
#endif

#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	uint32 i = 0, position = 0, startColumn = x, temp = 0;
	uint32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
	FontData* fontData = printing->lastUsedFontData;

	if(NULL == fontData)
	{
		fontData = Printer::getFontByName(font);
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}
	else if(printing->lastUsedFont != font && strcmp(fontData->fontSpec->name, font))
	{
		fontData = Printer::getFontByName(font);
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}
	else
	{
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}

	if(NULL == fontData || (__PRINTING_MODE_DEBUG != printing->mode && isDeleted(fontData->charSet)))
	{
		return;
	}

	uint32 offset = __PRINTING_MODE_DEBUG == printing->mode ? VUENGINE_DEBUG_FONT_CHARSET_OFFSET : CharSet::getOffset(fontData->charSet);

	if(isDeleted(printing->activePrintingSprite))
	{
		return;
	}

	BgmapTexture texture = BgmapTexture::safeCast(PrintingSprite::getTexture(printing->activePrintingSprite));

	if(isDeleted(texture))
	{
		return;
	}

	int16 printingBgmapSegment = BgmapTexture::getSegment(texture);
	int16 xOffset = BgmapTexture::getXOffset(texture);
	int16 yOffset = BgmapTexture::getYOffset(texture);

	// Print text
	while('\0' != string[i] && x < (__SCREEN_WIDTH_IN_CHARS))
	{
		// Do not allow printing outside of the visible area, since that would corrupt the param table
		if(y > 27/* || y < 0*/)
		{
			break;
		}

		position = (y << 6) + x;

		switch(string[i])
		{
			// Line feed
			case 13:
			{
				break;
			}

			// Tab
			case 9:
			{
				if(kPrintingOrientationHorizontal == printing->orientation)
				{
					x = (x / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				}
				else
				{
					y = (y / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.y;
				}
				break;
			}

			// Carriage return
			case 10:
			{
				temp = fontData->fontSpec->fontSize.y;
				y = (printing->direction == kPrintingDirectionLTR)
					? y + temp
					: y - temp;
				x = startColumn;
				break;
			}

			default:
			{
				for(charOffsetX = 0; charOffsetX < fontData->fontSpec->fontSize.x; charOffsetX++)
				{
					for(charOffsetY = 0; charOffsetY < fontData->fontSpec->fontSize.y; charOffsetY++)
					{
						charOffset = 
							charOffsetX + 
							(charOffsetY * fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x);

						uint16* offsetDisplacement = (HWORD*)__BGMAP_SEGMENT(printingBgmapSegment) + xOffset + (yOffset << 6) +
							+ position + charOffsetX + (charOffsetY << 6);


						*offsetDisplacement =
							(
								// Offset of charset in char memory
								offset +
								((uint8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

								(((uint8)(string[i] - fontData->fontSpec->offset)
									/ fontData->fontSpec->charactersPerLineInCharset
									* fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x)
										* (fontData->fontSpec->fontSize.y - 1)) +

								// Respective char of character
								charOffset
							)
							| (printing->palette << 14);
					}
				}

				if(kPrintingOrientationHorizontal == printing->orientation)
				{
					temp = fontData->fontSpec->fontSize.x;
					x = (printing->direction == kPrintingDirectionLTR)
						? x + temp
						: x - temp;
				}
				else
				{
					temp = fontData->fontSpec->fontSize.y;
					y = (printing->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
				}

				if(x >= 48/* || x < 0*/)
				{
					// Wrap around when outside of the visible area
					temp = fontData->fontSpec->fontSize.y;
					y = (printing->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
					x = startColumn;
				}

				break;
			}
		}

		i++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Printer::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventCharSetChangedOffset:
		{
			CharSet charSet = CharSet::safeCast(eventFirer);

			if(!isDeleted(charSet))
			{
				CharSet::write(charSet);
				Printer::fireEvent(this, kEventFontRewritten);
				NM_ASSERT(!isDeleted(this), "Printer::onEvent: deleted printing during kEventFontRewritten");
			}

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::removedComponent(Component component __attribute__((unused)))
{
	this->activePrintingSprite = NULL;

	Printer::setActiveSprite(0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void Printer::reset()
{
	this->activePrintingSprite = NULL;

	Printer::releaseFonts(this);
	Printer::resetCoordinates();
	Printer::setOrientation(kPrintingOrientationHorizontal);
	Printer::setTextDirection(kPrintingDirectionLTR);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Initialize members
	this->mode = __PRINTING_MODE_DEFAULT;
	this->palette = __PRINTING_PALETTE;
	this->orientation = kPrintingOrientationHorizontal;
	this->direction = kPrintingDirectionLTR;
	this->lastUsedFont = NULL;
	this->lastUsedFontData = NULL;
	this->activePrintingSprite = NULL;

	for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
	{
		_fontData[i].charSet = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
