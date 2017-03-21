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
#include <Game.h>
#include <FrameRate.h>
#include <ParamTableManager.h>
#include <CharSetManager.h>
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
		u16 gameFrameStarted;																			\
		u16 drawingEnded;																			\

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

#ifdef __PROFILE_GAME
void Game_saveProcessDuringDRAMWritingDoneName(Game this);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
bool Game_isGameFrameDone(Game this);
#endif

static VIPManager _vipManager;
static ParamTableManager _paramTableManager;
static CharSetManager _charSetManager;
static PolyhedronManager _polyhedronManager;
static SpriteManager _spriteManager;

static void VIPManager_constructor(VIPManager this);


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
	ASSERT(this, "VIPManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->postProcessingEffects = __NEW(VirtualList);
	this->currentDrawingFrameBufferSet = 0;
	this->gameFrameStarted = false;
	this->drawingEnded = false;

	_vipManager = this;
	_paramTableManager = ParamTableManager_getInstance();
	_charSetManager = CharSetManager_getInstance();
	_spriteManager = SpriteManager_getInstance();
	_polyhedronManager = PolyhedronManager_getInstance();

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
	ASSERT(this, "VIPManager::destructor: null this");

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
	ASSERT(this, "VIPManager::enableDrawing: null this");

	while(_vipRegisters[__XPSTTS] & __XPBSYR);
	_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | __XPEN;
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
	ASSERT(this, "VIPManager::disableDrawing: null this");

	_vipRegisters[__XPCTRL] |= __XPRST;
	_vipRegisters[__XPCTRL] &= ~__XPEN;
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
	ASSERT(this, "VIPManager::enableInterrupt: null this");

	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
#ifdef __ALERT_VIP_OVERTIME
	_vipRegisters[__INTENB]= interruptCode | __TIMEERR;
#else
	_vipRegisters[__INTENB]= interruptCode;
#endif
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
	ASSERT(this, "VIPManager::disableInterrupt: null this");

	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}

u32 __attribute__ ((noinline)) VIPManager_waitForFrameStart(VIPManager this)
{
	ASSERT(this, "VIPManager::waitForGameFrame: null this");

	return !this->gameFrameStarted;
}

u32 __attribute__ ((noinline)) VIPManager_drawingEnded(VIPManager this)
{
	ASSERT(this, "VIPManager::waitForGameFrame: null this");

	return this->drawingEnded;
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

	const u16 interruptTable[] =
	{
		__GAMESTART,
		__XPEND,
#ifdef __ALERT_VIP_OVERTIME
		__TIMEERR
#endif
	};

	int i = 0;
	const int limit = sizeof(interruptTable) / sizeof(u16);

	for(; i < limit; i++)
	{
		switch(interrupt & interruptTable[i])
		{
			case __GAMESTART:

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
				{
					static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS;

					if(/*!Game_isGameFrameDone(Game_getInstance()) &&*/ 0 >= messageDelay)
					{
						messageDelay = __TARGET_FPS;
						Printing_text(Printing_getInstance(), "VIP GCLK:                  ", 0, 0, NULL);
						Printing_text(Printing_getInstance(), Game_getLastProcessName(Game_getInstance()), 11, 0, NULL);

					}

					if(0 == --messageDelay)
					{
						Printing_text(Printing_getInstance(), "                                                ", 0, 0, NULL);
						messageDelay = -1;
					}
				}
#endif
				_vipManager->gameFrameStarted = true;
				VIPManager_registerCurrentDrawingFrameBufferSet(_vipManager);
				VIPManager_enableInterrupt(_vipManager, __XPEND);
				break;

			case __XPEND:

#ifdef __PROFILE_GAME
				Game_saveProcessDuringDRAMWritingDoneName(Game_getInstance());
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
				{
					static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS;

					if(!Game_isGameFrameDone(Game_getInstance()) && 0 >= messageDelay)
					{
						messageDelay = __TARGET_FPS;
						Printing_text(Printing_getInstance(), "VIP XPEND:                  ", 0, 1, NULL);
						Printing_text(Printing_getInstance(), Game_getLastProcessName(Game_getInstance()), 11, 1, NULL);
					}

					if(0 == --messageDelay)
					{
						Printing_text(Printing_getInstance(), "                                                ", 0, 1, NULL);
						messageDelay = -1;
					}
				}
#endif

				_vipManager->drawingEnded = true;
				VIPManager_disableDrawing(_vipManager);
				VIPManager_enableInterrupt(_vipManager, __GAMESTART);
				break;

#ifdef __ALERT_VIP_OVERTIME
			case __TIMEERR:

				{
					static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS;

					if(0 >= messageDelay)
					{
						messageDelay = __TARGET_FPS;
						static u32 count = 0;
						Printing_text(Printing_getInstance(), "VIP Overtime! (   )", 0, 2, NULL);
						Printing_int(Printing_getInstance(), ++count, 15, 2, NULL);
					}

					if(0 == --messageDelay)
					{
						Printing_text(Printing_getInstance(), "                   ", 0, 2, NULL);
						messageDelay = -1;
					}
				}

				VIPManager_enableInterrupt(_vipManager, __GAMESTART);
				break;
#endif
		}
	}
}

