
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include "interrupts.h"
#include "vip.h"
#include "misc.h"
#include "joypad.h"


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

typedef struct ClockManager_str* const ClockManager;

extern ClockManager ClockManager_getInstance();
extern void ClockManager_update(ClockManager this, u32 ticksElapsed);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INTERRUPT FUNCTIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


void key_hnd(void){   // Controller Interrupt Handler
	while(1);
}


void tim_hnd(){

	//disable interrupts
	timerInt(false);
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	
	//disable timer
	timerEnable(0);
	
	// update clocks
	ClockManager_update(ClockManager_getInstance(), __TIMER_RESOLUTION);
	
    //clear timer state
	timerClearStat();
	
	//enable timer
	timerEnable(1);
	
	// enable interrupts
    timerInt(true);
}

void cro_hnd(void){   // Expantion Port Interupt Handler

}
void com_hnd(void){   // Link Port Interrupt Handler

}
void vpu_hnd(void){   // Video Retrace Interrupt Handler


}
void setInterruptsVectors(){

	key_vector = (u32)key_hnd;
	tim_vector = (u32)tim_hnd;
	cro_vector = (u32)cro_hnd;
	com_vector = (u32)com_hnd;
	vpu_vector = (u32)vpu_hnd;
}
