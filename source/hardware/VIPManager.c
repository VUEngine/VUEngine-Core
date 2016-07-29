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

volatile u16* VIP_REGS = (u16*)0x0005F800;


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern ColumnTableROMDef DEFAULT_COLUMN_TABLE;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define VIPManager_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES;																				\
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
#ifdef __PRINT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU_WARNING
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

	while(VIP_REGS[XPSTTS] & XPBSYR);
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VIPManager_enableInterrupt(this);
}

void VIPManager_disableDrawing(VIPManager this)
{
	NM_ASSERT(false, "VIPManager::idleDrawing: null this");

	VIPManager_disableInterrupt(this);

	while(VIP_REGS[XPSTTS] & XPBSYR);
	VIP_REGS[XPCTRL] |= XPRST;
	VIP_REGS[XPCTRL] &= ~XPEN;
}

// enable interrupt
void VIPManager_enableInterrupt(VIPManager this)
{
	ASSERT(this, "VIPManager::enableInterrupt: null this");

	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
#ifdef __ALERT_VPU_OVERTIME
	VIP_REGS[INTENB]= XPEND | TIMEERR;
#else
	VIP_REGS[INTENB]= XPEND;
#endif
}

// disable interrupt
void VIPManager_disableInterrupt(VIPManager this)
{
	ASSERT(this, "VIPManager::disableInterrupt: null this");

	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
}

void VIPManager_interruptHandler(void)
{
	bool idle = VIP_REGS[INTPND] & XPEND;
#ifdef __ALERT_VPU_OVERTIME
	bool overtime = VIP_REGS[INTPND] & TIMEERR;
#endif

	// disable interrupts
	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];

#ifdef __ALERT_VPU_OVERTIME
    {
        static int messageDelay = __TARGET_FPS;

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
		VIP_REGS[XPCTRL] |= XPRST;
		VIP_REGS[XPCTRL] &= ~XPEN;

		while(VIP_REGS[XPSTTS] & XPBSYR);

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
                ((void (*)(u32))node->data)(_vipManager->currentDrawingframeBufferSet);
            }
        }

		// enable drawing
		while (VIP_REGS[XPSTTS] & XPBSYR);
		VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	}

#ifndef	__FORCE_VPU_SYNC
#ifdef __PRINT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU_WARNING
    {
#ifdef __ALERT_VPU_OVERTIME
        int y = 2;
#else
        int y = 1;
#endif
        static int messageDelay = __TARGET_FPS;
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
    VIP_REGS[INTCLR] = VIP_REGS[INTPND];
#ifdef __ALERT_VPU_OVERTIME
	VIP_REGS[INTENB]= XPEND | TIMEERR;
#else
	VIP_REGS[INTENB]= XPEND;
#endif
}

