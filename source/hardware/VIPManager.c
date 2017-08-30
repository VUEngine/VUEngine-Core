/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <PolyhedronManager.h>
#include <Mem.h>
#include <Printing.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

volatile u16* _vipRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (u16*)0x0005F800;
u32* _currentDrawingFrameBufferSet = NULL;

//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern ColumnTableROMDef DEFAULT_COLUMN_TABLE;
extern BrightnessRepeatROMDef DEFAULT_BRIGHTNESS_REPEAT;

/**
 * Texture Post Processing Effect Registry
 *
 * @memberof VIPManager
 */
typedef struct PostProcessingEffectRegistry
{
	PostProcessingEffect postProcessingEffect;
	SpatialObject spatialObject;

} PostProcessingEffectRegistry;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define VIPManager_ATTRIBUTES																			\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* dram managers */																				\
		/* post processing effects */																	\
		VirtualList postProcessingEffects;																\
		u32 currentDrawingFrameBufferSet;																\
		bool processingXPEND;																			\
		bool drawingEnded;																				\
		bool renderingCompleted;																		\
		bool allowDRAMAccess;																			\


/**
 * @class	VIPManager
 * @extends Object
 * @ingroup hardware
 */
__CLASS_DEFINITION(VIPManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Game_currentFrameEnded(Game this);
void Game_increaseGameFrameDuration(Game this, u32 gameFrameDuration);

#ifdef __PROFILE_GAME
void Game_saveProcessNameDuringFRAMESTART(Game this);
void Game_saveProcessNameDuringXPEND(Game this);
#endif

static VIPManager _vipManager;
static TimerManager _timerManager;
static PolyhedronManager _polyhedronManager;
static SpriteManager _spriteManager;
static HardwareManager _hardwareManager;

static void VIPManager_constructor(VIPManager this);
static void VIPManager_processFrameBuffers(VIPManager this);
static void VIPManager_processInterrupt(VIPManager this, u16 interrupt);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			VIPManager_getInstance()
 * @memberof	VIPManager
 * @public
 *
 * @return		VIPManager instance
 */
__SINGLETON(VIPManager);

/**
 * Class constructor
 *
 * @memberof	VIPManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) VIPManager_constructor(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->postProcessingEffects = __NEW(VirtualList);
	this->currentDrawingFrameBufferSet = 0;
	this->drawingEnded = false;
	this->processingXPEND = false;
	this->renderingCompleted = false;
	this->allowDRAMAccess = false;

	_vipManager = this;
	_timerManager = TimerManager_getInstance();
	_spriteManager = SpriteManager_getInstance();
	_polyhedronManager = PolyhedronManager_getInstance();
	_hardwareManager = HardwareManager_getInstance();

	_currentDrawingFrameBufferSet = &this->currentDrawingFrameBufferSet;
}

/**
 * Class destructor
 *
 * @memberof	VIPManager
 * @public
 *
 * @param this	Function scope
 */
void VIPManager_destructor(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::destructor: null this");

	__DELETE(this->postProcessingEffects);

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Allow VIP's drawing process to start
 *
 * @memberof	VIPManager
 * @public
 *
 * @param this	Function scope
 */
void VIPManager_enableDrawing(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::enableDrawing: null this");

	while(_vipRegisters[__XPSTTS] & __XPBSYR);
	_vipRegisters[__XPCTRL] = (_vipRegisters[__XPSTTS] | __XPEN) & 0xFFFE;
}

/**
 * Disallow VIP's drawing process to start
 *
 * @memberof	VIPManager
 * @public
 *
 * @param this	Function scope
 */
void VIPManager_disableDrawing(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::disableDrawing: null this");

	_vipRegisters[__XPCTRL] &= ~(__XPEN | __DPRST);
}

/**
 * Enable VIP's interrupts
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 * @param interruptCode			Interrupts to enable
 */
void VIPManager_enableInterrupt(VIPManager this __attribute__ ((unused)), u16 interruptCode)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::enableInterrupt: null this");

	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
	_vipRegisters[__INTENB]= interruptCode | __TIMEERR;
}

/**
 * Disable VIP's interrupts
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 */
void VIPManager_disableInterrupts(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::disableInterrupt: null this");

	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}

/**
 * Enable / disable DRAM writing
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 * @param allowDRAMAccess		Flag's value
 */
void VIPManager_allowDRAMAccess(VIPManager this, bool allowDRAMAccess)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::allowDRAMAccess: null this");

	this->allowDRAMAccess = allowDRAMAccess;
}

