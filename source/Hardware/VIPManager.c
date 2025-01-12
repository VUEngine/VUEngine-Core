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

#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Mem.h>
#include <Printing.h>
#include <Profiler.h>
#include <VirtualList.h>
#include <VirtualNode.h>

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

WorldAttributes _worldAttributesCache[__TOTAL_LAYERS] __attribute__((section(".dram_bss")));
ObjectAttributes _objectAttributesCache[__TOTAL_OBJECTS] __attribute__((section(".dram_bss")));

volatile uint16* _vipRegisters __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = (uint16*)0x0005F800;
uint32* _currentDrawingFrameBufferSet __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

static uint16* const _columnTableBaseAddressLeft =	(uint16*)0x0003DC00; // base address of Column Table (Left Eye)
static uint16* const _columnTableBaseAddressRight =	(uint16*)0x0003DE00; // base address of Column Table (Right Eye)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VIPManager::interruptHandler()
{
	VIPManager vipManager = VIPManager::getInstance();

	vipManager->currrentInterrupt = _vipRegisters[__INTPND];

	// Clear interrupts
	VIPManager::clearInterrupts(vipManager);

#ifndef __DEBUG
	if(kVIPNoMultiplexedInterrupts != vipManager->enabledMultiplexedInterrupts)
	{
		if(kVIPOnlyVIPMultiplexedInterrupts == vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(vipManager->enabledMultiplexedInterrupts);
		}

		HardwareManager::enableMultiplexedInterrupts();
	}
#endif

	// Handle the interrupt
	VIPManager::processInterrupt(vipManager, vipManager->currrentInterrupt);

#ifndef __DEBUG
	if(kVIPNoMultiplexedInterrupts != vipManager->enabledMultiplexedInterrupts)
	{
		HardwareManager::disableMultiplexedInterrupts();

		if(kVIPOnlyVIPMultiplexedInterrupts == vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(0);
		}
	}
#endif

	vipManager->currrentInterrupt = 0;
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
	
#ifndef __ENABLE_PROFILER
	this->enabledMultiplexedInterrupts = kVIPAllMultiplexedInterrupts;
#else
	this->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
#endif

	VIPManager::lowerBrightness(this);
	VIPManager::removePostProcessingEffects(this);

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);
	VIPManager::setupColumnTable(this, NULL);

	VIPManager::clearDRAM(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::configure
(
	uint8 backgroundColor, Brightness* brightness, BrightnessRepeatSpec* brightnessRepeat, 
	PaletteConfig* paletteConfig, PostProcessingEffect* postProcessingEffects
)
{
	VIPManager::configureBackgroundColor(this, backgroundColor);
	VIPManager::configureBrightness(this, brightness);
	VIPManager::configureBrightnessRepeat(this, brightnessRepeat);
	VIPManager::configurePalettes(this, paletteConfig);
	VIPManager::configurePostProcessingEffects(this, postProcessingEffects);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::enableCustomInterrupts(uint16 customInterrupts)
{
	this->customInterrupts = customInterrupts;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::enableMultiplexedInterrupts(uint32 enabledMultiplexedInterrupts __attribute__((unused)))
{
#ifndef __ENABLE_PROFILER
	this->enabledMultiplexedInterrupts = enabledMultiplexedInterrupts;
#else
	this->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::setDrawingStrategy(uint32 drawingStrategy)
{
	if(kVIPManagerStrategyLimiter <= drawingStrategy)
	{
		drawingStrategy = 0;
	}

	this->drawingStrategy = drawingStrategy;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 VIPManager::getDrawingStrategy()
{
	return this->drawingStrategy;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::startDrawing()
{
	this->isDrawingAllowed = true;

	VIPManager::enableInterrupts(this, __FRAMESTART | __XPEND);

	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__XPCTRL] |= __XPEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::resumeDrawing()
{
	if(this->isDrawingAllowed)
	{
		if(kVIPManagerFavorStability == this->drawingStrategy)
		{
			while(_vipRegisters[__XPSTTS] & __XPBSY);
		}

		_vipRegisters[__XPCTRL] |= __XPEN;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::suspendDrawing()
{
	this->isDrawingAllowed = VIPManager::isDrawingAllowed(this);
	
	if(kVIPManagerFavorStability == this->drawingStrategy)
	{
		while(_vipRegisters[__XPSTTS] & __XPBSY);
	}

	_vipRegisters[__XPCTRL] &= ~__XPEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::stopDrawing()
{
	this->isDrawingAllowed = false;

	VIPManager::disableInterrupts(this);

	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__XPCTRL] &= ~__XPEN;
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

secure void VIPManager::setFrameCycle(uint8 frameCycle __attribute__((unused)))
{
#ifdef __DEBUG
	frameCycle = 2;
#else
	if(3 < frameCycle)
	{
		frameCycle = 3;
	}
#endif

	this->gameFrameDuration = (__MILLISECONDS_PER_SECOND / __MAXIMUM_FPS) << frameCycle;

	_vipRegisters[__FRMCYC] = frameCycle;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::configurePalettes(PaletteConfig* paletteConfig)
{
	_vipRegisters[__GPLT0] = paletteConfig->bgmap.gplt0;
	_vipRegisters[__GPLT1] = paletteConfig->bgmap.gplt1;
	_vipRegisters[__GPLT2] = paletteConfig->bgmap.gplt2;
	_vipRegisters[__GPLT3] = paletteConfig->bgmap.gplt3;

	_vipRegisters[__JPLT0] = paletteConfig->object.jplt0;
	_vipRegisters[__JPLT1] = paletteConfig->object.jplt1;
	_vipRegisters[__JPLT2] = paletteConfig->object.jplt2;
	_vipRegisters[__JPLT3] = paletteConfig->object.jplt3;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::setupColumnTable(ColumnTableSpec* columnTableSpec)
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

void VIPManager::configureBrightness(Brightness* brightness)
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = brightness->darkRed;
	_vipRegisters[__BRTB] = brightness->mediumRed;
	_vipRegisters[__BRTC] = brightness->brightRed - (brightness->mediumRed + brightness->darkRed);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::configureBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeatSpec)
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

void VIPManager::configureBackgroundColor(uint8 color)
{
	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::configurePostProcessingEffects(PostProcessingEffect* postProcessingEffects)
{
	if(NULL == postProcessingEffects)
	{
		return;
	}

	for(int32 i = 0; NULL != postProcessingEffects[i]; i++)
	{
		VIPManager::pushFrontPostProcessingEffect(this, postProcessingEffects[i], NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::upBrightness()
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::lowerBrightness()
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	PostProcessingEffectRegistry* postProcessingEffectRegistry = 
		VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, entity);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->entity = entity;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushFront(this->postProcessingEffects, postProcessingEffectRegistry);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	PostProcessingEffectRegistry* postProcessingEffectRegistry = 
		VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, entity);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->entity = entity;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushBack(this->postProcessingEffects, postProcessingEffectRegistry);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	for(VirtualNode node = this->postProcessingEffects->head; NULL != node; node = node->next)
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

secure void VIPManager::removePostProcessingEffects()
{
	VirtualList::deleteData(this->postProcessingEffects);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 VIPManager::getCurrentInterrupt()
{
	return this->currrentInterrupt;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 VIPManager::getGameFrameDuration()
{
	return this->gameFrameDuration;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
	this->enabledMultiplexedInterrupts = kVIPAllMultiplexedInterrupts;
	this->isDrawingAllowed = false;
	this->drawingStrategy = kVIPManagerFavorStability;

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);

	_currentDrawingFrameBufferSet = &this->currentDrawingFrameBufferSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VIPManager::destructor()
{
	VIPManager::removePostProcessingEffects(this);

	delete this->postProcessingEffects;

	// Allow a new construct
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

				this->FRAMESTARTDuringXPEND = this->processingXPEND;

				VIPManager::fireEvent(this, kEventVIPManagerFRAMESTART);
				break;

			case __GAMESTART:

#ifdef __ENABLE_PROFILER
				Profiler::lap(kProfilerLapTypeStartInterrupt, NULL);
#endif

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

#ifdef __ENABLE_PROFILER
				Profiler::lap(kProfilerLapTypeVIPInterruptGAMESTARTProcess, PROCESS_NAME_RENDER);
#endif
				break;

			case __XPEND:

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

#ifndef __SHIPPING
			case __TIMEERR:

				VIPManager::fireEvent(this, kEventVIPManagerTimeError);
				break;

			case __SCANERR:

				VIPManager::fireEvent(this, kEventVIPManagerScanError);

				Error::triggerException("VIPManager::servo error", NULL);		
				break;
#endif
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VIPManager::enableInterrupts(uint16 interruptCode)
{
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

void VIPManager::applyPostProcessingEffects()
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

secure void VIPManager::clearDRAM()
{
	uint8* bgmapStartAddress = (uint8*)__BGMAP_SPACE_BASE_ADDRESS;

	// Clear every bgmap segment
	for(bgmapStartAddress = 0; bgmapStartAddress < (uint8*)__PARAM_TABLE_END; bgmapStartAddress++)
	{
		*bgmapStartAddress = 0;
	}

	Mem::clear((BYTE*) __CHAR_SPACE_BASE_ADDRESS, 8192 * 4);

	for(int32 i = 0; i < __TOTAL_LAYERS; i++)
	{
		_worldAttributesCache[i].head = 0;
		_worldAttributesCache[i].gx = 0;
		_worldAttributesCache[i].gp = 0;
		_worldAttributesCache[i].gy = 0;
		_worldAttributesCache[i].mx = 0;
		_worldAttributesCache[i].mp = 0;
		_worldAttributesCache[i].my = 0;
		_worldAttributesCache[i].w = 0;
		_worldAttributesCache[i].h = 0;
		_worldAttributesCache[i].param = 0;
		_worldAttributesCache[i].ovr = 0;

		_worldAttributesBaseAddress[i].head = 0;
		_worldAttributesBaseAddress[i].gx = 0;
		_worldAttributesBaseAddress[i].gp = 0;
		_worldAttributesBaseAddress[i].gy = 0;
		_worldAttributesBaseAddress[i].mx = 0;
		_worldAttributesBaseAddress[i].mp = 0;
		_worldAttributesBaseAddress[i].my = 0;
		_worldAttributesBaseAddress[i].w = 0;
		_worldAttributesBaseAddress[i].h = 0;
		_worldAttributesBaseAddress[i].param = 0;
		_worldAttributesBaseAddress[i].ovr = 0;
	}

	for(int32 i = 0; i < __TOTAL_OBJECTS; i++)
	{
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;

		_objectAttributesBaseAddress[i].jx = 0;
		_objectAttributesBaseAddress[i].head = 0;
		_objectAttributesBaseAddress[i].jy = 0;
		_objectAttributesBaseAddress[i].tile = 0;
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
