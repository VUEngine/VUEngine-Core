/*---------------------------------INCLUDES--------------------------------*/
#include "joypad.h"
#include "timer.h"

/*----------------------------------GLOBALS--------------------------------*/
static u16 oldkeydown = 0x0000;
static u16 oldkeyup = 0x0000;
static u16 currentKey = 0x0000;
static u16 previousKey = 0x0000;
/*---------------------------------FUNCTIONS-------------------------------*/

/* ---------------------------------------------------------------------------------------------------------*/
void vbKeyFlush(){
	
	currentKey = previousKey = 0;
}
/* Reads the keypad, returns the 16 button bits */
u16 vbKeyPressed(){
	
	return currentKey;
}

/* ---------------------------------------------------------------------------------------------------------*/
/* Reads the keypad, returns the 16 button bits */
u16 vbReadPad(){
	
	unsigned int volatile *readingStatus =	(unsigned int *)&HW_REGS[SCR];
	HW_REGS[SCR] = (S_INTDIS | S_HW);
	
	// save last pressed key
	previousKey = currentKey;
	
	//wait for screen to idle	
	while (*readingStatus & S_STAT);
	
	// now read the key
	{
	u16 high = HW_REGS[SDHR];
	high <<= 8;
	currentKey = (high | HW_REGS[SDLR]) & 0xFFFC;
	}
	//currentKey = ((((u16)HW_REGS[SDHR]) << 8) | HW_REGS[SDLR]) & 0xFFFC;	
	 
	return currentKey;
}

/* ---------------------------------------------------------------------------------------------------------*/
void vbDisableReadPad(){
	HW_REGS[SCR] |= S_HWDIS;	
}

/* ---------------------------------------------------------------------------------------------------------*/
void vbEnableReadPad(){
	HW_REGS[SCR] &= ~S_HWDIS;	
}

/* ---------------------------------------------------------------------------------------------------------*/
u16 vbPadPreviousKey(){
	return previousKey;
}

/* ---------------------------------------------------------------------------------------------------------*/
/* Check if a button has been pressed since the last read. If button state matches last read, it is returned as 'off' */
u16 vbPadKeyDown(){
	u16 keystate,keydown;
	keystate = vbReadPad() & K_ANY;
	keydown = (oldkeydown & keystate) ^ keystate;
	oldkeydown = keystate;
	return keydown;
}

/* Check if a button has been released since the last read. If button state matches last read, it is returned as 'off'  */
u16 vbPadKeyUp(){
	u16 keystate,keyup;
	keystate = vbReadPad() & K_ANY;
	keyup = (oldkeyup & ~keystate);
	oldkeyup = keystate;
	return keyup;
}

/* ---------------------------------------------------------------------------------------------------------*/
int vbNewKeyPressed(u16 key){
	
	if(currentKey == previousKey){
		
		return 0;
	}
	
	if( (currentKey & key) && !(key & previousKey) ){

		return 1;
	}
	return 0;
}
