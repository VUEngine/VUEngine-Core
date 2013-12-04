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


#include <Printing.h>



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												IMPLEMENTATIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print hardware register's state
void vbjPrintVIP_REGS(int x,int y){
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "INTPND:", 0);		
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[INTPND],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "INTENB:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[INTENB],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "INTCLR:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[INTCLR],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "DPSTTS:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[DPSTTS],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "DPCTRL:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[DPCTRL],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "FRMCYC:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[FRMCYC],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "CTA:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[CTA],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "XPSTTS:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[XPSTTS],16,3), 0);
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "XPCTRL:", 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x+8,y++, itoa((u32)VIP_REGS[XPCTRL],16,3), 0);
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, "GCLK:", 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int vbjDigitCount(int value){

	int size = 0;

	do{
		value /= 10;
		size++;
		
	}while(value);

	return (size)? size: 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vbjPrintInt(int value,int x,int y){
		
	if(value < 0){
		
		value *= -1;
		
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x++,y,"-", 0);
		
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, 
				itoa((int)(value), 10, vbjDigitCount(value)), 0);
	
	}
	else{

		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), 
				//x - vbjDigitCount(value), y, itoa((int)(value), 10, vbjDigitCount(value)) + 1, 0);				
				x, y, itoa((int)(value), 10, vbjDigitCount(value)), 0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vbjPrintHex(WORD value,int x,int y){
	
	if(0 && value<0){
		
		value *= -1;
		
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x++,y,"-", 0);
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, itoa((int)(value),16,8), 0);
	
	}
	else{
		
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y, itoa((int)(value),16,8), 0);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int vbjIntLen(int value){
	
	int length = 0;
	
	while(value > 0){
		
		value /= 10;
		
		length++;
	}
	(!length)? length++: length;
	
	return length;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vbjPrintFloat(float value,int x,int y){
	
	int sign = 1;
	
	int i = 0;
	
	int length;
	
	int size = 10;

	int decimal = (int)(((float)FIX7_9_FRAC(FTOFIX7_9(value)) / 512.f) * 100.f);
	
	if(value < 0){
		sign = -1;
		
		vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x++,y,"-", 0);
	}
	
	decimal = (int)(((float)FIX7_9_FRAC(FTOFIX7_9(value * sign)) / 512.f) * 100.f);

	// print integral part
	length = vbjIntLen((int)value * sign);

	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x, y, itoa(F_FLOOR(value * sign), 10, length), 0);
	
	// print the dot
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x + length, y, ".", 0);
	
	// print the decimal part
	//
	for(i = 0; size; i++){
		
		if(decimal < size){
			vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x + length + 1 + i,y, itoa(0, 10, 1), 0);			
		}		
		else{
			
			i++;
			break;
		}
		size /= 10;
	}	
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x + length  + i ,y, itoa(decimal, 10, 0), 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vbjPrintText(char *string,int x,int y){
	
	vbPrint(TextureManager_getFreeBgmap(TextureManager_getInstance()), x,y,string, 3);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//render general print output layer
void vbjRenderOutputText(int textLayer, int textBgMap){
	
	unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];
	
	while (*xpstts & XPBSYR);
	
	//set the world's head
    WORLD_HEAD((textLayer), WRLD_ON | textBgMap);
    
    //set the world's size
    WORLD_SIZE((textLayer), 384, 224);
    
    //set the world's ...
    WORLD_GSET((textLayer), 0, __ZZERO, 0);
    
    //set world cuting point
    WORLD_MSET((textLayer), 0, 0, 0);

    //stop rendering at next world layer (creates a flash!)
    WORLD_HEAD((textLayer - 1), WRLD_END);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//setup the bgmap and char memory with printing data
void vbjSetPrintingMemory(int bgMapSegment){
	
	// check that character definitions is not null
	if(!_asciiChar){
		
		_asciiChar = (const u16*)ASCII_CH;
	}
	
	//copy ascii char definition to charsegment 3
	memcpy((void*)(CharSeg3+(254*16)), (void*)_asciiChar, 258 << 4);
    
    //set third char segment's mem usage
    CharSetManager_setChars(CharSetManager_getInstance(),3, 200);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  debug(u32 x){
	
	vbjSetPrintingMemory(TextureManager_getFreeBgmap(TextureManager_getInstance()));
	
	vbjRenderOutputText(31,TextureManager_getFreeBgmap(TextureManager_getInstance()));
		
	vbjPrintText("debug: ",10,10);	
	vbjPrintHex(x,17,10);
	vbjPrintInt(x,17,11);
	while(1);
}

