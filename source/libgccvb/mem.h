#ifndef MEM_H_
#define MEM_H_
/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"

/*---------------------------------CONSTANTS-------------------------------*/
/*
static u8* const	EXPANSION =	(u8*)0x04000000;				// Expansion bus area
static u8* const	WORKRAM =	(u8*)0x05000000;				// Scratchpad RAM; USE WITH CAUTION!
														// (In fact, just leave it alone!)
static u16* const	SAVERAM =	(u16*)0x06000000;				// Cartridge's Battery-backed SRAM
*/

/*---------------------------------FUNCTIONS-------------------------------*/

//static void copymem (u8* dest, const u8* src, u16 num);
void setMem (u8* dest, u16 src, u16 num);
void clearMemFast(u32* dest,  u16 num );
void clearMem (u16* dest,  u16 num ) ;
void subMem (u8* dest, const u8* src, u16 num, u16 offset,u8 modifier);
void addmem (u8* dest, const u8* src, u16 num, u16 offset);
//void addMem (u16* dest, const u16* src, u16 num, u16 offset);
void copymem (int* dest, const int* src, u16 num);


#endif
