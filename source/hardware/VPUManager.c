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

#include <VPUManager.h>
#include <HardwareManager.h>
#include <Game.h>
#include <FrameRate.h>
#include <ParamTableManager.h>
#include <CharSetManager.h>
#include <SpriteManager.h>

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

#define VPUManager_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* dram managers */																					\
	FrameRate frameRate;																				\
	ParamTableManager paramTableManager;																\
	CharSetManager charSetManager;																		\
	SpriteManager spriteManager;																		\
																										\
	/* post processing effects */																		\
	VirtualList postProcessingEffects;																	\
	u32 currentDrawingframeBufferSet;																	\

// define the VPUManager
__CLASS_DEFINITION(VPUManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

#ifndef	__FORCE_VPU_SYNC
bool Game_doneDRAMPrecalculations(Game this);
const char* Game_getDRAMPrecalculationsStep(Game this);
#endif

static void VPUManager_constructor(VPUManager this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(VPUManager);

// class's constructor
static void VPUManager_constructor(VPUManager this)
{
	ASSERT(this, "VPUManager::constructor: null this");

	__CONSTRUCT_BASE();

	this->frameRate = FrameRate_getInstance();
	this->paramTableManager = ParamTableManager_getInstance();
	this->charSetManager = CharSetManager_getInstance();
	this->spriteManager = SpriteManager_getInstance();

    this->postProcessingEffects = __NEW(VirtualList);
    this->currentDrawingframeBufferSet = 0;
}

// class's destructor
void VPUManager_destructor(VPUManager this)
{
	ASSERT(this, "VPUManager::destructor: null this");

    __DELETE(this->postProcessingEffects);

	// allow a new construct
	__SINGLETON_DESTROY;
}

void VPUManager_enableDrawing(VPUManager this)
{
	ASSERT(this, "VPUManager::enableDrawing: null this");

	while(VIP_REGS[XPSTTS] & XPBSYR);
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VPUManager_enableInterrupt(this);
}

void VPUManager_disableDrawing(VPUManager this)
{
	NM_ASSERT(false, "VPUManager::idleDrawing: null this");

	VPUManager_disableInterrupt(this);

	while(VIP_REGS[XPSTTS] & XPBSYR);
	VIP_REGS[XPCTRL] |= XPRST;
	VIP_REGS[XPCTRL] &= ~XPEN;
}

// enable interrupt
void VPUManager_enableInterrupt(VPUManager this)
{
	ASSERT(this, "VPUManager::enableInterrupt: null this");

	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
//	VIP_REGS[INTENB]= XPEND | TIMEERR;
	VIP_REGS[INTENB]= XPEND;
}

// disable interrupt
void VPUManager_disableInterrupt(VPUManager this)
{
	ASSERT(this, "VPUManager::disableInterrupt: null this");

	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
}

void VPUManager_interruptHandler(void)
{
	bool idle = VIP_REGS[INTPND] & XPEND;

	// disable interrupts
	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];

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

		VPUManager this = VPUManager_getInstance();

		// if performance was good enough in the
		// the previous second do some defragmenting
		if(!ParamTableManager_processRemovedSprites(this->paramTableManager))
		{
			CharSetManager_defragmentProgressively(this->charSetManager);
			// TODO: bgmap memory defragmentation
		}

		// write to DRAM
		SpriteManager_render(this->spriteManager);

        // check if the current frame buffer set is valid
        if(0 == this->currentDrawingframeBufferSet || 0x8000 == this->currentDrawingframeBufferSet)
        {
            VirtualNode node = this->postProcessingEffects->head;

            for(; node; node = node->next)
            {
                ((void (*)(u32))node->data)(this->currentDrawingframeBufferSet);
            }
        }

		// enable drawing
		while(VIP_REGS[XPSTTS] & XPBSYR);
		VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	}

#ifndef	__FORCE_VPU_SYNC
#ifdef __PRINT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU_WARNING
	static int messageDelay = __TARGET_FPS;
	if(!Game_doneDRAMPrecalculations(Game_getInstance()))
	{
		Printing_text(Printing_getInstance(), "                      ", 0, 1, NULL);
		Printing_text(Printing_getInstance(), "                               ", 0, 2, NULL);
		Printing_text(Printing_getInstance(), "VPU: out of budget", 0, 1, NULL);
		Printing_text(Printing_getInstance(), (char*)Game_getDRAMPrecalculationsStep(Game_getInstance()), 0, 2, NULL);
		messageDelay = __TARGET_FPS;
	}

	if(0 == --messageDelay )
	{
		Printing_text(Printing_getInstance(), "                      ", 0, 1, NULL);
		Printing_text(Printing_getInstance(), "                               ", 0, 2, NULL);
		messageDelay = -1;
	}
#endif
#endif

	// enable interrupt
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	VIP_REGS[INTENB]= XPEND;
}

