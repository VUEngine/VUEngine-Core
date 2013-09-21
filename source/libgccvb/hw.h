#ifndef HW_H_
#define HW_H_

/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"


static u8* const HW_REGS =			(u8*)0x02000000;

/***** Hardware Register Mnemonics *****/
#define	CCR		0x00	// Communication Control Register	(0x0200 0000)
#define	CCSR	0x04	// COMCNT Control Register			(0x0200 0004)
#define	CDTR	0x08	// Transmitted Data Register		(0x0200 0008)
#define	CDRR	0x0C	// Received Data Register			(0x0200 000C)
#define	SDLR	0x10	// Serial Data Low Register			(0x0200 0010)
#define	SDHR	0x14	// Serial Data High Register		(0x0200 0014)
#define	TLR		0x18	// Timer Low Register				(0x0200 0018)
#define	THR		0x1C	// Timer High Register				(0x0200 001C)
#define	TCR		0x20	// Timer Control Register			(0x0200 0020)
#define	WCR		0x24	// Wait-state Control Register		(0x0200 0024)
#define	SCR		0x28	// Serial Control Register			(0x0200 0028)


/********Cache Management***************/
#define CACHE_ENABLE    asm("mov 2,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */) 
#define CACHE_DISABLE    asm("ldsr r0,sr24")

#endif
