/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef __DEBUG

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Debug.h>
#include <Game.h>
#include <MemoryPool.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <Globals.h>
#include <FrameRate.h>
#include <Clock.h>
#include <TextureManager.h>
#include <Level.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <SoundManager.h>
#include <StateMachine.h>
#include <Screen.h>
#include <Background.h>
#include <Image.h>
#include <VPUManager.h>
#include <Printing.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define Debug_ATTRIBUTES			\
									\
	/* super's attributes */		\
	Object_ATTRIBUTES;				\
									\
	/* current page */				\
	int currentPage;				\


	

// define the Debug
__CLASS_DEFINITION(Debug);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void Debug_constructor(Debug this);

static void Debug_showPage(Debug this);

static void Debug_printClassSizes(int x, int y);


enum DebugPages{
	kGeneralStatusPage = 0,
	kMemoryPage,
	kCharSetPage,
	kGraphicsPage,
	kHardwarePage,
	kLastPage
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define __DEBUG_TOTAL_PAGES		3

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(Debug);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Debug_constructor(Debug this){

	ASSERT(this, "Debug::constructor: null this");

	__CONSTRUCT_BASE(Object);
	
	this->currentPage = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Debug_destructor(Debug this){
	
	ASSERT(this, "Debug::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// show debug screens
void Debug_show(Debug this){
	
	//TODO
#define __PRINTING_BGMAP (__NUM_BGMAPS + 1)

	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP);

	//VIP_REGS[GPLT0] = 0;	/* Set all eight palettes to: 11100100 */
	//VIP_REGS[GPLT1] = 0;	/* (i.e. "Normal" dark to light progression.) */
	VIP_REGS[GPLT2] = __GPLT2VALUE;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = 0;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
	
	Debug_showPage(this);
}

// show debug screens
void Debug_hide(Debug this){

	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP);
	//VPUManager_setupPalettes(VPUManager_getInstance());
	VIP_REGS[GPLT0] = __GPLT0VALUE;	/* Set all eight palettes to: 11100100 */
	VIP_REGS[GPLT1] = __GPLT1VALUE;	/* (i.e. "Normal" dark to light progression.) */
	VIP_REGS[GPLT2] = __GPLT2VALUE;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = __JPLT0VALUE;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
}

// show debug screens
void Debug_showNext(Debug this){
	
	if(++this->currentPage >= kLastPage) {
		
		this->currentPage = 0;
	}

	Debug_showPage(this);
}

// show debug screens
void Debug_showPrevious(Debug this){
	
	if(0 > --this->currentPage) {
		
		this->currentPage = kLastPage - 1;
	}
	
	Debug_showPage(this);
}

static void Debug_showPage(Debug this) {
	
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP);
	
	Printing_text("DEBUG SYSTEM", 15, 0);
	
	int y = 3;

	switch(this->currentPage) {
	
		case kGeneralStatusPage:
			
			Printing_text("GENERAL STATUS", 1, y++);
			Printing_text("General clock's time: ", 1, ++y);
			Clock_print(Game_getClock(Game_getInstance()), 23, y);
			Printing_text("In game clock's time: ", 1, ++y);
			Clock_print(Game_getInGameClock(Game_getInstance()), 23, y);
	    	FrameRate_print(FrameRate_getInstance(), 1, y + 10);
			break;
			
		case kMemoryPage:
	    	MemoryPool_printMemUsage(MemoryPool_getInstance(), 1, y);
			Debug_printClassSizes(22, y);
			break;
			
		case kCharSetPage:
			CharSetManager_print(CharSetManager_getInstance(), 1, y);
			break;

		case kGraphicsPage:
			TextureManager_print(TextureManager_getInstance(), 1, y);
			SpriteManager_print(SpriteManager_getInstance(), 1, y + 5);
			ParamTableManager_print(ParamTableManager_getInstance(), 25, y);
			break;


		case kHardwarePage:
			
			HardwareManager_print(HardwareManager_getInstance(), 1, y);
			break;
			
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Debug_printClassSizes(int x, int y){

	int columnIncrement = 20;

	Printing_text("CLASSES' MEMORY USAGE (B) ", x, y++);
	Printing_text("Name				Size", x, ++y);
	Printing_text("AnimatedSprite", x, ++y);
	Printing_int(AnimatedSprite_getObjectSize(), x + columnIncrement, y);
	Printing_text("Background", x, ++y);
	Printing_int(Background_getObjectSize(), x + columnIncrement, y);
	Printing_text("Character", x, ++y);
	Printing_int(Actor_getObjectSize(), x + columnIncrement, y);
	Printing_text("CharGroup", x, ++y);
	Printing_int(CharGroup_getObjectSize(), x + columnIncrement, y);
	Printing_text("Clock", x, ++y);
	Printing_int(Clock_getObjectSize(), x + columnIncrement, y);
	Printing_text("Entity", x, ++y);
	Printing_int(Entity_getObjectSize(), x + columnIncrement, y);
	Printing_text("Image", x, ++y);
	Printing_int(Image_getObjectSize(), x + columnIncrement, y);
	Printing_text("InGameEntity", x, ++y);
	Printing_int(InGameEntity_getObjectSize(), x + columnIncrement, y);
//	Printing_text("Rect", x, ++y);
//	Printing_int(Rect_getObjectSize(), x + columnIncrement, y);
	Printing_text("Shape", x, ++y);
	Printing_int(Shape_getObjectSize(), x + columnIncrement, y);
	Printing_text("Sprite", x, ++y);
	Printing_int(Sprite_getObjectSize(), x + columnIncrement, y);
	Printing_text("State", x, ++y);
	Printing_int(State_getObjectSize(), x + columnIncrement, y);
	Printing_text("StateMachine", x, ++y);
	Printing_int(StateMachine_getObjectSize(), x + columnIncrement, y);
	//vbjPrintText("Scroll", x, ++y);
	//vbjPrintInt(Scroll_getObjectSize(), x + columnIncrement, y);
	Printing_text("Telegram", x, ++y);
	Printing_int(Telegram_getObjectSize(), x + columnIncrement, y);;
	Printing_text("Texture", x, ++y);
	Printing_int(Texture_getObjectSize(), x + columnIncrement, y);
	Printing_text("VirtualList", x, ++y);
	Printing_int(VirtualList_getObjectSize(), x + columnIncrement, y);
	Printing_text("VirtualNode", x, ++y);
	Printing_int(VirtualNode_getObjectSize(), x + columnIncrement, y);
}

#endif 
