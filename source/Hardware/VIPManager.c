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

#include <CharSet.h>
#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Mem.h>
#include <Printer.h>
#include <Profiler.h>
#include <Singleton.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "VIPManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

extern ColumnTableROMSpec DefaultColumnTableSpec;
extern BrightnessRepeatROMSpec DefaultBrightnessRepeatSpec;
extern uint32 _dramDirtyStart;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TIMEERR					0x8000
#define __XPEND						0x4000
#define __SBHIT						0x2000
#define __FRAMESTART				0x0010
#define __GAMESTART					0x0008
#define __RFBEND					0x0004
#define __LFBEND					0x0002
#define __SCANERR					0x0001

#define __LOCK						0x0400	// VPU SELECT __CTA
#define __SYNCE						0x0200	// L,R_SYNC TO VPU
#define __RE						0x0100	// MEMORY REFLASH CYCLE ON
#define __FCLK						0x0080
#define __SCANRDY					0x0040
#define __DISP						0x0002	// DISPLAY ON
#define __DPRST						0x0001	// RESET VPU COUNTER AND WAIT __FCLK
#define __DPBSY						0x003C	// In the midst of displaying

#define __SBOUT						0x8000					// In FrameBuffer drawing included
#define __SBCOUNT					0x1F00					// Current bloc being drawn
#define __OVERTIME					0x0010					// Processing
#define __XPBSY1					0x0008					// In the midst of FrameBuffer 1 picture editing
#define __XPBSY0					0x0004					// In the midst of FrameBuffer 0 picture editing
#define __XPBSY						(__XPBSY0 | __XPBSY1)  // In the midst of drawing
#define __XPEN						0x0002					// Start of drawing
#define __XPRST						0x0001					// Forcing idling

// VIP Register Mnemonics
#define __INTPND					0x00  // Interrupt Pending
#define __INTENB					0x01  // Interrupt Enable
#define __INTCLR					0x02  // Interrupt Clear

#define __DPSTTS					0x10  // Display Status
#define __DPCTRL					0x11  // Display Control
#define __BRTA						0x12  // Brightness A
#define __BRTB						0x13  // Brightness B
#define __BRTC						0x14  // Brightness C
#define __REST						0x15  // Brightness Idle

#define __FRMCYC					0x17  // Frame Repeat
#define __CTA						0x18  // Column Table Pointer

#define __XPSTTS					0x20  // Drawing Status
#define __XPCTRL					0x21  // Drawing Control
#define __VER						0x22  // VIP Version

#define __GPLT0						0x30  // BGMap Palette 0
#define __GPLT1						0x31  // BGMap Palette 1
#define __GPLT2						0x32  // BGMap Palette 2
#define __GPLT3						0x33  // BGMap Palette 3

#define __JPLT0						0x34  // OBJ Palette 0
#define __JPLT1						0x35  // OBJ Palette 1
#define __JPLT2						0x36  // OBJ Palette 2
#define __JPLT3						0x37  // OBJ Palette 3

#define __BACKGROUND_COLOR			0x38  // Background Color

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Processing Effect Registry
/// @memberof VIPManager
typedef struct PostProcessingEffectRegistry
{
	PostProcessingEffect postProcessingEffect;
	Entity entity;
	bool remove;

} PostProcessingEffectRegistry;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

volatile uint16* _vipRegisters __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = (uint16*)0x0005F800;

