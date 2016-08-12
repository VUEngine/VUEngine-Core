/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VIPManager.h>
#include <HardwareManager.h>
#include <Game.h>
#include <FrameRate.h>
#include <ParamTableManager.h>
#include <CharSetManager.h>
#include <SpriteManager.h>
#include <Mem.h>
#include <Printing.h>
#include <debugConfig.h>

//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

volatile u16* _vipRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (u16*)0x0005F800;


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern ColumnTableROMDef DEFAULT_COLUMN_TABLE;


typedef struct PostProcessingEffect
{
    void (*function) (u32);

} PostProcessingEffect;

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define VIPManager_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* dram managers */																				\
        /* post processing effects */																	\
        VirtualList postProcessingEffects;																\
        u32 currentDrawingframeBufferSet;																\

// define the VIPManager
__CLASS_DEFINITION(VIPManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

#ifndef	__FORCE_VPU_SYNC
#ifdef __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU
bool Game_doneDRAMPrecalculations(Game this);
const char* Game_getDRAMPrecalculationsStep(Game this);
#endif
#endif

static VIPManager _vipManager;
static ParamTableManager _paramTableManager;
static CharSetManager _charSetManager;
static SpriteManager _spriteManager;

static void VIPManager_constructor(VIPManager this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(VIPManager);

// class's constructor
static void __attribute__ ((noinline)) VIPManager_constructor(VIPManager this)
{
	ASSERT(this, "VIPManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

    this->postProcessingEffects = __NEW(VirtualList);
    this->currentDrawingframeBufferSet = 0;

    _vipManager = this;
	_paramTableManager = ParamTableManager_getInstance();
	_charSetManager = CharSetManager_getInstance();
	_spriteManager = SpriteManager_getInstance();

}

// class's destructor
void VIPManager_destructor(VIPManager this)
{
	ASSERT(this, "VIPManager::destructor: null this");

    __DELETE(this->postProcessingEffects);

	// allow a new construct
	__SINGLETON_DESTROY;
}

void VIPManager_enableDrawing(VIPManager this)
{
	ASSERT(this, "VIPManager::enableDrawing: null this");

	while(_vipRegisters[__XPSTTS] & XPBSYR);
	_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | XPEN;
	VIPManager_enableInterrupt(this);
}

void VIPManager_disableDrawing(VIPManager this)
{
	NM_ASSERT(false, "VIPManager::idleDrawing: null this");

	VIPManager_disableInterrupt(this);

	while(_vipRegisters[__XPSTTS] & XPBSYR);
	_vipRegisters[__XPCTRL] |= XPRST;
	_vipRegisters[__XPCTRL] &= ~XPEN;
}

// enable interrupt
void VIPManager_enableInterrupt(VIPManager this __attribute__ ((unused)))
{
	ASSERT(this, "VIPManager::enableInterrupt: null this");

	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
#ifdef __ALERT_VPU_OVERTIME
	_vipRegisters[__INTENB]= XPEND | TIMEERR;
#else
	_vipRegisters[__INTENB]= XPEND;
#endif
}

// disable interrupt
void VIPManager_disableInterrupt(VIPManager this __attribute__ ((unused)))
{
	ASSERT(this, "VIPManager::disableInterrupt: null this");

	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
}

void VIPManager_interruptHandler(void)
{
	bool idle = _vipRegisters[__INTPND] & XPEND;
#ifdef __ALERT_VPU_OVERTIME
	bool overtime = _vipRegisters[__INTPND] & TIMEERR;
#endif

	// disable interrupts
	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];

#ifdef __ALERT_VPU_OVERTIME
    {
        static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS;

        if(overtime)
        {
            Printing_text(Printing_getInstance(), "VPU Overtime!   ", 0, 1, NULL);
            messageDelay = __TARGET_FPS;

            // force normal rendering
            idle = true;
        }
        else if(0 == --messageDelay)
        {
            Printing_text(Printing_getInstance(), "                      ", 0, 1, NULL);
            messageDelay = -1;
        }
    }
#endif

#ifdef __ALERT_STACK_OVERFLOW
	HardwareManager_checkStackStatus(HardwareManager_getInstance());
#endif

	// if the VPU is idle
	if(idle)
	{
		// disable drawing
		_vipRegisters[__XPCTRL] |= XPRST;
		_vipRegisters[__XPCTRL] &= ~XPEN;

		while(_vipRegisters[__XPSTTS] & XPBSYR);

		// if performance was good enough in the
		// the previous second do some defragmenting
		if(!ParamTableManager_processRemovedSprites(_paramTableManager))
		{
			CharSetManager_defragmentProgressively(_charSetManager);
			// TODO: bgmap memory defragmentation
		}

		// write to DRAM
		SpriteManager_render(_spriteManager);

        // check if the current frame buffer set is valid
        if(0 == _vipManager->currentDrawingframeBufferSet || 0x8000 == _vipManager->currentDrawingframeBufferSet)
        {
            VirtualNode node = _vipManager->postProcessingEffects->head;

            for(; node; node = node->next)
            {
                ((PostProcessingEffect*)node->data)->function(_vipManager->currentDrawingframeBufferSet);
            }
        }

		// enable drawing
		while (_vipRegisters[__XPSTTS] & XPBSYR);
		_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | XPEN;
	}

#ifndef	__FORCE_VPU_SYNC
#ifdef __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU
    {
#ifdef __ALERT_VPU_OVERTIME
        int y = 2;
#else
        int y = 1;
#endif
        static int messageDelay __INITIALIZED_DATA_SECTION_ATTRIBUTE = __TARGET_FPS;
        if(!Game_doneDRAMPrecalculations(Game_getInstance()))
        {
            Printing_text(Printing_getInstance(), "                      ", 0, y, NULL);
            Printing_text(Printing_getInstance(), "                               ", 0, y + 1, NULL);
            Printing_text(Printing_getInstance(), "VPU: out of budget", 0, y, NULL);
            Printing_text(Printing_getInstance(), (char*)Game_getDRAMPrecalculationsStep(Game_getInstance()), 0, y + 1, NULL);
            messageDelay = __TARGET_FPS;
        }

        if(0 == --messageDelay)
        {
            Printing_text(Printing_getInstance(), "                      ", 0, y, NULL);
            Printing_text(Printing_getInstance(), "                               ", 0, y + 1, NULL);
            messageDelay = -1;
        }
    }
#endif
#endif

	// enable interrupt
    _vipRegisters[__INTCLR] = _vipRegisters[__INTPND];
#ifdef __ALERT_VPU_OVERTIME
	_vipRegisters[__INTENB]= XPEND | TIMEERR;
#else
	_vipRegisters[__INTENB]= XPEND;
#endif
}

// turn display on
void VIPManager_displayOn(VIPManager this __attribute__ ((unused)))
{
	ASSERT(this, "VIPManager::displayOn: null this");

	_vipRegisters[__REST] = 0;
	_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | XPEN;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (SYNCE | RE | DISP);
	_vipRegisters[__FRMCYC] = 0;
}

// turn display off
void VIPManager_displayOff(VIPManager this)
{
	ASSERT(this, "VIPManager::displayOff: null this");

	_vipRegisters[__REST] = 0;
	_vipRegisters[__XPCTRL] = 0;
	_vipRegisters[__DPCTRL] = 0;
	_vipRegisters[__FRMCYC] = 1;

	VIPManager_disableInterrupt(this);
}

// setup backgorund color
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

	_vipRegisters[__BACKGROUND_COLOR] = paletteConfig->backgroundColor <= __COLOR_BRIGHT_RED? paletteConfig->backgroundColor: __COLOR_BRIGHT_RED;
}

// set brightness all the way up
void VIPManager_upBrightness(VIPManager this __attribute__ ((unused)))
{
	ASSERT(this, "VIPManager::upBrightness: null this");

	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;
}

// set brightness all way down
void VIPManager_lowerBrightness(VIPManager this)
{
	ASSERT(this, "VIPManager::displayHide: null this");

	_vipRegisters[__BRTA] = 0;
	_vipRegisters[__BRTB] = 0;
	_vipRegisters[__BRTC] = 0;

	VIPManager_setBackgroundColor(this, __COLOR_BLACK);
}


// clear screen
void VIPManager_clearScreen(VIPManager this __attribute__ ((unused)))
{
	ASSERT(this, "VIPManager::clearScreen: null this");
	u16* bgmapStartAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	//clear every bgmap segment
    for(bgmapStartAddress = 0; bgmapStartAddress < (u16*)__PARAM_TABLE_END; bgmapStartAddress++)
	{
        *bgmapStartAddress = 0;
    }

	//clear every char segment
	Mem_clear ((u16*) __CHAR_SEGMENT_0_BASE_ADDRESS, 8192);
	Mem_clear ((u16*) __CHAR_SEGMENT_1_BASE_ADDRESS, 8192);
	Mem_clear ((u16*) __CHAR_SEGMENT_2_BASE_ADDRESS, 8192);
	Mem_clear ((u16*) __CHAR_SEGMENT_3_BASE_ADDRESS, 8192);
}

// clear bgmap
void VIPManager_clearBgmap(VIPManager this __attribute__ ((unused)), int bgmap, int size)
{
	ASSERT(this, "VIPManager::clearBgmap: null this");

	Mem_clear((u16*)__BGMAP_SEGMENT(bgmap), size);
}

// setup default column table
void VIPManager_setupColumnTable(VIPManager this __attribute__ ((unused)), ColumnTableDefinition* columnTableDefinition)
{
	ASSERT(this, "VIPManager::setupColumnTable: null this");

    u8 i;

    // use the default column table or the one defined in current stage as fallback
	if(columnTableDefinition == NULL)
	{
	    Stage stage = GameState_getStage(Game_getCurrentState(Game_getInstance()));
	    if(stage != NULL)
	    {
	        StageDefinition* stageDefinition = Stage_stageDefinition(stage);
	        if(stageDefinition->rendering.columnTableDefinition != NULL)
	        {
	            columnTableDefinition = stageDefinition->rendering.columnTableDefinition;
	        }
	    }

        if(columnTableDefinition == NULL)
        {
            columnTableDefinition = (ColumnTableDefinition*)&DEFAULT_COLUMN_TABLE;
        }
	}

    // write column table (first half)
    for(i = 0; i < 128; i++)
    {
        // left screen
        _columnTableBaseAddress[i] = columnTableDefinition->columnTable[i];
        // right screen
        _columnTableBaseAddress[i + 0x0100] = columnTableDefinition->columnTable[i];
    }

    // write column table (second half)
    if(columnTableDefinition->mirror)
    {
        for(i = 0; i < 128; i++)
        {
            // left screen
            _columnTableBaseAddress[i + 0x0080] = columnTableDefinition->columnTable[127 - i];
            // right screen
            _columnTableBaseAddress[i + 0x0180] = columnTableDefinition->columnTable[127 - i];
        }
    }
    else
    {
        for(i = 0; i < 128; i++)
        {
            // left screen
            _columnTableBaseAddress[i + 0x0080] = columnTableDefinition->columnTable[127 + i];
            // right screen
            _columnTableBaseAddress[i + 0x0180] = columnTableDefinition->columnTable[127 + i];
        }
    }
}

// use the vip's built-in column table instead of reading the one defined in memory
void VIPManager_useInternalColumnTable(VIPManager this __attribute__ ((unused)), bool useInternal)
{
	ASSERT(this, "VIPManager::useInternalColumnTable: null this");

    // TODO: why does this not work?
    if(useInternal)
    {
        // set lock bit
        _vipRegisters[__DPCTRL] |= LOCK;
    }
    else
    {
        // unset lock bit
        _vipRegisters[__DPCTRL] &= LOCK;
    }
}

// set background color
void VIPManager_setBackgroundColor(VIPManager this __attribute__ ((unused)), u8 color)
{
	ASSERT(this, "VIPManager::setBackgroundColor: null this");

    if(color > __COLOR_BRIGHT_RED)
    {
        color = __COLOR_BRIGHT_RED;
    }

	_vipRegisters[__BACKGROUND_COLOR] = color;
}

// register post processing effect
void VIPManager_addPostProcessingEffect(VIPManager this, void (*postProcessingEffectFunction) (u32))
{
	ASSERT(this, "VIPManager::addPostProcessingEffect: null this");

    VirtualNode node = this->postProcessingEffects->head;

    for(; node; node = node->next)
    {
        PostProcessingEffect* postProcessingEffect = (PostProcessingEffect*)node->data;

        if(postProcessingEffect->function == postProcessingEffectFunction)
        {
            break;
        }
    }

    if(!node)
    {
        PostProcessingEffect* postProcessingEffect = __NEW_BASIC(PostProcessingEffect);
        postProcessingEffect->function = postProcessingEffectFunction;

        VirtualList_pushBack(this->postProcessingEffects, postProcessingEffect);
    }
}

// remove post processing effect
void VIPManager_removePostProcessingEffect(VIPManager this, void (*postProcessingEffectFunction) (u32))
{
	ASSERT(this, "VIPManager::removePostProcessingEffect: null this");

    VirtualNode node = this->postProcessingEffects->head;

    for(; node; node = node->next)
    {
        PostProcessingEffect* postProcessingEffect = (PostProcessingEffect*)node->data;
        if(postProcessingEffect->function == postProcessingEffectFunction)
        {
            VirtualList_removeElement(this->postProcessingEffects, postProcessingEffect);

            __DELETE_BASIC(postProcessingEffect);
            break;
        }
    }
}
// register post processing effect
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

// register the frame buffer in use by the VPU's drawing process
void VIPManager_registerCurrentDrawingframeBufferSet(VIPManager this)
{
	ASSERT(this, "VIPManager::registerCurrentDrawingframeBufferSet: null this");

    u32 currentDrawingframeBufferSet = _vipRegisters[__XPSTTS] & 0x000C;

    this->currentDrawingframeBufferSet = 0xFFFF;

    if(0x0004 == currentDrawingframeBufferSet)
    {
        this->currentDrawingframeBufferSet = 0;
    }
    else if(0x0008 == currentDrawingframeBufferSet)
    {
        this->currentDrawingframeBufferSet = 0x8000;
    }
}