// turn display on
void VPUManager_displayOn(VPUManager this)
{
	ASSERT(this, "VPUManager::displayOn: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VIP_REGS[DPCTRL] = VIP_REGS[DPSTTS] | (SYNCE | RE | DISP);
	VIP_REGS[FRMCYC] = 0;
}

// turn display off
void VPUManager_displayOff(VPUManager this)
{
	ASSERT(this, "VPUManager::displayOff: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = 0;
	VIP_REGS[DPCTRL] = 0;
	VIP_REGS[FRMCYC] = 1;

	VPUManager_disableInterrupt(this);
}

// setup backgorund color
void VPUManager_setupPalettes(VPUManager this, PaletteConfig* paletteConfig)
{
	ASSERT(this, "VPUManager::setupPalettes: null this");

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
void VPUManager_upBrightness(VPUManager this)
{
	ASSERT(this, "VPUManager::upBrightness: null this");

	VIP_REGS[BRTA] = 32;
	VIP_REGS[BRTB] = 64;
	VIP_REGS[BRTC] = 32;
}

// set brightness all way down
void VPUManager_lowerBrightness(VPUManager this)
{
	ASSERT(this, "VPUManager::displayHide: null this");

	VIP_REGS[BRTA] = 0;
	VIP_REGS[BRTB] = 0;
	VIP_REGS[BRTC] = 0;

	VPUManager_setBackgroundColor(this, __COLOR_BLACK);
}


// clear screen
void VPUManager_clearScreen(VPUManager this)
{
	ASSERT(this, "VPUManager::clearScreen: null this");

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
void VPUManager_clearBgmap(VPUManager this, int bgmap, int size)
{
	ASSERT(this, "VPUManager::clearBgmap: null this");

	Mem_clear((u16*)BGMap(bgmap), size);
}

// setup default column table
void VPUManager_setupColumnTable(VPUManager this, ColumnTableDefinition* columnTableDefinition)
{
	ASSERT(this, "VPUManager::setupColumnTable: null this");

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
void VPUManager_useInternalColumnTable(VPUManager this, bool useInternal)
{
	ASSERT(this, "VPUManager::useInternalColumnTable: null this");

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
void VPUManager_setBackgroundColor(VPUManager this, u8 color)
{
	ASSERT(this, "VPUManager::setBackgroundColor: null this");

    if(color > __COLOR_BRIGHT_RED)
    {
        color = __COLOR_BRIGHT_RED;
    }

	VIP_REGS[BKCOL] = color;
}

// register post processing effect
void VPUManager_addPostProcessingEffect(VPUManager this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "VPUManager::addPostProcessingEffect: null this");

	VirtualList_pushBack(this->postProcessingEffects, postProcessingEffect);
}

// remove post processing effect
void VPUManager_removePostProcessingEffect(VPUManager this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "VPUManager::removePostProcessingEffect: null this");

	VirtualList_removeElement(this->postProcessingEffects, postProcessingEffect);
}
// register post processing effect
void VPUManager_removePostProcessingEffects(VPUManager this)
{
	ASSERT(this, "VPUManager::removePostProcessingEffects: null this");

	VirtualList_clear(this->postProcessingEffects);
}

// register the frame buffer in use by the VPU's drawing process
void VPUManager_registerCurrentDrawingframeBufferSet(VPUManager this)
{
	ASSERT(this, "VPUManager::registerCurrentDrawingframeBufferSet: null this");

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
