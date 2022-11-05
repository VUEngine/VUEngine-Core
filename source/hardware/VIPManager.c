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

#include <VIPManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>
#include <VUEngine.h>
#include <FrameRate.h>
#include <SpriteManager.h>
#include <DirectDraw.h>
#include <WireframeManager.h>
#include <Profiler.h>
#include <Mem.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

WorldAttributes _worldAttributesCache[__TOTAL_LAYERS] __attribute__((section(".dram_bss")));
ObjectAttributes _objectAttributesCache[1024] __attribute__((section(".dram_bss")));

volatile uint16* _vipRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (uint16*)0x0005F800;
uint32* _currentDrawingFrameBufferSet = NULL;

static VIPManager _vipManager = NULL;
static WireframeManager _wireframeManager = NULL;
static SpriteManager _spriteManager = NULL;
static DirectDraw _directDraw = NULL;

extern ColumnTableROMSpec DefaultColumnTable;
extern BrightnessRepeatROMSpec DefaultBrightnessRepeat;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#ifdef __ENABLE_PROFILER
#undef __FORCE_VIP_SYNC
//#define __FORCE_VIP_SYNC
#endif

friend class VirtualNode;
friend class VirtualList;


/**
 * Texture Post Processing Effect Registry
 *
 * @memberof VIPManager
 */
typedef struct PostProcessingEffectRegistry
{
	PostProcessingEffect postProcessingEffect;
	SpatialObject spatialObject;
	bool remove;

} PostProcessingEffectRegistry;



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			VIPManager::getInstance()
 * @memberof	VIPManager
 * @public
 * @return		VIPManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void VIPManager::constructor()
{
	Base::constructor();

	this->postProcessingEffects = new VirtualList();
	this->currentDrawingFrameBufferSet = 0;
	this->drawingEnded = false;
	this->frameStartedDuringXPEND = false;
	this->processingXPEND = false;
	this->processingGAMESTART = false;
	this->customInterrupts = 0;
	this->currrentInterrupt = 0;
	this->skipFrameBuffersProcessing = false;
	this->multiplexedGAMESTARTCounter = 0;
	this->multiplexedXPENDCounter = 0;
	this->timeErrorCounter = 0;
	this->scanErrorCounter = 0;
	this->totalMilliseconds = 0;

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);

	_vipManager = this;
	_spriteManager = SpriteManager::getInstance();
	_wireframeManager = WireframeManager::getInstance();
	_directDraw = DirectDraw::getInstance();

	_currentDrawingFrameBufferSet = &this->currentDrawingFrameBufferSet;
}

/**
 * Class destructor
 */
void VIPManager::destructor()
{
	VIPManager::removePostProcessingEffects(this);

	delete this->postProcessingEffects;

	// allow a new construct
	Base::destructor();
}

void VIPManager::reset()
{
	this->customInterrupts = 0;
	this->currrentInterrupt = 0;
	this->skipFrameBuffersProcessing = false;
	this->processingGAMESTART = false;
	this->processingXPEND = false;
	this->drawingEnded = false;
	this->totalMilliseconds = 0;

	VIPManager::setFrameCycle(this, __FRAME_CYCLE);
}

void VIPManager::setSkipFrameBuffersProcessing(bool skipFrameBuffersProcessing)
{
	this->skipFrameBuffersProcessing = skipFrameBuffersProcessing;
}

void VIPManager::enableCustomInterrupts(uint16 customInterrupts)
{
	this->customInterrupts = customInterrupts;
}

/**
 * Allow VIP's drawing process to start
 */
void VIPManager::enableDrawing()
{
	_vipRegisters[__XPCTRL] |= __XPEN;
}

/**
 * Disallow VIP's drawing process to start
 */
void VIPManager::disableDrawing()
{
	while(_vipRegisters[__XPSTTS] & __XPBSYR);
	_vipRegisters[__XPCTRL] &= ~__XPEN;
}

/**
 * Return true if rendering is allowed
 */
bool VIPManager::isDrawingAllowed()
{
	return _vipRegisters[__XPSTTS] & __XPEN ? true : false;
}

/**
 * Return true if FRAMESTART happened during XPEND's processing
 */
__attribute__((noinline))
bool VIPManager::hasFrameStartedDuringXPEND()
{
	return this->frameStartedDuringXPEND;
}

/**
 * Return game frame duration
 */
uint16 VIPManager::getGameFrameDuration()
{
	return this->gameFrameDuration;
}

/**
 * Enable VIP's interrupts
 *
 * @param interruptCode			Interrupts to enable
 */
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

