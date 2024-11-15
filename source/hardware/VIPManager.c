/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>
#include <DirectDraw.h>
#include <HardwareManager.h>
#include <Mem.h>
#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <Printing.h>
#include <Profiler.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "VIPManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;

extern ColumnTableROMSpec DefaultColumnTableSpec;
extern BrightnessRepeatROMSpec DefaultBrightnessRepeatSpec;
extern uint32 _dramDirtyStart;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Processing Effect Registry
/// @memberof VIPManager
typedef struct PostProcessingEffectRegistry
{
	PostProcessingEffect postProcessingEffect;
	SpatialObject spatialObject;
	bool remove;

} PostProcessingEffectRegistry;


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

WorldAttributes _worldAttributesCache[__TOTAL_LAYERS] __attribute__((section(".dram_bss")));
ObjectAttributes _objectAttributesCache[__TOTAL_OBJECTS] __attribute__((section(".dram_bss")));

volatile uint16* _vipRegisters __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = (uint16*)0x0005F800;
uint32* _currentDrawingFrameBufferSet __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

static VIPManager _vipManager = NULL;
static DirectDraw _directDraw = NULL;
static WireframeManager _wireframeManager = NULL;
static SpriteManager _spriteManager = NULL;
static uint16* const _columnTableBaseAddressLeft =	(uint16*)0x0003DC00; // base address of Column Table (Left Eye)
static uint16* const _columnTableBaseAddressRight =	(uint16*)0x0003DE00; // base address of Column Table (Right Eye)


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void VIPManager::interruptHandler()
{
	_vipManager->currrentInterrupt = _vipRegisters[__INTPND];

	// disable interrupts
	VIPManager::disableInterrupts(_vipManager);

#ifndef __DEBUG
	if(kVIPNoMultiplexedInterrupts != _vipManager->enabledMultiplexedInterrupts)
	{
		if(kVIPOnlyVIPMultiplexedInterrupts == _vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(_vipManager->enabledMultiplexedInterrupts);
		}

		HardwareManager::enableMultiplexedInterrupts();
	}
#endif

#ifdef __VIP_MANAGER_FIRE_INTERRUPT_EVENT
	if(_vipManager->events)
	{
		VIPManager::fireEvent(_vipManager, kEventVIPManagerInterrupt);
	}
#endif

	// handle the interrupt
	VIPManager::processInterrupt(_vipManager, _vipManager->currrentInterrupt);

#ifndef __DEBUG
	if(kVIPNoMultiplexedInterrupts != _vipManager->enabledMultiplexedInterrupts)
	{
		HardwareManager::disableMultiplexedInterrupts();

		if(kVIPOnlyVIPMultiplexedInterrupts == _vipManager->enabledMultiplexedInterrupts)
		{
			HardwareManager::setInterruptLevel(0);
		}
	}
#endif

	_vipManager->currrentInterrupt = 0;

	VIPManager::enableInterrupts(_vipManager, __GAMESTART | __XPEND);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VIPManager::reset()
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

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);
	VIPManager::setupColumnTable(this, NULL);

	VIPManager::clearDRAM(this);
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::enableCustomInterrupts(uint16 customInterrupts)
{
	this->customInterrupts = customInterrupts;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::enableMultiplexedInterrupts(uint32 enabledMultiplexedInterrupts __attribute__((unused)))
{
#ifndef __ENABLE_PROFILER
	this->enabledMultiplexedInterrupts = enabledMultiplexedInterrupts;
#else
	this->enabledMultiplexedInterrupts = kVIPNoMultiplexedInterrupts;
#endif
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::startDrawing()
{
	this->isDrawingAllowed = true;

	VIPManager::enableInterrupts(this, __FRAMESTART | __XPEND);

	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__XPCTRL] |= __XPEN;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::resumeDrawing()
{
	if(this->isDrawingAllowed)
	{
		while(_vipRegisters[__XPSTTS] & __XPBSY);
		_vipRegisters[__XPCTRL] |= __XPEN;
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::suspendDrawing()
{
	this->isDrawingAllowed = VIPManager::isDrawingAllowed(this);
	
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__XPCTRL] &= ~__XPEN;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::stopDrawing()
{
	this->isDrawingAllowed = false;

	VIPManager::disableInterrupts(this);

	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__XPCTRL] &= ~__XPEN;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::startDisplaying()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = (__SYNCE | __RE | __DISP) & ~__LOCK;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::stopDisplaying()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = 0;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::setFrameCycle(uint8 frameCycle __attribute__((unused)))
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
//---------------------------------------------------------------------------------------------------------
void VIPManager::setupPalettes(PaletteConfig* paletteConfig)
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);

	_vipRegisters[__GPLT0] = paletteConfig->bgmap.gplt0;
	_vipRegisters[__GPLT1] = paletteConfig->bgmap.gplt1;
	_vipRegisters[__GPLT2] = paletteConfig->bgmap.gplt2;
	_vipRegisters[__GPLT3] = paletteConfig->bgmap.gplt3;

	_vipRegisters[__JPLT0] = paletteConfig->object.jplt0;
	_vipRegisters[__JPLT1] = paletteConfig->object.jplt1;
	_vipRegisters[__JPLT2] = paletteConfig->object.jplt2;
	_vipRegisters[__JPLT3] = paletteConfig->object.jplt3;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::setupColumnTable(ColumnTableSpec* columnTableSpec)
{
	int32 i, value;

	// use the default column table as fallback
	if(columnTableSpec == NULL)
	{
		columnTableSpec = (ColumnTableSpec*)&DefaultColumnTableSpec;
	}

	// write column table
	for(i = 0; i < 256; i++)
	{
		value = (columnTableSpec->mirror && (i > (__COLUMN_TABLE_ENTRIES / 2 - 1)))
			? columnTableSpec->columnTable[(__COLUMN_TABLE_ENTRIES - 1) - i]
			: columnTableSpec->columnTable[i];

		_columnTableBaseAddressLeft[i] = value;
		_columnTableBaseAddressRight[i] = value;
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::setupBrightness(Brightness* brightness)
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = brightness->darkRed;
	_vipRegisters[__BRTB] = brightness->mediumRed;
	_vipRegisters[__BRTC] = brightness->brightRed - (brightness->mediumRed + brightness->darkRed);
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::setupBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeatSpec)
{
	// use the default repeat values as fallback
	if(brightnessRepeatSpec == NULL)
	{
		brightnessRepeatSpec = (BrightnessRepeatSpec*)&DefaultBrightnessRepeatSpec;
	}
	// column table offsets
	int16 leftCta = _vipRegisters[__CTA] & 0xFF;
	int16 rightCta = _vipRegisters[__CTA] >> 8;

	CACHE_RESET;

	// write repeat values to column table
	for(int16 i = 0; i < 96; i++)
	{
		int16 value = (brightnessRepeatSpec->mirror && (i > (__BRIGHTNESS_REPEAT_ENTRIES / 2 - 1)))
			? brightnessRepeatSpec->brightnessRepeat[__BRIGHTNESS_REPEAT_ENTRIES - 1 - i] << 8
			: brightnessRepeatSpec->brightnessRepeat[i] << 8;

		_columnTableBaseAddressLeft[leftCta - i] = (_columnTableBaseAddressLeft[leftCta - i] & 0xff) | value;
		_columnTableBaseAddressRight[rightCta - i] = (_columnTableBaseAddressRight[rightCta - i] & 0xff) | value;
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::setBackgroundColor(uint8 color)
{
	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::upBrightness()
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::lowerBrightness()
{
	while(_vipRegisters[__XPSTTS] & __XPBSY);
	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	PostProcessingEffectRegistry* postProcessingEffectRegistry = VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushFront(this->postProcessingEffects, postProcessingEffectRegistry);
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	PostProcessingEffectRegistry* postProcessingEffectRegistry = VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject);

	if(!isDeleted(postProcessingEffectRegistry))
	{
		postProcessingEffectRegistry->remove = false;
		return;
	}

	postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;
	postProcessingEffectRegistry->remove = false;

	VirtualList::pushBack(this->postProcessingEffects, postProcessingEffectRegistry);
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	for(VirtualNode node = this->postProcessingEffects->head; NULL != node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			postProcessingEffectRegistry->remove = true;
			return;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::removePostProcessingEffects()
{
	VirtualList::deleteData(this->postProcessingEffects);
}
//---------------------------------------------------------------------------------------------------------
uint16 VIPManager::getCurrentInterrupt()
{
	return this->currrentInterrupt;
}
//---------------------------------------------------------------------------------------------------------
uint16 VIPManager::getGameFrameDuration()
{
	return this->gameFrameDuration;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VIPManager::constructor()
{
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

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);

	_vipManager = this;
	_spriteManager = SpriteManager::getInstance();
	_directDraw = DirectDraw::getInstance();
	_wireframeManager = WireframeManager::getInstance();

	_currentDrawingFrameBufferSet = &this->currentDrawingFrameBufferSet;
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::destructor()
{
	VIPManager::removePostProcessingEffects(this);

	delete this->postProcessingEffects;

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::processInterrupt(uint16 interrupt)
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

#ifdef __SHOW_PROCESS_NAME_DURING_FRAMESTART
				PRINT_TEXT("F START:            ", 0, 27);
				PRINT_TEXT(VUEngine::getLastProcessName(_vuEngine), 9, 27);
#endif

				this->FRAMESTARTDuringXPEND = this->processingXPEND;
				VUEngine::nextFrameStarted(_vuEngine, __MILLISECONDS_PER_SECOND / __MAXIMUM_FPS);
				break;

			case __GAMESTART:

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeStartInterrupt, NULL);
#endif

#ifdef __SHOW_PROCESS_NAME_DURING_GAMESTART
				PRINT_TEXT("G START:           ", 0, 26);
				PRINT_TEXT(VUEngine::getLastProcessName(_vuEngine), 9, 26);
#endif

				this->processingGAMESTART = true;

				// Configure the drawing frame buffers
				VIPManager::registerCurrentDrawingFrameBufferSet(this);

				if(this->processingXPEND)
				{
					if(NULL != this->events)
					{
						VIPManager::fireEvent(this, kEventVIPManagerGAMESTARTDuringXPEND);
					}
				}
				else
				{
					// Listen for the end of drawing operations
					if(!(__XPEND & interrupt) && 0 != (kVIPAllMultiplexedInterrupts & this->enabledMultiplexedInterrupts))
					{
						VIPManager::enableInterrupts(this, __XPEND);
					}
				}

				// Process game's logic
				VUEngine::nextGameCycleStarted(_vuEngine, this->gameFrameDuration);
				SpriteManager::render(_spriteManager);
				WireframeManager::render(_wireframeManager);

				this->processingGAMESTART = false;

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeVIPInterruptGAMESTARTProcess, PROCESS_NAME_RENDER);
#endif

#ifdef __SHOW_VIP_STATUS
				VIPManager::print(this, 1, 2);
#endif
				break;

			case __XPEND:

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeStartInterrupt, NULL);
#endif

#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
				PRINT_TEXT("XPEND:            ", 0, 27);
				PRINT_TEXT(VUEngine::getLastProcessName(_vuEngine), 9, 27);
#endif

				this->processingXPEND = true;

				if(this->processingGAMESTART)
				{
					if(NULL != this->events)
					{
						VIPManager::fireEvent(this, kEventVIPManagerXPENDDuringGAMESTART);
					}
				}
				else
				{
#ifdef __RELEASE
					_vipRegisters[__XPCTRL] &= ~__XPEN;
#else
					VIPManager::suspendDrawing(this);
#endif

					// Allow game start interrupt because the frame buffers can change mid drawing
					if(!(__GAMESTART & interrupt) && 0 != (kVIPGameStartMultiplexedInterrupts & this->enabledMultiplexedInterrupts))
					{
						VIPManager::enableInterrupts(this, __GAMESTART);
					}
				}

				SpriteManager::writeDRAM(_spriteManager);
				DirectDraw::preparteToDraw(_directDraw);
				WireframeManager::draw(_wireframeManager);
				VIPManager::applyPostProcessingEffects(_vipManager);

				this->processingXPEND = false;

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeVIPInterruptXPENDProcess, PROCESS_NAME_VRAM_WRITE);
#endif

				if(!this->processingGAMESTART)
				{
#ifdef __RELEASE
					_vipRegisters[__XPCTRL] |= __XPEN;
#else
					VIPManager::resumeDrawing(this);
#endif
				}

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
//---------------------------------------------------------------------------------------------------------
void VIPManager::enableInterrupts(uint16 interruptCode)
{
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];

	interruptCode |= this->customInterrupts;

#ifndef __SHIPPING
	_vipRegisters[__INTENB] = interruptCode | __FRAMESTART | __TIMEERR | __SCANERR;
#else
	_vipRegisters[__INTENB] = interruptCode | __FRAMESTART;
#endif
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::disableInterrupts()
{
	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}
void VIPManager::applyPostProcessingEffects()
{
	for(VirtualNode node = this->postProcessingEffects->tail, previousNode = NULL; !this->FRAMESTARTDuringXPEND && NULL != node; node = previousNode)
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
			postProcessingEffectRegistry->postProcessingEffect(this->currentDrawingFrameBufferSet, postProcessingEffectRegistry->spatialObject);
		}
	}
}
void VIPManager::clearDRAM()
{	
	uint8* bgmapStartAddress = (uint8*)__BGMAP_SPACE_BASE_ADDRESS;

	// clear every bgmap segment
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
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;

		_objectAttributesBaseAddress[i].jx = 0;
		_objectAttributesBaseAddress[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
		_objectAttributesBaseAddress[i].jy = 0;
		_objectAttributesBaseAddress[i].tile = 0;
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::useInternalColumnTable(bool useInternal)
{
	// TODO: why does this not work?
	if(useInternal)
	{
		// set lock bit
		_vipRegisters[__DPCTRL] |= __LOCK;
	}
	else
	{
		// unset lock bit
		_vipRegisters[__DPCTRL] &= ~__LOCK;
	}
}
//---------------------------------------------------------------------------------------------------------
void VIPManager::registerCurrentDrawingFrameBufferSet()
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
//---------------------------------------------------------------------------------------------------------
bool VIPManager::isDrawingAllowed()
{
	return 0 != (_vipRegisters[__XPSTTS] & __XPEN);
}
//---------------------------------------------------------------------------------------------------------
PostProcessingEffectRegistry* VIPManager::isPostProcessingEffectRegistered(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VirtualNode node = this->postProcessingEffects->head;

	for(; NULL != node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			return postProcessingEffectRegistry;
		}
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void VIPManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "VIP Status", x, y++, NULL);
}
#endif
//---------------------------------------------------------------------------------------------------------
