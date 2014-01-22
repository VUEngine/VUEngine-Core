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


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <HardwareManager.h>
#include <Game.h>
#include <ClockManager.h>

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


#define HardwareManager_ATTRIBUTES										\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/* Timer manager */													\
	TimerManager timerManager;											\
																		\
	/* VPU manager */													\
	VPUManager vpuManager;												\
																		\
	/* VPU manager */													\
	KeypadManager keypadManager;										\
																		\
	/* HW registry */													\
	u8*  hwRegisters;													\
	

// define the HardwareManager
__CLASS_DEFINITION(HardwareManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

extern u32 key_vector;
extern u32 tim_vector;
extern u32 cro_vector;
extern u32 com_vector;
extern u32 vpu_vector;

// class's constructor
static void HardwareManager_constructor(HardwareManager this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(HardwareManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void HardwareManager_constructor(HardwareManager this){
	
	__CONSTRUCT_BASE(Object);
	
	// set ROM wainting to 1 cycle
	HW_REGS[WCR] |= 0x0001;	
	
	this->hwRegisters =	(u8*)0x02000000;
	this->timerManager = TimerManager_getInstance();
	this->vpuManager = VPUManager_getInstance();
	this->keypadManager = KeypadManager_getInstance();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void HardwareManager_destructor(HardwareManager this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// timer's interrupt handler
void HardwareManager_timerInterruptHandler(){

	//disable interrupts
	TimerManager_setInterrupt(TimerManager_getInstance(), false);
	
	//disable timer
	TimerManager_enable(TimerManager_getInstance(), false);
	
	// update clocks
	ClockManager_update(ClockManager_getInstance(), __TIMER_RESOLUTION);
	
    //clear timer state
	TimerManager_clearStat(TimerManager_getInstance());
	
	//enable timer
	TimerManager_enable(TimerManager_getInstance(), true);
	
	// enable interrupts
	TimerManager_setInterrupt(TimerManager_getInstance(), true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// keypad's interrupt handler
void HardwareManager_keypadInterruptHandler(void){

	Printing_text("CALLED INTERRUPTION", 15, 3);

	// broadcast keypad event
	Game_handleInput(Game_getInstance(), KeypadManager_read(KeypadManager_getInstance()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// cro's interrupt handler
void HardwareManager_croInterruptHandler(void){   // Expantion Port Interupt Handler

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// com's interrupt handler
void HardwareManager_communicationInterruptHandler(void){   // Link Port Interrupt Handler

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vpu's interrupt handler
void HardwareManager_vpuInterruptHandler(void){   // Video Retrace Interrupt Handler

	// wait for frame before rendering
	VPUManager_waitForFrame(VPUManager_getInstance());

	// call game's rendering routine
	Game_render(Game_getInstance());
	
	// enable interrupts
	VPUManager_displayOn(VPUManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup interrupt vectors
void HardwareManager_setInterruptVectors(HardwareManager this){

	key_vector = (u32)HardwareManager_keypadInterruptHandler;
	tim_vector = (u32)HardwareManager_timerInterruptHandler;
	cro_vector = (u32)HardwareManager_croInterruptHandler;
	com_vector = (u32)HardwareManager_communicationInterruptHandler;
	vpu_vector = (u32)HardwareManager_vpuInterruptHandler;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set interruption level
void HardwareManager_setInterruptLevel(HardwareManager this, u8 level) {
	
	asm(" \n\
		stsr	sr5,r5 \n\
		movhi	0xFFF1,r0,r6 \n\
		movea	0xFFFF,r6,r6 \n\
		and		r5,r6 \n\
		mov		%0,r5 \n\
		andi	0x000F,r5,r5 \n\
		shl		0x10,r5 \n\
		or		r6,r5 \n\
		ldsr	r5,sr5 \n\
		"	
	: // Output 
	: "r" (level) // Input 
	: "r5", "r6" // Clobber 
	);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get interruption level
inline int HardwareManager_geInterruptLevel(HardwareManager this) {

	int level;

	asm(" \n\
		stsr	sr5,r5 \n\
		shr		0x10,r5 \n\
		andi	0x000F,r5,r5 \n\
		mov		r5,%0 \n\
	"
	: "=r" (level) // Output 
	: // Input 
	: "r5" // Clobber 
	);
	
	return level;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get PSW
inline int HardwareManager_getPSW(HardwareManager this) {
	
	int psw;
	asm(" \n\
		stsr	psw,%0  \n\
		"	
	: "=r" (psw) // Output 
	);
	return psw;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get stack pointer
int HardwareManager_getStackPointer(HardwareManager this) {
	
	int sp;
	asm(" \
		mov		sp,%0  \
		"	
	: "=r" (sp) // Output 
	);
	return sp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialize timer
void HardwareManager_initializeTimer(HardwareManager this){
	
	TimerManager_initialize(this->timerManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear screen
void HardwareManager_clearScreen(HardwareManager this){
	
	VPUManager_clearScreen(this->vpuManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// display on
void HardwareManager_displayOn(HardwareManager this){

	VPUManager_displayOn(this->vpuManager);
	VPUManager_setupPalettes(this->vpuManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// display off
void HardwareManager_displayOff(HardwareManager this){
	
	VPUManager_displayOff(this->vpuManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// make sure the brigtness is ok
void HardwareManager_upBrightness(HardwareManager this){
	
	VPUManager_upBrightness(this->vpuManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup default column table
void HardwareManager_setupColumnTable(HardwareManager this){
	
	VPUManager_setupColumnTable(this->vpuManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable key pad
void HardwareManager_enableKeypad(HardwareManager this){
	
	KeypadManager_enable(this->keypadManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disable key pad
void HardwareManager_disableKeypad(HardwareManager this){
	
	KeypadManager_disable(this->keypadManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read keypad
u16 HardwareManager_readKeypad(HardwareManager this){
	
	return KeypadManager_read(this->keypadManager);
}