static uint16* const _columnTableBaseAddressLeft =	(uint16*)0x0003DC00; // base address of Column Table (Left Eye)
static uint16* const _columnTableBaseAddressRight =	(uint16*)0x0003DE00; // base address of Column Table (Right Eye)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::interruptHandler()
{
	VIPManager vipManager = VIPManager::getInstance();

	vipManager->currrentInterrupt = _vipRegisters[__INTPND];

	// Clear interrupts
	VIPManager::clearInterrupts(vipManager);

	if(kVIPNoMultiplexedInterrupts != vipManager->enabledMultiplexedInterrupts)
	{
		if(kVIPOnlyVIPMultiplexedInterrupts == vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(vipManager->enabledMultiplexedInterrupts);
		}

		HardwareManager::enableMultiplexedInterrupts();
	}

	// Handle the interrupt
	VIPManager::processInterrupt(vipManager, vipManager->currrentInterrupt);
	
	if(kVIPNoMultiplexedInterrupts != vipManager->enabledMultiplexedInterrupts)
	{
		HardwareManager::disableMultiplexedInterrupts();

		if(kVIPOnlyVIPMultiplexedInterrupts == vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(0);
		}
	}

	vipManager->currrentInterrupt = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager vipManager = VIPManager::getInstance();

	PostProcessingEffectRegistry* postProcessingEffectRegistry = 
		VIPManager::isPostProcessingEffectRegistered(vipManager, postProcessingEffect, entity);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->entity = entity;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushFront(vipManager->postProcessingEffects, postProcessingEffectRegistry);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager vipManager = VIPManager::getInstance();

	PostProcessingEffectRegistry* postProcessingEffectRegistry = 
		VIPManager::isPostProcessingEffectRegistered(vipManager, postProcessingEffect, entity);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->entity = entity;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushBack(vipManager->postProcessingEffects, postProcessingEffectRegistry);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager vipManager = VIPManager::getInstance();

	for(VirtualNode node = vipManager->postProcessingEffects->head; NULL != node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if
		(
			postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect 
			&& 
			postProcessingEffectRegistry->entity == entity
		)
		{
			postProcessingEffectRegistry->remove = true;
			return;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::removePostProcessingEffects()
{
	VIPManager vipManager = VIPManager::getInstance();

	VirtualList::deleteData(vipManager->postProcessingEffects);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::enableCustomInterrupts(uint16 customInterrupts)
{
	VIPManager vipManager = VIPManager::getInstance();

	vipManager->customInterrupts = customInterrupts;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::enableMultiplexedInterrupts(uint32 enabledMultiplexedInterrupts __attribute__((unused)))
{
	VIPManager vipManager = VIPManager::getInstance();

#ifndef __ENABLE_PROFILER
	vipManager->enabledMultiplexedInterrupts = enabledMultiplexedInterrupts;
#else
	vipManager->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::setDrawingStrategy(uint32 drawingStrategy)
{
	VIPManager vipManager = VIPManager::getInstance();

	if(kVIPManagerStrategyLimiter <= drawingStrategy)
	{
		drawingStrategy = 0;
	}

	vipManager->drawingStrategy = drawingStrategy;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 VIPManager::getDrawingStrategy()
{
	VIPManager vipManager = VIPManager::getInstance();

	return vipManager->drawingStrategy;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::setFrameCycle(uint8 frameCycle __attribute__((unused)))
{
	VIPManager vipManager = VIPManager::getInstance();

#ifdef __DEBUG
	frameCycle++;
#endif

	if(3 < frameCycle)
	{
		frameCycle = 3;
	}

	vipManager->gameFrameDuration = (__MILLISECONDS_PER_SECOND / __MAXIMUM_FPS) << frameCycle;

	_vipRegisters[__FRMCYC] = frameCycle;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configurePalettes(PaletteConfig paletteConfig)
{
	_vipRegisters[__GPLT0] = paletteConfig.bgmap.gplt0;
	_vipRegisters[__GPLT1] = paletteConfig.bgmap.gplt1;
	_vipRegisters[__GPLT2] = paletteConfig.bgmap.gplt2;
	_vipRegisters[__GPLT3] = paletteConfig.bgmap.gplt3;

	_vipRegisters[__JPLT0] = paletteConfig.object.jplt0;
	_vipRegisters[__JPLT1] = paletteConfig.object.jplt1;
	_vipRegisters[__JPLT2] = paletteConfig.object.jplt2;
	_vipRegisters[__JPLT3] = paletteConfig.object.jplt3;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configureColumnTable(const ColumnTableSpec* columnTableSpec)
{
	int32 i, value;

	// Use the default column table as fallback
	if(columnTableSpec == NULL)
	{
		columnTableSpec = (ColumnTableSpec*)&DefaultColumnTableSpec;
	}

	// Write column table
	for(i = 0; i < 256; i++)
	{
		value = (columnTableSpec->mirror && (i > (__COLUMN_TABLE_ENTRIES / 2 - 1)))
			? columnTableSpec->columnTable[(__COLUMN_TABLE_ENTRIES - 1) - i]
			: columnTableSpec->columnTable[i];

		_columnTableBaseAddressLeft[i] = value;
		_columnTableBaseAddressRight[i] = value;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configureBrightness(Brightness brightness)
{
	VIPManager vipManager = VIPManager::getInstance();

	while(vipManager->isDrawingAllowed && 0 != (_vipRegisters[__XPSTTS] & __XPBSY));
	
	_vipRegisters[__BRTA] = brightness.darkRed;
	_vipRegisters[__BRTB] = brightness.mediumRed;
	_vipRegisters[__BRTC] = brightness.brightRed - (brightness.mediumRed + brightness.darkRed);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Brightness VIPManager::getBrightness()
{
	return 	
		(Brightness)
		{
			_vipRegisters[__BRTA],
			_vipRegisters[__BRTB],
			_vipRegisters[__BRTC] + (_vipRegisters[__BRTB] + _vipRegisters[__BRTA])
		};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configureBrightnessRepeat(const BrightnessRepeatSpec* brightnessRepeatSpec)
{
	// Use the default repeat values as fallback
	if(brightnessRepeatSpec == NULL)
	{
		brightnessRepeatSpec = (BrightnessRepeatSpec*)&DefaultBrightnessRepeatSpec;
	}
	// Column table offsets
	int16 leftCta = _vipRegisters[__CTA] & 0xFF;
	int16 rightCta = _vipRegisters[__CTA] >> 8;

	CACHE_RESET;

	// Write repeat values to column table
	for(int16 i = 0; i < 96; i++)
	{
		int16 value = (brightnessRepeatSpec->mirror && (i > (__BRIGHTNESS_REPEAT_ENTRIES / 2 - 1)))
			? brightnessRepeatSpec->brightnessRepeat[__BRIGHTNESS_REPEAT_ENTRIES - 1 - i] << 8
			: brightnessRepeatSpec->brightnessRepeat[i] << 8;

		_columnTableBaseAddressLeft[leftCta - i] = (_columnTableBaseAddressLeft[leftCta - i] & 0xff) | value;
		_columnTableBaseAddressRight[rightCta - i] = (_columnTableBaseAddressRight[rightCta - i] & 0xff) | value;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configureBackgroundColor(uint8 color)
{
	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::configurePostProcessingEffects(PostProcessingEffect* postProcessingEffects)
{
	if(NULL == postProcessingEffects)
	{
		return;
	}

	for(int32 i = 0; NULL != postProcessingEffects[i]; i++)
	{
		VIPManager::pushFrontPostProcessingEffect(postProcessingEffects[i], NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::upBrightness()
{
	VIPManager vipManager = VIPManager::getInstance();

	while(vipManager->isDrawingAllowed && 0 != (_vipRegisters[__XPSTTS] & __XPBSY));

	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::lowerBrightness()
{
	VIPManager vipManager = VIPManager::getInstance();

	while(vipManager->isDrawingAllowed && 0 != (_vipRegisters[__XPSTTS] & __XPBSY));

	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 VIPManager::getCurrentInterrupt()
{
	VIPManager vipManager = VIPManager::getInstance();

	return vipManager->currrentInterrupt;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 VIPManager::getGameFrameDuration()
{
	VIPManager vipManager = VIPManager::getInstance();

	return vipManager->gameFrameDuration;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::reset()
{
	this->customInterrupts = 0;
	this->currrentInterrupt = 0;
	this->processingGAMESTART = false;
	this->processingXPEND = false;
	this->isDrawingAllowed = false;
	this->drawingStrategy = kVIPManagerFavorStability;
	this->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
	this->allowInterrupts = true;

	VIPManager::lowerBrightness();
	VIPManager::removePostProcessingEffects();

	VIPManager::setFrameCycle(__FRAME_CYCLE);
	VIPManager::configureColumnTable(NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::configure
(
	uint8 backgroundColor, Brightness brightness, const BrightnessRepeatSpec* brightnessRepeat, 
	PaletteConfig paletteConfig, PostProcessingEffect* postProcessingEffects
)
{
	VIPManager::configureBackgroundColor(backgroundColor);
	VIPManager::configureBrightness(brightness);
	VIPManager::configureBrightnessRepeat(brightnessRepeat);
	VIPManager::configurePalettes(paletteConfig);
	VIPManager::configurePostProcessingEffects(postProcessingEffects);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::startMemoryRefresh()
{
	_vipRegisters[__FRMCYC] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::waitForFRAMESTART()
{
	while(!(_vipRegisters[__DPSTTS] & __FCLK));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::startDrawing()
{
	this->isDrawingAllowed = true;

	VIPManager::enableInterrupts(this, __GAMESTART | __XPEND);

	while(0 != (_vipRegisters[__XPSTTS] & __XPBSY));
	
	_vipRegisters[__XPCTRL] |= __XPEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::resumeDrawing()
{
	if(this->isDrawingAllowed)
	{
		if(kVIPManagerFavorStability == this->drawingStrategy)
		{
			while(0 != (_vipRegisters[__XPSTTS] & __XPBSY));
		}

		_vipRegisters[__XPCTRL] |= __XPEN;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::suspendDrawing()
{
	this->isDrawingAllowed = VIPManager::isDrawingAllowed(this);
	
	if(kVIPManagerFavorStability == this->drawingStrategy)
	{
		while(this->isDrawingAllowed && 0 != (_vipRegisters[__XPSTTS] & __XPBSY));
	}

	_vipRegisters[__XPCTRL] &= ~__XPEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::stopDrawing()
{
	VIPManager::disableInterrupts(this);

	while(this->isDrawingAllowed && 0 != (_vipRegisters[__XPSTTS] & __XPBSY));

	_vipRegisters[__XPCTRL] &= ~__XPEN;

	this->isDrawingAllowed = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::startDisplaying()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = (__SYNCE | __RE | __DISP) & ~__LOCK;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::stopDisplaying()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::allowInterrupts(bool allowInterrupts)
{
	this->allowInterrupts = allowInterrupts;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 VIPManager::getCurrentDrawingFrameBufferSet()
{
	return this->currentDrawingFrameBufferSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::print(int16 x, int16 y)
{
	int16 xDisplacement = 8;

	Printer::text("VIP\nRegisters", x, ++y, NULL);
	y += 2;
	Printer::text("INTPND:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__INTPND], x + xDisplacement, y, 4, NULL);
	Printer::text("INTENB:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__INTENB], x + xDisplacement, y, 4, NULL);
	Printer::text("INTCLR:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__INTCLR], x + xDisplacement, y, 4, NULL);
	Printer::text("DPSTTS:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__DPSTTS], x + xDisplacement, y, 4, NULL);
	Printer::text("DPCTRL:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__DPCTRL], x + xDisplacement, y, 4, NULL);
	Printer::text("BRTA:", x, ++y, NULL);
	Printer::hex((uint8)_vipRegisters[__BRTA], x + xDisplacement, y, 4, NULL);
	Printer::text("BRTB:", x, ++y, NULL);
	Printer::hex((uint8)_vipRegisters[__BRTB], x + xDisplacement, y, 4, NULL);
	Printer::text("BRTC:", x, ++y, NULL);
	Printer::hex((uint8)_vipRegisters[__BRTC], x + xDisplacement, y, 4, NULL);
	Printer::text("REST:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__REST], x + xDisplacement, y, 4, NULL);
	Printer::text("FRMCYC:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__FRMCYC], x + xDisplacement, y, 4, NULL);
	Printer::text("CTA:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__CTA], x + xDisplacement, y, 4, NULL);
	Printer::text("XPSTTS:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__XPSTTS], x + xDisplacement, y, 4, NULL);
	Printer::text("XPCTRL:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__XPCTRL], x + xDisplacement, y, 4, NULL);
	Printer::text("VER:", x, ++y, NULL);
	Printer::hex(_vipRegisters[__VER], x + xDisplacement, y, 4, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->postProcessingEffects = new VirtualList();
	this->currentDrawingFrameBufferSet = 0;
	this->FRAMESTARTDuringXPEND = false;
	this->processingXPEND = false;
	this->processingGAMESTART = false;
	this->customInterrupts = 0;
	this->currrentInterrupt = 0;
	this->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
	this->isDrawingAllowed = false;
	this->drawingStrategy = kVIPManagerFavorStability;
	this->allowInterrupts = true;

	VIPManager::setFrameCycle(__FRAME_CYCLE);
	VIPManager::lowerBrightness();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::destructor()
{
	VIPManager::removePostProcessingEffects(this);

	delete this->postProcessingEffects;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::processInterrupt(uint16 interrupt)
{
	static uint16 interruptTable[] =
	{
		__FRAMESTART,
		__GAMESTART,
		__XPEND,
#ifndef __SHIPPING
		__TIMEERR,
		__SCANERR
#endif
	};

	for(uint32 i = 0; i < sizeof(interruptTable) / sizeof(uint16); i++)
	{
		switch(interrupt & interruptTable[i])
		{
			case __FRAMESTART:
			{
				this->FRAMESTARTDuringXPEND = this->processingXPEND;

				VIPManager::fireEvent(this, kEventVIPManagerFRAMESTART);
				break;
			}

			case __GAMESTART:
			{
				this->processingGAMESTART = true;

				VIPManager::registerCurrentDrawingFrameBufferSet(this);

				VIPManager::fireEvent
				(
					this, 
					this->processingXPEND ? 
						kEventVIPManagerGAMESTARTDuringXPEND 
						:
						kEventVIPManagerGAMESTART
				);

				this->processingGAMESTART = false;

				break;
			}

			case __XPEND:
			{
#ifdef __ENABLE_PROFILER
				Profiler::lap(kProfilerLapTypeStartInterrupt, NULL);
#endif
				VIPManager::suspendDrawing(this);

				this->processingXPEND = true;				

				VIPManager::fireEvent
				(
					this, 
					this->processingGAMESTART ? 
						kEventVIPManagerXPENDDuringGAMESTART 
						:
						kEventVIPManagerXPEND
				);

				VIPManager::applyPostProcessingEffects(this);

				this->processingXPEND = false;

				VIPManager::resumeDrawing(this);

#ifdef __ENABLE_PROFILER
				Profiler::lap(kProfilerLapTypeVIPInterruptXPENDProcess, PROCESS_NAME_VRAM_WRITE);
#endif
				break;
			}
#ifndef __SHIPPING
			case __TIMEERR:
			{
				VIPManager::fireEvent(this, kEventVIPManagerTimeError);
				break;
			}

			case __SCANERR:
			{
				VIPManager::fireEvent(this, kEventVIPManagerScanError);

				Error::triggerException("VIPManager::servo error", NULL);
				break;
			}
#endif
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::enableInterrupts(uint16 interruptCode)
{
	if(!this->allowInterrupts)
	{
		return;
	}

	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];

	interruptCode |= this->customInterrupts;

#ifndef __SHIPPING
	_vipRegisters[__INTENB] = interruptCode | __FRAMESTART | __TIMEERR | __SCANERR;
#else
	_vipRegisters[__INTENB] = interruptCode | __FRAMESTART;
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::disableInterrupts()
{
	_vipRegisters[__INTENB] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::clearInterrupts()
{
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::applyPostProcessingEffects()
{
	for
	(
		VirtualNode node = this->postProcessingEffects->tail, previousNode = NULL; 
		!this->FRAMESTARTDuringXPEND && NULL != node; 
		node = previousNode
	)
	{
		previousNode = node->previous;

		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(isDeleted(postProcessingEffectRegistry) || postProcessingEffectRegistry->remove)
		{
			VirtualList::removeNode(this->postProcessingEffects, node);

			if(!isDeleted(postProcessingEffectRegistry))
			{
				delete postProcessingEffectRegistry;
			}
		}
		else
		{
			postProcessingEffectRegistry->postProcessingEffect
			(
				this->currentDrawingFrameBufferSet, postProcessingEffectRegistry->entity
			);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::useInternalColumnTable(bool useInternal)
{
	if(useInternal)
	{
		// Set lock bit
		_vipRegisters[__DPCTRL] |= __LOCK;
	}
	else
	{
		// Unset lock bit
		_vipRegisters[__DPCTRL] &= ~__LOCK;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::registerCurrentDrawingFrameBufferSet()
{
	uint16 currentDrawingFrameBufferSet = _vipRegisters[__XPSTTS] & __XPBSY;

	if(0x0004 == currentDrawingFrameBufferSet)
	{
		this->currentDrawingFrameBufferSet = 0;
	}
	else if(0x0008 == currentDrawingFrameBufferSet)
	{
		this->currentDrawingFrameBufferSet = 0x8000;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VIPManager::isDrawingAllowed()
{
	return 0 != (_vipRegisters[__XPSTTS] & __XPEN);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

PostProcessingEffectRegistry* VIPManager::isPostProcessingEffectRegistered
(
	PostProcessingEffect postProcessingEffect, Entity entity
)
{
	VirtualNode node = this->postProcessingEffects->head;

	for(; NULL != node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if
		(
			postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect 
			&& 
			postProcessingEffectRegistry->entity == entity
		)
		{
			return postProcessingEffectRegistry;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 VIPManager::getCurrentBlockBeingDrawn()
{
	if(_vipRegisters[__XPSTTS] & __SBOUT)
	{
		return (_vipRegisters[__XPSTTS] & __SBCOUNT) >> 8;
	}

	if(0 == (_vipRegisters[__XPSTTS] & __XPBSY))
	{
		return -1;
	}

	while(!(_vipRegisters[__XPSTTS] & __SBOUT));

	return (_vipRegisters[__XPSTTS] & __SBCOUNT) >> 8;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