/**
 * Disable VIP's interrupts
 */
void VIPManager::disableInterrupts()
{
	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}

/**
 * Check if rendering is pending
 *
 * @return						True if XPEND already happened but DRAM writing didn't take place
 */
bool VIPManager::isRenderingPending()
{
	return !this->drawingEnded;
}

uint16 VIPManager::getCurrentInterrupt()
{
	return this->currrentInterrupt;
}

/**
 * VIP's interrupt handler
 */
static void VIPManager::interruptHandler()
{
	// save the interrupt event
	_vipManager->currrentInterrupt = _vipRegisters[__INTPND];

	// disable interrupts
	VIPManager::disableInterrupts(_vipManager);

	HardwareManager::enableMultiplexedInterrupts();

#ifdef __VIP_MANAGER_FIRE_INTERRUPT_EVENT
	if(_vipManager->events)
	{
		VIPManager::fireEvent(_vipManager, kEventVIPManagerInterrupt);
	}
#endif

	// handle the interrupt
	VIPManager::processInterrupt(_vipManager, _vipManager->currrentInterrupt);

	HardwareManager::disableMultiplexedInterrupts();

	// enable interrupts
	VIPManager::enableInterrupts(_vipManager, __GAMESTART | __XPEND);
}

static void VIPManager::updateVRAM()
{
	SpriteManager::writeDRAM(_spriteManager);

	DirectDraw::startDrawing(_directDraw);
	WireframeManager::draw(_wireframeManager);
	VIPManager::applyPostProcessingEffects(_vipManager);
}

/**
 * Process interrupt method
 */
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

				this->totalMilliseconds += __MILLISECONDS_PER_SECOND / __MAXIMUM_FPS;
				this->frameStartedDuringXPEND = this->processingXPEND;
				VUEngine::nextFrameStarted(_vuEngine, __MILLISECONDS_PER_SECOND / __MAXIMUM_FPS);

#ifdef __ENABLE_PROFILER
				if(0 == (__GAMESTART & interrupt))
				{
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeVIPInterruptFRAMESTARTProcess, PROCESS_NAME_RENDER);
				}
#endif
				break;

			case __GAMESTART:

#ifdef __REGISTER_PROCESS_NAME_DURING_FRAMESTART
				VUEngine::saveProcessNameDuringGAMESTART(_vuEngine);
#endif

				this->drawingEnded = false;
				this->processingGAMESTART = true;

				if(this->processingXPEND)
				{
					this->multiplexedGAMESTARTCounter++;

					VIPManager::fireEvent(this, kEventVIPManagerGAMESTARTDuringXPEND);
				}
				else
				{
					// Listen for the end of drawing operations
					VIPManager::enableInterrupts(this, __XPEND);
				}

				// Configure the drawing frame buffers
				VIPManager::registerCurrentDrawingFrameBufferSet(this);

				// Process game's logic
				VUEngine::nextGameCycleStarted(_vuEngine, this->gameFrameDuration);

				// The VIP finished drawing the current frame when the game was being rendered
				// so it didn't touch VRAM during the last XPEND
				if(this->drawingEnded)
				{
					VIPManager::updateVRAM();
				}	

				this->processingGAMESTART = false;

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeVIPInterruptGAMESTARTProcess, PROCESS_NAME_RENDER);
#endif

#ifdef __SHOW_VIP_STATUS
				VIPManager::print(this, 1, 2);
#endif

#ifdef __DEBUG_TOOLS
				if(VUEngine::isInDebugMode(_vuEngine))
				{
					Debug::render(Debug::getInstance());
				}
#endif
				break;

			case __XPEND:

#ifdef __REGISTER_PROCESS_NAME_DURING_XPEND
				VUEngine::saveProcessNameDuringXPEND(_vuEngine);
#endif

				if(this->processingGAMESTART)
				{
					this->multiplexedXPENDCounter++;

					VIPManager::fireEvent(this, kEventVIPManagerXPENDDuringGAMESTART);
				}
				else
				{
					this->processingXPEND = true;

					// Allow game start interrupt
					VIPManager::enableInterrupts(this, __GAMESTART);

					// Write to VRAM
					VIPManager::updateVRAM();

					this->processingXPEND = false;
				}

				this->drawingEnded = true;

#ifdef __ENABLE_PROFILER
				Profiler::lap(Profiler::getInstance(), kProfilerLapTypeVIPInterruptXPENDProcess, PROCESS_NAME_VRAM_WRITE);
