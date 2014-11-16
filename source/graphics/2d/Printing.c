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
#include <HardwareManager.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												DEFINES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define TAB_SIZE 4 //horizontal tab size in chars
#define __PRINTING_BGMAP (__NUM_BGMAPS + 1)
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												IMPLEMENTATIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// direct printing out method
void Printing_out(u8 bgmap, u16 x, u16 y, const char* string, u16 bplt){

	/* Font consists of the last 256 chars (1792-2047) */
	u16 i=0,pos=0,col=x;
	
	while(string[i])
	{
		pos = (y << 6) + x;

		switch(string[i])
		{
			case 7:
				// Bell (!)
				break;
			case 9:
				// Horizontal Tab
				x = (x / TAB_SIZE + 1) * TAB_SIZE;
				break;
			case 10:
				// Carriage Return
				y++;
				x = col;
				break;
			case 13:
				// Line Feed
				// x = col;
				break;
			default:
				BGMM[(0x1000 * bgmap) + pos] = ((u16)string[i] + 0x700) | (bplt << 14);
				if (x++ > 63)
				{
					x = col;
					y++;
				}
				break;
		}
		i++;
	}
}

// print hardware register's state
void Printing_hwRegisters(int x,int y){

	Printing_text("TCR:", x, y);		
	Printing_hex(HW_REGS[TCR], x+8,y++);

	Printing_text("SCR:", x, y);		
	Printing_hex(HW_REGS[SCR], x+8,y++);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print hardware register's state
void Printing_vipRegisters(int x,int y){
	
	Printing_out(__PRINTING_BGMAP, x,y, "INTPND:", 0);		
//	Printing_out(__PRINTING_BGMAP, x+8,y++,(const char*) Utilities_itoa((u32)VIP_REGS[INTPND],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "INTENB:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[INTENB],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "INTCLR:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[INTCLR],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "DPSTTS:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[DPSTTS],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "DPCTRL:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[DPCTRL],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "FRMCYC:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[FRMCYC],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "CTA:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[CTA],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "XPSTTS:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[XPSTTS],16,3), 0);
	
	Printing_out(__PRINTING_BGMAP, x,y, "XPCTRL:", 0);
	Printing_out(__PRINTING_BGMAP, x+8,y++, Utilities_itoa((u32)VIP_REGS[XPCTRL],16,3), 0);
	Printing_out(__PRINTING_BGMAP, x,y, "GCLK:", 0);
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
void Printing_int(int value,int x,int y){
		
	if(value < 0){
		
		value *= -1;
		
		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0);
		
		Printing_out(__PRINTING_BGMAP, x,y, 
				Utilities_itoa((int)(value), 10, vbjDigitCount(value)), 0);
	
	}
	else{

		Printing_out(__PRINTING_BGMAP, 
				//x - vbjDigitCount(value), y, itoa((int)(value), 10, vbjDigitCount(value)) + 1, 0);				
				x, y, Utilities_itoa((int)(value), 10, vbjDigitCount(value)), 0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Printing_hex(WORD value,int x,int y){
	
	if(0 && value<0){
		
		value *= -1;
		
		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0);
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), 0);
	
	}
	else{
		
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), 0);
	}
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Printing_float(float value,int x,int y){
	
	int sign = 1;
	
	int i = 0;
	
	int length;
	
	int size = 10;

//	int decimal = (int)(((float)FIX7_9_FRAC(FTOFIX7_9(value)) / 512.f) * 100.f);
	
	#define FIX19_13_FRAC(n)		((n)&0x1FFF)

	int decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	if(value < 0){
		sign = -1;
		
		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0);
	}
	
//	decimal = (int)(((float)FIX7_9_FRAC(FTOFIX7_9(value * sign)) / 512.f) * 100.f);
	decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	// print integral part
	length = Utilities_intLength((int)value * sign);

	Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa(F_FLOOR(value * sign), 10, length), 0);
	
	// print the dot
	Printing_out(__PRINTING_BGMAP, x + length, y, ".", 0);
	
	// print the decimal part
	//
	for(i = 0; size; i++){
		
		if(decimal < size){
			Printing_out(__PRINTING_BGMAP, x + length + 1 + i,y, Utilities_itoa(0, 10, 1), 0);			
		}		
		else{
			
			i++;
			break;
		}
		size /= 10;
	}	
	
	Printing_out(__PRINTING_BGMAP, x + length  + i ,y, Utilities_itoa(decimal, 10, 0), 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Printing_text(char *string, int x,int y){
	
	Printing_out(__PRINTING_BGMAP, x,y,string, 3);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//render general print output layer
void Printing_render(int textLayer){
	
	//set the world's head
    WORLD_HEAD((textLayer), WRLD_ON | WRLD_BGMAP | WRLD_OVR | (__PRINTING_BGMAP));
    
    //set the world's size
    WORLD_SIZE((textLayer), 384, 224);
    
    //set the world's ...
    WORLD_GSET((textLayer), 0, __ZZERO, 0);
    
    //set world cuting point
    WORLD_MSET((textLayer), 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//setup the bgmap and char memory with printing data
void Printing_writeAscii(){
	
	// check that character definitions is not null
	if(!_asciiChar){
		
		_asciiChar = (const u16*)ASCII_CH;
	}
	
	//copy ascii char definition to charsegment 3
	Mem_copy((u8*)(CharSeg3 + (254 * 16)), (u8*)_asciiChar, 258 << 4);
	
    //set third char segment's mem usage
   // CharSetManager_setChars(CharSetManager_getInstance(), 3, 200);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Printing_debug(u32 x){
	
	Printing_writeAscii(__PRINTING_BGMAP);
	
	Printing_render(__TOTAL_LAYERS);
		
	Printing_text("debug: ",10,10);	
	Printing_hex(x,17,10);
	Printing_int(x,17,11);
	while(1);
}