/**
 * Check if rendering is pending
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 *
 * @return						True if XPEND already happened but DRAM writing didn't take place
 */
bool __attribute__ ((noinline)) VIPManager_isRenderingPending(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::isRenderingPending: null this");

	return this->drawingEnded && !this->renderingCompleted;
}

/**
 * VIP's interrupt handler
 *
 * @memberof		VIPManager
 * @public
 */
void VIPManager_interruptHandler(void)
{
	// save the interrupt event
	u16 interrupt = _vipRegisters[__INTPND];

	// disable interrupts
	VIPManager_disableInterrupts(_vipManager);

	// handle the interrupt
	VIPManager_processInterrupt(_vipManager, interrupt);

	// enable interrupts
	if(_vipManager->processingXPEND)
	{
		VIPManager_enableInterrupt(_vipManager, __FRAMESTART);
	}
	else
	{
		VIPManager_enableInterrupt(_vipManager, __FRAMESTART | __XPEND);
	}
}
/**
 * Process interrupt method
 *
 * @memberof		VIPManager
 * @public
 */
inline static void VIPManager_processInterrupt(VIPManager this, u16 interrupt)
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
				Game_saveProcessNameDuringFRAMESTART(Game_getInstance());
#endif

				VIPManager_registerCurrentDrawingFrameBufferSet(this);
				Game_currentFrameEnded(Game_getInstance());
				this->drawingEnded = false;
				this->renderingCompleted = false;
				break;

			case __XPEND:

				this->processingXPEND = true;
				this->renderingCompleted = false;

#ifdef __PROFILE_GAME
				Game_saveProcessNameDuringXPEND(Game_getInstance());
#endif

#ifdef __PROFILE_GAME
				{
					s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(_timerManager);
#endif

					// prevent VIP's drawing operations
//#ifndef __ALERT_VIP_OVERTIME
					VIPManager_disableDrawing(this);
//#endif
					// to allow timer interrupts
					HardwareManager_enableMultiplexedInterrupts();
					VIPManager_enableInterrupt(this, __FRAMESTART);

					if(this->allowDRAMAccess)
					{
						// write to DRAM
						SpriteManager_render(_spriteManager);
						this->renderingCompleted = true;
					}

					// write to the frame buffers
					VIPManager_processFrameBuffers(this);


					// allow VIP's drawing operations
					VIPManager_enableDrawing(this);

					// flag completions
					this->drawingEnded = true;

					HardwareManager_disableMultiplexedInterrupts();

#ifdef __DEBUG_TOOLS
					if(Game_isInDebugMode(Game_getInstance()))
					{
						Debug_render(Debug_getInstance());
					}
#endif

#ifdef __PROFILE_GAME
					extern s16 _renderingProcessTimeHelper;
					extern s16 _renderingProcessTime;
					extern s16 _renderingHighestTime;
					extern s16 _renderingTotalTime;
					extern bool _updateProfiling;

					if(_updateProfiling)
					{
						_renderingProcessTimeHelper = _renderingProcessTime = TimerManager_getMillisecondsElapsed(_timerManager) - timeBeforeProcess;
						_renderingHighestTime = _renderingProcessTime > _renderingHighestTime ? _renderingProcessTime : _renderingHighestTime;
						_renderingTotalTime += _renderingProcessTime;
					}
				}
#endif

				this->processingXPEND = false;
				break;

			case __TIMEERR:

				Game_increaseGameFrameDuration(Game_getInstance(), __GAME_FRAME_DURATION);

#ifdef __ALERT_VIP_OVERTIME
				{
					static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS / 2;

					if(0 >= messageDelay)
					{
						messageDelay = __TARGET_FPS;
						static u32 count = 0;
						Printing_text(Printing_getInstance(), "VIP Overtime! (   )", 0, 26, NULL);
						Printing_int(Printing_getInstance(), ++count, 15, 26, NULL);
					}

					if(0 == --messageDelay)
					{
						Printing_text(Printing_getInstance(), "                   ", 0, 26, NULL);
						messageDelay = -1;
					}
				}
#endif

				break;
		}
	}
}

/**
 * Start frame buffer writing operations
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			The time in milliseconds that it took to process the interrupt (only if profiling is enabled)
 */
static void VIPManager_processFrameBuffers(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::processFrameBuffers: null this");

#ifdef __REGISTER_LAST_PROCESS_NAME
	Game_setLastProcessName(Game_getInstance(), "rendering");
#endif

	// draw 3d objects
	PolyhedronManager_drawPolyhedrons(_polyhedronManager);

	// check if the current frame buffer set is valid
	VirtualNode node = this->postProcessingEffects->tail;

	for(; node; node = node->previous)
	{
		((PostProcessingEffectRegistry*)node->data)->postProcessingEffect(this->currentDrawingFrameBufferSet, ((PostProcessingEffectRegistry*)node->data)->spatialObject);
	}
}

