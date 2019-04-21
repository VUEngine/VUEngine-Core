/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <VIPManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>
#include <ClockManager.h>
#include <Game.h>
#include <FrameRate.h>
#include <SpriteManager.h>
#include <WireframeManager.h>
#include <Mem.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

volatile u16* _vipRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (u16*)0x0005F800;
u32* _currentDrawingFrameBufferSet = NULL;

static VIPManager _vipManager;
static TimerManager _timerManager;
static WireframeManager _wireframeManager;
static SpriteManager _spriteManager;
static HardwareManager _hardwareManager;

extern ColumnTableROMSpec DEFAULT_COLUMN_TABLE;
extern BrightnessRepeatROMSpec DEFAULT_BRIGHTNESS_REPEAT;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


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
	this->processingXPEND = false;
	this->renderingCompleted = false;
	this->allowDRAMAccess = false;

	_vipManager = this;
	_timerManager = TimerManager::getInstance();
	_spriteManager = SpriteManager::getInstance();
	_wireframeManager = WireframeManager::getInstance();
	_hardwareManager = HardwareManager::getInstance();

	_currentDrawingFrameBufferSet = &this->currentDrawingFrameBufferSet;
}

/**
 * Class destructor
 */
void VIPManager::destructor()
{
	delete this->postProcessingEffects;

	// allow a new construct
	Base::destructor();
}

/**
 * Allow VIP's drawing process to start
 */
void VIPManager::enableDrawing()
{
	while(_vipRegisters[__XPSTTS] & __XPBSYR);
	_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | __XPEN;
}

/**
 * Disallow VIP's drawing process to start
 */
void VIPManager::disableDrawing()
{
	_vipRegisters[__XPCTRL] &= ~__XPEN;
}

/**
 * Enable VIP's interrupts
 *
 * @param interruptCode			Interrupts to enable
 */
void VIPManager::enableInterrupt(u16 interruptCode)
{
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
	_vipRegisters[__INTENB]= interruptCode | __TIMEERR;
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
 * Enable / disable DRAM writing
 *
 * @param allowDRAMAccess		Flag's value
 */
void VIPManager::allowDRAMAccess(bool allowDRAMAccess)
{
	this->allowDRAMAccess = allowDRAMAccess;
}

/**
 * Check if rendering is pending
 *
 * @return						True if XPEND already happened but DRAM writing didn't take place
 */
bool VIPManager::isRenderingPending()
{
	return this->drawingEnded && !this->renderingCompleted;
}

/**
 * VIP's interrupt handler
 */
static void VIPManager::interruptHandler()
{
	// save the interrupt event
	u16 interrupt = _vipRegisters[__INTPND];

	// disable interrupts
	VIPManager::disableInterrupts(_vipManager);

	HardwareManager::enableMultiplexedInterrupts();

	// handle the interrupt
	VIPManager::processInterrupt(_vipManager, interrupt);

	HardwareManager::disableMultiplexedInterrupts();

	// enable interrupts
	if(_vipManager->processingXPEND)
	{
		VIPManager::enableInterrupt(_vipManager, __FRAMESTART);
	}
	else
	{
		VIPManager::enableInterrupt(_vipManager, __FRAMESTART | __XPEND);
	}
}
/**
 * Process interrupt method
 */
void VIPManager::processInterrupt(u16 interrupt)
{
	const u16 interruptTable[] =
	{
		__FRAMESTART,
		__XPEND,
		__TIMEERR
	};

	int i = 0;
	const int limit = sizeof(interruptTable) / sizeof(u16);

	for(; i < limit; i++)
	{
		switch(interrupt & interruptTable[i])
		{
			case __FRAMESTART:

#ifdef __PROFILE_GAME
				Game::saveProcessNameDuringFRAMESTART(Game::getInstance());
#endif

				ClockManager::update(ClockManager::getInstance(), __GAME_FRAME_DURATION);

				VIPManager::registerCurrentDrawingFrameBufferSet(this);

				// increase current game frame's duration
				Game::increaseGameFrameDuration(Game::getInstance(), __GAME_FRAME_DURATION);

				Game::currentFrameEnded(Game::getInstance());

				if(!_vipManager->processingXPEND)
				{
					this->drawingEnded = false;
					this->renderingCompleted = false;
				}
				break;

			case __XPEND:

				this->processingXPEND = true;
				this->renderingCompleted = false;

#ifdef __PROFILE_GAME
				Game::saveProcessNameDuringXPEND(Game::getInstance());
#endif

#ifdef __PROFILE_GAME
				{
					s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(_timerManager);
#endif

					// prevent VIP's drawing operations
//#ifndef __ALERT_VIP_OVERTIME
					VIPManager::disableDrawing(this);
//#endif
					// to allow timer interrupts
					VIPManager::enableInterrupt(this, __FRAMESTART);

					if(this->allowDRAMAccess)
					{
						// write to DRAM
						SpriteManager::render(_spriteManager);
						this->renderingCompleted = true;
					}

					// write to the frame buffers
					VIPManager::processFrameBuffers(this);

					// allow VIP's drawing operations
					VIPManager::enableDrawing(this);

					// flag completions
					this->drawingEnded = true;

#ifdef __DEBUG_TOOLS
					if(Game::isInDebugMode(Game::getInstance()))
					{
						Debug::render(Debug::getInstance());
					}
#endif

#ifdef __PROFILE_GAME
					extern s16 _renderingHighestTime;
					extern s16 _renderingProcessTimeHelper;
					extern s16 _renderingProcessTime;
					extern s16 _renderingTotalTime;
					extern bool _updateProfiling;

					if(_updateProfiling)
					{
						_renderingProcessTimeHelper = _renderingProcessTime = TimerManager::getMillisecondsElapsed(_timerManager) - timeBeforeProcess;
						_renderingTotalTime += _renderingProcessTime;
						_renderingHighestTime = _renderingHighestTime < _renderingProcessTime ? _renderingProcessTime : _renderingHighestTime;
					}
				}
#endif

				this->processingXPEND = false;
				break;

			case __TIMEERR:

				Game::increaseGameFrameDuration(Game::getInstance(), __GAME_FRAME_DURATION);

#ifdef __ALERT_VIP_OVERTIME
				{
					static u32 count = 0;
					PRINT_TEXT("VIP Overtime! (   )", 0, 27);
					PRINT_INT(++count, 15, 27);
				}
#endif

				break;
		}
	}
}

/**
 * Start frame buffer writing operations
 *
 * @return			The time in milliseconds that it took to process the interrupt (only if profiling is enabled)
 */
void VIPManager::processFrameBuffers()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	Game::setLastProcessName(Game::getInstance(), "rendering");
#endif

	// draw 3d objects
	WireframeManager::drawWireframes(_wireframeManager);

	// check if the current frame buffer set is valid
	VirtualNode node = this->postProcessingEffects->tail;

	for(; node; node = node->previous)
	{
		((PostProcessingEffectRegistry*)node->data)->postProcessingEffect(this->currentDrawingFrameBufferSet, ((PostProcessingEffectRegistry*)node->data)->spatialObject);
	}
}