/**
 * Reset flag that indicates if the GAME_FRAME_STARTED interrupt has been triggered
 *
 * @memberof					VIPManager
 * @public
 *
 * @param this					Function scope
 */
void VIPManager_resetGameFrameStarted(VIPManager this)
{
	ASSERT(this, "VIPManager::resetGameFrameStarted: null this");

	this->gameFrameStarted = false;
}

/**
 * Start DRAM writing operations
 *
 * @memberof		VIPManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			The time in milliseconds that it took to process the interrupt (only if profiling is enabled)
 */
void VIPManager_writeDRAM(VIPManager this)
{
	ASSERT(this, "VIPManager::writeDRAM: null this");

	// write newly created chars
	if(!CharSetManager_writeCharSetsProgressively(CharSetManager_getInstance()))
	{
		ParamTableManager_defragmentProgressively(ParamTableManager_getInstance());
	}

	// write to DRAM
	SpriteManager_render(_spriteManager);

#ifdef __DEBUG_TOOLS
	if(Game_isInDebugMode(Game_getInstance()))
	{
		Debug_render(Debug_getInstance());
	}
#endif

	// draw 3d objects
	PolyhedronManager_drawPolyhedrons(_polyhedronManager);

	// check if the current frame buffer set is valid
	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		((PostProcessingEffectRegistry*)node->data)->postProcessingEffect(this->currentDrawingFrameBufferSet, ((PostProcessingEffectRegistry*)node->data)->spatialObject);
	}

	this->drawingEnded = false;

	// enable drawing
	VIPManager_enableInterrupt(_vipManager, __GAMESTART);
	VIPManager_enableDrawing(this);
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
	ASSERT(this, "VIPManager::displayOn: null this");

	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE | __DISP);
	_vipRegisters[__FRMCYC] = 0;

	VIPManager_enableDrawing(this);
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
	ASSERT(this, "VIPManager::displayOff: null this");

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
	ASSERT(this, "VIPManager::setupPalettes: null this");

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
	ASSERT(this, "VIPManager::upBrightness: null this");

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
	ASSERT(this, "VIPManager::displayHide: null this");

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
	ASSERT(this, "VIPManager::clearScreen: null this");
	u16* bgmapStartAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	// clear every bgmap segment
	for(bgmapStartAddress = 0; bgmapStartAddress < (u16*)__PARAM_TABLE_END; bgmapStartAddress++)
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
	ASSERT(this, "VIPManager::clearBgmap: null this");

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
	ASSERT(this, "VIPManager::setupColumnTable: null this");

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
	ASSERT(this, "VIPManager::useInternalColumnTable: null this");

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
	ASSERT(this, "VIPManager::setupBrightnessRepeat: null this");

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
	ASSERT(this, "VIPManager::setBackgroundColor: null this");

	_vipRegisters[__BACKGROUND_COLOR] = (color <= __COLOR_BRIGHT_RED)
		? color
		: __COLOR_BRIGHT_RED;
}

/**
 * Register a post-processing effect
 *
 * @memberof							VIPManager
 * @public
 *
 * @param this							Function scope
 * @param postProcessingEffect			Post-processing effect function
 * @param spatialObject					Post-processing effect function's scope
 */
void VIPManager_addPostProcessingEffect(VIPManager this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(this, "VIPManager::addPostProcessingEffect: null this");

	VirtualNode node = this->postProcessingEffects->head;

	for(; node; node = node->next)
	{
		PostProcessingEffectRegistry* postProcessingEffectRegistry = (PostProcessingEffectRegistry*)node->data;

		if(postProcessingEffectRegistry->postProcessingEffect == postProcessingEffect && postProcessingEffectRegistry->spatialObject == spatialObject)
		{
			return;
		}
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
	ASSERT(this, "VIPManager::removePostProcessingEffect: null this");

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
	ASSERT(this, "VIPManager::removePostProcessingEffects: null this");

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
	ASSERT(this, "VIPManager::registerCurrentDrawingframeBufferSet: null this");

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
	ASSERT(this, "VIPManager::getCurrentDrawingframeBufferSet: null this");

	return this->currentDrawingFrameBufferSet;
}