/**
 * Turn on the displays
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 */
void VIPManager_displayOn(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::displayOn: null this");

	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE | __DISP);
	_vipRegisters[__FRMCYC] = __FRAME_CYCLE;
}

/**
 * Turn off the displays
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 */
void VIPManager_displayOff(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::displayOff: null this");

	_vipRegisters[__REST] = 0;
	_vipRegisters[__XPCTRL] = 0;
	_vipRegisters[__DPCTRL] = 0;
	_vipRegisters[__FRMCYC] = 1;

	VIPManager_disableInterrupts(this);
}

/**
 * Setup the palettes
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 * @param paletteConfig			Configuration of the palettes
 */
void VIPManager_setupPalettes(VIPManager this __attribute__ ((unused)), PaletteConfig* paletteConfig)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::setupPalettes: null this");

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
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 */
void VIPManager_upBrightness(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::upBrightness: null this");

	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}

/**
 * Turn brightness all the way down
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 */
void VIPManager_lowerBrightness(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::displayHide: null this");

	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;

	VIPManager_setBackgroundColor(this, __COLOR_BLACK);
}

/**
 * Clear the CHAR and Param table memory
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 */
void VIPManager_clearScreen(VIPManager this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::clearScreen: null this");
	u8* bgmapStartAddress = (u8*)__BGMAP_SPACE_BASE_ADDRESS;

	// clear every bgmap segment
	for(bgmapStartAddress = 0; bgmapStartAddress < (u8*)__PARAM_TABLE_END; bgmapStartAddress++)
	{
		*bgmapStartAddress = 0;
	}

	// clear every char segment
	Mem_clear ((BYTE*) __CHAR_SEGMENT_0_BASE_ADDRESS, 8192);
	Mem_clear ((BYTE*) __CHAR_SEGMENT_1_BASE_ADDRESS, 8192);
	Mem_clear ((BYTE*) __CHAR_SEGMENT_2_BASE_ADDRESS, 8192);
	Mem_clear ((BYTE*) __CHAR_SEGMENT_3_BASE_ADDRESS, 8192);
}

/**
 * Clear a BGMAP segment
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 * @param segment	The segment to clean up
 * @param size		Segment's size
 */
void VIPManager_clearBgmapSegment(VIPManager this __attribute__ ((unused)), int segment, int size)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::clearBgmap: null this");

	Mem_clear((BYTE*)__BGMAP_SEGMENT(segment), size * 2);
}

/**
 * Setup the column table
 *
 * @memberof						VIPManager
 * @public
 *
 * @param this						Function scope
 * @param columnTableDefinition		Definition to use
 */
void VIPManager_setupColumnTable(VIPManager this __attribute__ ((unused)), ColumnTableDefinition* columnTableDefinition)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::setupColumnTable: null this");

	int i, value;

	// use the default column table as fallback
	if(columnTableDefinition == NULL)
	{
		columnTableDefinition = (ColumnTableDefinition*)&DEFAULT_COLUMN_TABLE;
	}

	// write column table
	for(i = 0; i < 256; i++)
	{
		value = (columnTableDefinition->mirror && (i > 127))
			? columnTableDefinition->columnTable[255 - i]
			: columnTableDefinition->columnTable[i];

		_columnTableBaseAddressLeft[i] = value;
		_columnTableBaseAddressRight[i] = value;
	}
}

/**
 * Use the vip's built-in column table instead of reading the one defined in memory
 *
 * @memberof				VIPManager
 * @public
 *
 * @param this				Function scope
 * @param useInternal		Flag
 */
void VIPManager_useInternalColumnTable(VIPManager this __attribute__ ((unused)), bool useInternal)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::useInternalColumnTable: null this");

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
 * @memberof								VIPManager
 * @public
 *
 * @param this								Function scope
 * @param brightnessRepeatDefinition		Definition
 */