/**
 * Turn on the displays
 */
void VIPManager::displayOn()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__FRMCYC] = __FRAME_CYCLE;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE | __DISP);
}

/**
 * Turn off the displays
 */
void VIPManager::displayOff()
{
	_vipRegisters[__REST] = 0;
	_vipRegisters[__XPCTRL] = 0;
	_vipRegisters[__DPCTRL] = 0;
	_vipRegisters[__FRMCYC] = 1;

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

	VIPManager::setBackgroundColor(this, __COLOR_BLACK);
}

/**
 * Clear the CHAR and Param table memory
 */
void VIPManager::clearScreen()
{	u8* bgmapStartAddress = (u8*)__BGMAP_SPACE_BASE_ADDRESS;

	// clear every bgmap segment
	for(bgmapStartAddress = 0; bgmapStartAddress < (u8*)__PARAM_TABLE_END; bgmapStartAddress++)
	{
		*bgmapStartAddress = 0;
	}

	// clear every char segment
	Mem::clear ((BYTE*) __CHAR_SEGMENT_0_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_1_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_2_BASE_ADDRESS, 8192);
	Mem::clear ((BYTE*) __CHAR_SEGMENT_3_BASE_ADDRESS, 8192);
}

/**
 * Clear a BGMAP segment
 *
 * @param segment	The segment to clean up
 * @param size		Segment's size
 */
void VIPManager::clearBgmapSegment(int segment, int size)
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
	int i, value;

	// use the default column table as fallback
	if(columnTableSpec == NULL)
	{
		columnTableSpec = (ColumnTableSpec*)&DEFAULT_COLUMN_TABLE;
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
		_vipRegisters[__DPCTRL] &= __LOCK;
	}
}

/**
 * Write brightness repeat values to column table
 *
 * @param brightnessRepeatSpec	Spec
 */
void VIPManager::setupBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeatSpec)
{
	int i, leftCta, rightCta, value;

	// use the default repeat values as fallback
	if(brightnessRepeatSpec == NULL)
	{
		brightnessRepeatSpec = (BrightnessRepeatSpec*)&DEFAULT_BRIGHTNESS_REPEAT;
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
void VIPManager::setBackgroundColor(u8 color)
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
bool VIPManager::isPostProcessingEffectRegistered(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			return true;
		}
	}

	return false;
}

/**
 * Register a post-processing effect with a higher priority
 *
 * @param postProcessingEffect	Post-processing effect function
 * @param spatialObject			Post-processing effect function's scope
 */
void VIPManager::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	if(VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject))
	{
//		return;
	}

	PostProcessingEffectRegistry* postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;

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
	if(VIPManager::isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject))
	{
		return;
	}

	PostProcessingEffectRegistry* postProcessingEffectRegistry = new PostProcessingEffectRegistry;
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;

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
	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			VirtualList::removeElement(this->postProcessingEffects, postProcessingEffectRegistry);

			delete postProcessingEffectRegistry;
			return;
		}
	}
}

/**
 * Remove all a post-processing effects
 */
void VIPManager::removePostProcessingEffects()
{
	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		delete node->data;
	}

	VirtualList::clear(this->postProcessingEffects);
}

/**
 * Register the frame buffer in use by the VIP's drawing process
 */
void VIPManager::registerCurrentDrawingFrameBufferSet()
{
	u32 currentDrawingFrameBufferSet = _vipRegisters[__XPSTTS] & 0x000C;

	this->currentDrawingFrameBufferSet = 0;

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
u32 VIPManager::getCurrentDrawingframeBufferSet()
{
	return this->currentDrawingFrameBufferSet;
}
