/*---------------------------------INCLUDES--------------------------------*/
#include "timer.h"
#include "asm.h"
/*----------------------------------GLOBALS--------------------------------*/
static u8 tcr_val = 0;
/*---------------------------------FUNCTIONS-------------------------------*/

void timerEnable(int enb) {
	if (enb){ 
		tcr_val |= TIMER_ENB;
	}		
	else{
		tcr_val &= ~TIMER_ENB;		
	}
	HW_REGS[TCR] = tcr_val;
}


u16 timerGet() {
	return (HW_REGS[TLR] | (HW_REGS[THR] << 8));
}

void timerSet(u16 time) {
	HW_REGS[TLR] = (time & 0xFF);
	HW_REGS[THR] = (time >> 8);
}

void timerFreq(int freq) {
	if (freq) tcr_val |= TIMER_20US;
	else tcr_val &= ~TIMER_20US;
	HW_REGS[TCR] = tcr_val;
}


void timerInt(int enb) {
	if (enb) tcr_val |= TIMER_INT;
	else tcr_val &= ~TIMER_INT;
	HW_REGS[TCR] = tcr_val;
}


int timerGetStat() {
	//return (!!(HW_REGS[TCR] & TIMER_ZSTAT));
	return (HW_REGS[TCR] & TIMER_ZSTAT);
}

void timerClearStat() {
	HW_REGS[TCR] = (tcr_val|TIMER_ZCLR);

}

void timerInitialize() {
	//setup timer interrupts
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	VIP_REGS[INTENB] = 0x0000;				//This is only for enabling\disabling different kinds of VPU and error ints
	setIntLevel(0);
	//INT_ENABLE;		
	//setup timer
	timerFreq(TIMER_100US);
	timerSet(TIME_MS(__TIMER_RESOLUTION));
	timerClearStat();
	timerInt(1);
	timerEnable(1);
}
// Timer Interrupt Handler
// Create a timed delay