// turn display on
void VIPManager_displayOn(VIPManager this)
{
	ASSERT(this, "VIPManager::displayOn: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VIP_REGS[DPCTRL] = VIP_REGS[DPSTTS] | (SYNCE | RE | DISP);
	VIP_REGS[FRMCYC] = 0;
}

// turn display off
void VIPManager_displayOff(VIPManager this)
{
	ASSERT(this, "VIPManager::displayOff: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = 0;
	VIP_REGS[DPCTRL] = 0;
	VIP_REGS[FRMCYC] = 1;

	VIPManager_disableInterrupt(this);
}

// setup backgorund color
void VIPManager_setupPalettes(VIPManager this, PaletteConfig* paletteConfig)
{
	ASSERT(this, "VIPManager::setupPalettes: null this");

	VIP_REGS[GPLT0] = paletteConfig->bgmap.gplt0;
	VIP_REGS[GPLT1] = paletteConfig->bgmap.gplt1;
	VIP_REGS[GPLT2] = paletteConfig->bgmap.gplt2;
	VIP_REGS[GPLT3] = paletteConfig->bgmap.gplt3;

	VIP_REGS[JPLT0] = paletteConfig->object.jplt0;
	VIP_REGS[JPLT1] = paletteConfig->object.jplt1;
	VIP_REGS[JPLT2] = paletteConfig->object.jplt2;
	VIP_REGS[JPLT3] = paletteConfig->object.jplt3;

	VIP_REGS[BKCOL] = paletteConfig->backgroundColor <= __COLOR_BRIGHT_RED? paletteConfig->backgroundColor: __COLOR_BRIGHT_RED;
}

// set brightness all the way up
void VIPManager_upBrightness(VIPManager this)
{
	ASSERT(this, "VIPManager::upBrightness: null this");

	VIP_REGS[BRTA] = 32;
	VIP_REGS[BRTB] = 64;
	VIP_REGS[BRTC] = 32;
}

// set brightness all way down
void VIPManager_lowerBrightness(VIPManager this)
{
	ASSERT(this, "VIPManager::displayHide: null this");

	VIP_REGS[BRTA] = 0;
	VIP_REGS[BRTB] = 0;
	VIP_REGS[BRTC] = 0;

	VIPManager_setBackgroundColor(this, __COLOR_BLACK);
}


// clear screen
void VIPManager_clearScreen(VIPManager this)
{
	ASSERT(this, "VIPManager::clearScreen: null this");

	int i;
	//clear every bgmap segment
    for(i = 0; i < 14; i++)
	{
		Mem_clear((u16*)BGMap(i), 8192);
    }

	//clear every char segment
	Mem_clear ((u16*) CharSeg0, 8192);
	Mem_clear ((u16*) CharSeg1, 8192);
	Mem_clear ((u16*) CharSeg2, 8192);
	Mem_clear ((u16*) CharSeg3, 8192);
}

// clear bgmap
void VIPManager_clearBgmap(VIPManager this, int bgmap, int size)
{
	ASSERT(this, "VIPManager::clearBgmap: null this");

	Mem_clear((u16*)BGMap(bgmap), size);
}

// setup default column table
void VIPManager_setupColumnTable(VIPManager this, ColumnTableDefinition* columnTableDefinition)
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
        CLMN_TBL[i] = columnTableDefinition->columnTable[i];
        // right screen
        CLMN_TBL[i + 0x0100] = columnTableDefinition->columnTable[i];
    }

    // write column table (second half)
    if(columnTableDefinition->mirror)
    {
        for(i = 0; i < 128; i++)
        {
            // left screen
            CLMN_TBL[i + 0x0080] = columnTableDefinition->columnTable[127 - i];
            // right screen
            CLMN_TBL[i + 0x0180] = columnTableDefinition->columnTable[127 - i];
        }
    }
    else
    {
        for(i = 0; i < 128; i++)
        {
            // left screen
            CLMN_TBL[i + 0x0080] = columnTableDefinition->columnTable[127 + i];
            // right screen
            CLMN_TBL[i + 0x0180] = columnTableDefinition->columnTable[127 + i];
        }
    }
}

// use the vip's built-in column table instead of reading the one defined in memory
void VIPManager_useInternalColumnTable(VIPManager this, bool useInternal)
{
	ASSERT(this, "VIPManager::useInternalColumnTable: null this");

    // TODO: why does this not work?
    if(useInternal)
    {
        // set lock bit
        VIP_REGS[DPCTRL] |= LOCK;
    }
    else
    {
        // unset lock bit
        VIP_REGS[DPCTRL] &= LOCK;
    }
}

// set background color
void VIPManager_setBackgroundColor(VIPManager this, u8 color)
{
	ASSERT(this, "VIPManager::setBackgroundColor: null this");

    if(color > __COLOR_BRIGHT_RED)
    {
        color = __COLOR_BRIGHT_RED;
    }

	VIP_REGS[BKCOL] = color;
}

// register post processing effect
void VIPManager_addPostProcessingEffect(VIPManager this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "VIPManager::addPostProcessingEffect: null this");

	VirtualList_pushBack(this->postProcessingEffects, postProcessingEffect);
}

// remove post processing effect
void VIPManager_removePostProcessingEffect(VIPManager this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "VIPManager::removePostProcessingEffect: null this");

	VirtualList_removeElement(this->postProcessingEffects, postProcessingEffect);
}
// register post processing effect
void VIPManager_removePostProcessingEffects(VIPManager this)
{
	ASSERT(this, "VIPManager::removePostProcessingEffects: null this");

	VirtualList_clear(this->postProcessingEffects);
}

// register the frame buffer in use by the VPU's drawing process
void VIPManager_registerCurrentDrawingframeBufferSet(VIPManager this)
{
	ASSERT(this, "VIPManager::registerCurrentDrawingframeBufferSet: null this");

    u32 currentDrawingframeBufferSet = VIP_REGS[XPSTTS] & 0x000C;

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
