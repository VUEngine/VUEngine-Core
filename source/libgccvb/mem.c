/*---------------------------------INCLUDES--------------------------------*/
#include "mem.h"
#include "hw.h"

/*---------------------------------FUNCTIONS-------------------------------*/

// Copy a block of data from one area in memory to another.
void copymem (int* dest, const int* src, u16 num){
	
	u16 i = 0;
	num >>= 2;
	CACHE_ENABLE;
	for (; i < num; i++){
		
		*dest++ = *src++;
	}
	// in theory it is faster (produces a bug)
	/*
	src += num;
	dest += num;
	for (; i--;) *dest-- = *src--;
	*/
	CACHE_DISABLE;
}

// Set each byte in a block of data to a given value.
void setMem (u8* dest, u16 src, u16 num){
	u16 i;
	for (i = 0; i < num; i++,dest++) {
		*dest += src;	
		dest++;
		*dest= src>>8;
	}
}

/*	Copy a block of data from one area in memory to another,
adding a given value to each byte, first.	*/
/*
void addmem0 (u8* dest, const u8* src, u16 num, u8 offset) {
	u16 i;
	for (i = 0; i < num; i++) *dest++ = (*src++ + offset);
}
*/

/*	Clear memory.	*/
/*
void clearMem1 (u8* dest,  u16 num ) {
	u16 i;
	for (i = 0; i < num; i++) *dest++ = 0x00;
}
*/
void clearMemFast(u32* dest,  u16 num ){
	u16 i;
	num >>= 1;
	
	//memset(dest, 0, num);
	for (i = 0; i < num; i += 16) {
		
		//*dest++ = 0x00000000;
		dest[i]    = 0x00000000;
		dest[i+1]  = 0x00000000;
		dest[i+2]  = 0x00000000;
		dest[i+3]  = 0x00000000;
		dest[i+4]  = 0x00000000;
		dest[i+5]  = 0x00000000;
		dest[i+6]  = 0x00000000;
		dest[i+7]  = 0x00000000;
		dest[i+8]  = 0x00000000;
		dest[i+9]  = 0x00000000;
		dest[i+10] = 0x00000000;
		dest[i+11] = 0x00000000;
		dest[i+12] = 0x00000000;
		dest[i+13] = 0x00000000;
		dest[i+14] = 0x00000000;
		dest[i+15] = 0x00000000;
		
	}

}

void clearMem (u16* dest,  u16 num ) {
	u16 i;
	for (i = 0; i < num; i++) *dest++ = 0x0000;
}
void subMem (u8* dest, const u8* src, u16 num, u16 offset,u8 modifier) {
	
}

void addmem (u8* dest, const u8* src, u16 num, u16 offset) {
	u16 i;
	for (i = 0; i < num; i++) {
		*dest++ = *src++ + offset;
		*dest++ =*src++ + (u8)(offset>>8);
		}	
}
/*
void addMem(u16* dest, const u16* src, u16 num, u16 offset) {
	
	u16 i = 0;
	CACHE_ENABLE;

	for (; i < num; i++) {
		*dest++ = *src++ + offset;		
	}
		
	CACHE_DISABLE;
}
*/