#endif
				break;

#ifndef __SHIPPING
			case __TIMEERR:

				this->timeErrorCounter++;
				VIPManager::fireEvent(this, kEventVIPManagerTimeError);
				break;

			case __SCANERR:

				this->scanErrorCounter++;
				VIPManager::fireEvent(this, kEventVIPManagerScanError);
				break;
#endif
		}
	}
}

/**
 * Start frame buffer writing operations
 *
 * @return			The time in milliseconds that it took to process the interrupt (only if profiling is enabled)
 */
void VIPManager::applyPostProcessingEffects()
{
	for(VirtualNode node = this->postProcessingEffects->tail, previousNode = NULL; !VIPManager::hasFrameStartedDuringXPEND(this) && NULL != node; node = previousNode)
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
		else if(!this->skipFrameBuffersProcessing)
		{
			postProcessingEffectRegistry->postProcessingEffect(this->currentDrawingFrameBufferSet, postProcessingEffectRegistry->spatialObject);
		}
	}
}

void VIPManager::setFrameCycle(uint8 frameCycle)
{
	this->frameCycle = frameCycle;

	if(3 < this->frameCycle)
	{
		this->frameCycle = 3;
	}

	this->gameFrameDuration = (__MILLISECONDS_PER_SECOND / __MAXIMUM_FPS) << this->frameCycle;

	VIPManager::displayOn(this);
}

/**
 * Turn on the displays
 */
void VIPManager::displayOn()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__FRMCYC] = this->frameCycle;
	_vipRegisters[__DPCTRL] = (_vipRegisters[__DPSTTS] | (__SYNCE | __RE | __DISP)) & ~__LOCK;
}

/**
 * Turn off the displays
 */
void VIPManager::displayOff()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__XPCTRL] = 0;
	_vipRegisters[__DPCTRL] = 0;
	_vipRegisters[__FRMCYC] = 3;

	VIPManager::disableInterrupts(this);
}

/**
 * Setup the palettes
 *
 * @param paletteConfig			Configuration of the palettes
 */
void VIPManager::setupPalettes(PaletteConfig* paletteConfig)
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

/**
 * Turn brightness all the way up
 */
void VIPManager::upBrightness()
{
	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}

/**
 * Turn brightness all the way down
 */
void VIPManager::lowerBrightness()
{
	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;
}

/**
 * Clear the CHAR and Param table memory
 */
void VIPManager::clearScreen()
{	uint8* bgmapStartAddress = (uint8*)__BGMAP_SPACE_BASE_ADDRESS;

	// clear every bgmap segment
	for(bgmapStartAddress = 0; bgmapStartAddress < (uint8*)__PARAM_TABLE_END; bgmapStartAddress++)
	{
		*bgmapStartAddress = 0;
	}

	// clear every char segment
	Mem::clear ((BYTE*) __CHAR_SEGMENT_0_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_1_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_2_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_3_BASE_ADDRESS, 8192);

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

	for(int32 i = 0; i < __AVAILABLE_CHAR_OBJECTS; i++)
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

/**
 * Clear a BGMAP segment
 *
 * @param segment	The segment to clean up
 * @param size		Segment's size
 */
void VIPManager::clearBgmapSegment(int32 segment, int32 size)
{
	Mem::clear((BYTE*)__BGMAP_SEGMENT(segment), size * 2);
}

/**
 * Setup the column table
 *
 * @param columnTableSpec		Spec to use
 */
void VIPManager::setupColumnTable(ColumnTableSpec* columnTableSpec)
{
	int32 i, value;

	// use the default column table as fallback
	if(columnTableSpec == NULL)
	{
		columnTableSpec = (ColumnTableSpec*)&DefaultColumnTable;
	}

	// write column table
	for(i = 0; i < 256; i++)
	{
		value = (columnTableSpec->mirror && (i > 127))
			? columnTableSpec->columnTable[255 - i]
			: columnTableSpec->columnTable[i];

		_columnTableBaseAddressLeft[i] = value;
		_columnTableBaseAddressRight[i] = value;
	}
}

/**
 * Use the vip's built-in column table instead of reading the one defined in memory
 *
 * @param useInternal	Flag
 */
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

/**
 * Write brightness values
 *
 * @param brightness	Struct
 */
void VIPManager::setupBrightness(Brightness* brightness)
{
	_vipRegisters[__BRTA] = brightness->darkRed;
	_vipRegisters[__BRTB] = brightness->mediumRed;
	_vipRegisters[__BRTC] = brightness->brightRed - (brightness->mediumRed + brightness->darkRed);
}

/**
 * Write brightness repeat values to column table
 *
 * @param brightnessRepeatSpec	Spec
 */
void VIPManager::setupBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeatSpec)
{
	int32 i, leftCta, rightCta, value;

	// use the default repeat values as fallback
	if(brightnessRepeatSpec == NULL)
	{
		brightnessRepeatSpec = (BrightnessRepeatSpec*)&DefaultBrightnessRepeat;
	}

	// column table offsets
	leftCta = _vipRegisters[__CTA] & 0xFF;
	rightCta = _vipRegisters[__CTA] >> 8;

	// write repeat values to column table
	for(i = 0; i < 96; i++)
	{
		value = (brightnessRepeatSpec->mirror && (i > 47))
			? brightnessRepeatSpec->brightnessRepeat[95 - i] << 8
			: brightnessRepeatSpec->brightnessRepeat[i] << 8;

		_columnTableBaseAddressLeft[leftCta - i] = (_columnTableBaseAddressLeft[leftCta - i] & 0xff) | value;
		_columnTableBaseAddressRight[rightCta - i] = (_columnTableBaseAddressRight[rightCta - i] & 0xff) | value;
	}
}