void VIPManager_setupBrightnessRepeat(VIPManager this __attribute__ ((unused)), BrightnessRepeatDefinition* brightnessRepeatDefinition)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::setupBrightnessRepeat: null this");

	int i, leftCta, rightCta, value;

	// use the default repeat values as fallback
	if(brightnessRepeatDefinition == NULL)
	{
		brightnessRepeatDefinition = (BrightnessRepeatDefinition*)&DEFAULT_BRIGHTNESS_REPEAT;
	}

	// column table offsets
	leftCta = _vipRegisters[__CTA] & 0xFF;
	rightCta = _vipRegisters[__CTA] >> 8;

	// write repeat values to column table
	for(i = 0; i < 96; i++)
	{
		value = (brightnessRepeatDefinition->mirror && (i > 47))
			? brightnessRepeatDefinition->brightnessRepeat[95 - i] << 8
			: brightnessRepeatDefinition->brightnessRepeat[i] << 8;

		_columnTableBaseAddressLeft[leftCta - i] = (_columnTableBaseAddressLeft[leftCta - i] & 0xff) | value;
		_columnTableBaseAddressRight[rightCta - i] = (_columnTableBaseAddressRight[rightCta - i] & 0xff) | value;
	}
}

/**
 * Set background color
 *
 * @memberof			VIPManager
 * @public
 *
 * @param this			Function scope
 * @param color			New color
 */
void VIPManager_setBackgroundColor(VIPManager this __attribute__ ((unused)), u8 color)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::setBackgroundColor: null this");

	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}

/**
 * Check if a post-processing effect is already registered
 *
 * @memberof							VIPManager
 * @public
 *
 * @param this							Function scope
 * @param postProcessingEffect			Post-processing effect function
 * @param spatialObject					Post-processing effect function's scope
 *
 * @return								Whether the effect and object are already registered
 */
static bool VIPManager_isPostProcessingEffectRegistered(VIPManager this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::addPostProcessingEffect: null this");

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
 * @memberof							VIPManager
 * @public
 *
 * @param this							Function scope
 * @param postProcessingEffect			Post-processing effect function
 * @param spatialObject					Post-processing effect function's scope
 */
void VIPManager_pushFrontPostProcessingEffect(VIPManager this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::pushFrontPostProcessingEffect: null this");

	if(VIPManager_isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject))
	{
//		return;
	}

	PostProcessingEffectRegistry* postProcessingEffectRegistry = __NEW_BASIC(PostProcessingEffectRegistry);
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;

	VirtualList_pushFront(this->postProcessingEffects, postProcessingEffectRegistry);
}

/**
 * Register a post-processing effect with lower priority
 *
 * @memberof							VIPManager
 * @public
 *
 * @param this							Function scope
 * @param postProcessingEffect			Post-processing effect function
 * @param spatialObject					Post-processing effect function's scope
 */
void VIPManager_pushBackPostProcessingEffect(VIPManager this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::pushBackPostProcessingEffect: null this");

	if(VIPManager_isPostProcessingEffectRegistered(this, postProcessingEffect, spatialObject))
	{
		return;
	}

	PostProcessingEffectRegistry* postProcessingEffectRegistry = __NEW_BASIC(PostProcessingEffectRegistry);
	postProcessingEffectRegistry->postProcessingEffect = postProcessingEffect;
	postProcessingEffectRegistry->spatialObject = spatialObject;

	VirtualList_pushBack(this->postProcessingEffects, postProcessingEffectRegistry);
}

/**
 * Remove a post-processing effect
 *
 * @memberof							VIPManager
 * @public
 *
 * @param this							Function scope
 * @param postProcessingEffect			Post-processing effect function
 * @param spatialObject					Post-processing effect function's scope
 */
void VIPManager_removePostProcessingEffect(VIPManager this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::removePostProcessingEffect: null this");

	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			VirtualList_removeElement(this->postProcessingEffects, postProcessingEffectRegistry);

			__DELETE_BASIC(postProcessingEffectRegistry);
			return;
		}
	}
}

/**
 * Remove all a post-processing effects
 *
 * @memberof			VIPManager
 * @public
 *
 * @param this			Function scope
 */
void VIPManager_removePostProcessingEffects(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::removePostProcessingEffects: null this");

	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->data);
	}

	VirtualList_clear(this->postProcessingEffects);
}

/**
 * Register the frame buffer in use by the VIP's drawing process
 *
 * @memberof			VIPManager
 * @public
 *
 * @param this			Function scope
 */
void VIPManager_registerCurrentDrawingFrameBufferSet(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::registerCurrentDrawingframeBufferSet: null this");

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
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Frame buffer in use by the VIP's drawing process
 */
u32 VIPManager_getCurrentDrawingframeBufferSet(VIPManager this)
{
	ASSERT(__SAFE_CAST(VIPManager, this), "VIPManager::getCurrentDrawingframeBufferSet: null this");

	return this->currentDrawingFrameBufferSet;
}
