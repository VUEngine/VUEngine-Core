#ifndef JOYPAD_H_
#define JOYPAD_H_

/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "hw.h"

/*---------------------------------CONSTANTS-------------------------------*/
/* Hardware reg SCR definitions */
#define	S_INTDIS	0x80 	// Disable Interrups
#define	S_SW		0x20 	// Software Reading
#define	S_SWCK		0x10 	// Software Clock, Interrupt?
#define	S_HW		0x04 	// Hardware Reading
#define	S_STAT		0x02 	// Hardware Reading Status
#define	S_HWDIS		0x01	// Disable Hardware Reading

/* Keypad Definitions */
#define	K_ANY	0xFFFC		// All keys, without pwr & sgn
#define	K_BTNS	0x303C		// All buttons; no d-pads, pwr or sgn
#define	K_PADS	0xCFC0		// All d-pads
#define	K_LPAD	0x0F00		// Left d-pad only
#define	K_RPAD	0xC0C0		// Right d-pad only
#define	K_PWR	0x0001		// Low Battery
#define	K_SGN	0x0002		// Signature; 1 = Standard Pad
#define	K_A		0x0004		// A Button
#define	K_B		0x0008		// B Button
#define	K_RT	0x0010		// R Trigger
#define	K_LT	0x0020		// L Trigger
#define	K_RU	0x0040		// Right Pad, Up
#define	K_RR	0x0080		// Right Pad, Right
#define	K_LR	0x0100		// Left Pad,  Right
#define	K_LL	0x0200		// Left Pad,  Left
#define	K_LD	0x0400		// Left Pad,  Down
#define	K_LU	0x0800		// Left Pad,  Up
#define	K_STA	0x1000		// Start Button
#define	K_SEL	0x2000		// Select Button
#define	K_RL	0x4000		// Right Pad, Left
#define	K_RD	0x8000		// Right Pad, Down


//0110 0000 0000 0000
//1100 0000 0000 0000


/*---------------------------------FUNCTIONS-------------------------------*/
void vbKeyFlush();
u16 vbKeyPressed();
u16 vbReadPad();
/* Check if a button has been pressed since the last read. If button state matches last read, it is returned as 'off' */
u16 vbPadKeyDown();
/* Check if a button has been released since the last read. If button state matches last read, it is returned as 'off'  */
u16 vbPadKeyUp();
u16 vbPadPreviousKey();

void vbDisableReadPad();
void vbEnableReadPad();

int vbNewKeyPressed(u16 key);
#endif
