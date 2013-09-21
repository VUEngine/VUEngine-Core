#ifndef MISC_H_
#define MISC_H_
/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "mem.h"
#include "video.h"
/*---------------------------------CONSTANTS-------------------------------*/
#define __ROT_LEFT 0x00
#define __ROT_RIGHT !__ROT_LEFT

#define tabsize 4 //horizontal tab size in chars

#define true 	(u8)1	
#define false 	(u8)0
//#define NULL	(void *)0x00000000
static const char nums[17]="0123456789ABCDEF";

/*---------------------------------FUNCTIONS-------------------------------*/
char *itoa(u32 num, u8 base, u8 digits);
void cls();
void vbTextOut(u16 bgmap, u16 col, u16 row, char *t_string);
void vbPrint(u8 bgmap, u16 x, u16 y,const char *t_string, u16 bplt);
WORD vbRotate(WORD invalue, int places, int direction);

#endif