/**
 * Set background color
 *
 * @param color		New color
 */
void VIPManager::setBackgroundColor(uint8 color)
{
	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}

/**
 * Check if a post-processing effect is already registered
 *
 * @param postProcessingEffect	Post-processing effect function
 * @param spatialObject			Post-processing effect function's scope
 * @return						Whether the effect and object are already registered
 */
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

/**
 * Register a post-processing effect with a higher priority
 *
 * @param postProcessingEffect	Post-processing effect function
 * @param spatialObject			Post-processing effect function's scope
 */
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

/**
 * Register a post-processing effect with lower priority
 *
 * @param postProcessingEffect	Post-processing effect function
 * @param spatialObject			Post-processing effect function's scope
 */
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

/**
 * Remove a post-processing effect
 *
 * @param postProcessingEffect	Post-processing effect function
 * @param spatialObject			Post-processing effect function's scope
 */
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

/**
 * Remove all a post-processing effects
 */
void VIPManager::removePostProcessingEffects()
{
	VirtualList::deleteData(this->postProcessingEffects);
}

/**
 * Register the frame buffer in use by the VIP's drawing process
 */
void VIPManager::registerCurrentDrawingFrameBufferSet()
{
	uint16 currentDrawingFrameBufferSet = _vipRegisters[__XPSTTS] & __XPBSYR;

	if(0x0004 == currentDrawingFrameBufferSet)
	{
		this->currentDrawingFrameBufferSet = 0;
	}
	else if(0x0008 == currentDrawingFrameBufferSet)
	{
		this->currentDrawingFrameBufferSet = 0x8000;
	}
}

/**
 * Retrieve the frame buffer in use by the VIP's drawing process
 *
 * @return	Frame buffer in use by the VIP's drawing process
 */
uint32 VIPManager::getCurrentDrawingframeBufferSet()
{
	return this->currentDrawingFrameBufferSet;
}

/**
 * Retrieve the block being drawn by the VIP
 *
 * @return	The number of the block being drawn by the VIP
 */
int16 VIPManager::getCurrentBlockBeingDrawn()
{
	if(_vipRegisters[__XPSTTS] & __SBOUT)
	{
		return (_vipRegisters[__XPSTTS] & __SBCOUNT) >> 8;
	}

	if(!(_vipRegisters[__XPSTTS] & __XPBSYR))
	{
		return -1;
	}

	while(!(_vipRegisters[__XPSTTS] & __SBOUT));

	return (_vipRegisters[__XPSTTS] & __SBCOUNT) >> 8;
}

/**
 * Print VIP's status
 *
 */
void VIPManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "VIP Status", x, y++, NULL);
	Printing::text(Printing::getInstance(), "TIMEERR counter:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->timeErrorCounter, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "SCANERR counter:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->scanErrorCounter, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Multi FRAMESTARTS:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->multiplexedGAMESTARTCounter, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Multi XPENDs:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->multiplexedXPENDCounter, x + 18, y, NULL);
}

void VIPManager::wait(uint32 milliSeconds)
{
	uint32 waitStartTime = this->totalMilliseconds;
	volatile uint32 *milliseconds = (uint32*)&this->totalMilliseconds;

	while ((*milliseconds - waitStartTime) < milliSeconds)
	{
		HardwareManager::halt();
	}
}
