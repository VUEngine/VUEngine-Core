/*---------------------------------INCLUDES--------------------------------*/
#include "video.h"
#include "timer.h"


/*---------------------------------FUNCTIONS-------------------------------*/

/* Delay execution */
/*
void vbWaitFrame(u16 count) {
	u16 i;

	for (i = 0; i < count; i++) {
		VIP_REGS[INTCLR] = VIP_REGS[INTPND];
		while (!(VIP_REGS[INTPND] & XPEND)); //FRAMESTART
	}
}
*/


void vbWaitFrame(u16 count) {
	
        u16 i;
        for (i = 0; i <= count; i++) {
                while (!(VIP_REGS[XPSTTS] & XPBSYR));
                while (VIP_REGS[XPSTTS] & XPBSYR);
        }
}


/* Turn the display on */
void vbDisplayOn() {
	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VIP_REGS[DPCTRL] = VIP_REGS[DPSTTS] | (SYNCE | RE | DISP);
//	VIP_REGS[FRMCYC] = 0;
//	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	
	//while (!(VIP_REGS[DPSTTS] & 0x3C));  //required?

	VIP_REGS[BRTA]  = __BRTA;
	VIP_REGS[BRTB]  = __BRTB;
	VIP_REGS[BRTC]  = __BRTC;	
	VIP_REGS[GPLT0] = __GPLT0VALUE;	/* Set all eight palettes to: 11100100 */
	VIP_REGS[GPLT1] = __GPLT1VALUE;	/* (i.e. "Normal" dark to light progression.) */
	VIP_REGS[GPLT2] = __GPLT2VALUE;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = __JPLT0VALUE;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
	VIP_REGS[BKCOL] = __BKCOL;	/* Clear the screen to black before rendering */

}

/* Turn the display off */
void vbDisplayOff() {
	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = 0;
	VIP_REGS[DPCTRL] = 0;
	VIP_REGS[FRMCYC] = 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
}

/* Call this after the display is on and you want the image to show up */
void vbDisplayShow() {
	VIP_REGS[BRTA] = 32;
	VIP_REGS[BRTB] = 64;
	VIP_REGS[BRTC] = 32;
	
}

/* Call this to hide the image; e.g. while setting things up */
void vbDisplayHide() {
	VIP_REGS[BRTA] = 0;
	VIP_REGS[BRTB] = 0;
	VIP_REGS[BRTC] = 0;
}

void vbFXFadeIn1(u16 wait) {
	u8 i;

	for (i = 0; i <= 32; i++) {
		vbWaitFrame(wait);
		SET_BRIGHT(i,i*2,i);
	}
}

void vbFXFadeOut1(u16 wait) {
	s8 i;

	for (i = 32; i >= 0; i--) {
		vbWaitFrame(wait);
		SET_BRIGHT(i,i*2,i);
	}
}
//extern void delay(u16);

/*
void vbFXFadeIn(u16 wait) {
	u8 i;

	for (i = 0; i <= 32; i++) {
		if(wait){
			timerDelay(wait);
		}
		SET_BRIGHT(i,i*2,i);
	}
}

void vbFXFadeOut(u16 wait) {
	s8 i;

	for (i = 32; i >= 0; i--) {
		if(wait){
			timerDelay(wait);
		}
		SET_BRIGHT(i,i*2,i);
	}
}
*/
void vbClearBGMap(int bgMapSegment,int numChars){

        clearMemFast ((u32*)BGMap(bgMapSegment), numChars);
}

void vbClearScreen(){
	int i;
	//clear every bgmap segment
    for(i = 0; i < 14; i++){
		clearMem ((u16*)BGMap(i), 8192);		
    }
	//clear every char segment
	clearMem ((u16*) CharSeg0, 8192);
	clearMem ((u16*) CharSeg1, 8192);
	clearMem ((u16*) CharSeg2, 8192);
	clearMem ((u16*) CharSeg3, 8192);
}



const static BYTE colTable[128] = {
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xe0, 0xbc,
	0xa6, 0x96, 0x8a, 0x82, 0x7a, 0x74, 0x6e, 0x6a,
	0x66, 0x62, 0x60, 0x5c, 0x5a, 0x58, 0x56, 0x54,
	0x52, 0x50, 0x50, 0x4e, 0x4c, 0x4c, 0x4a, 0x4a,
	0x48, 0x48, 0x46, 0x46, 0x46, 0x44, 0x44, 0x44,
	0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3c,
	0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
	0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c
};

/* Setup the default Column Table */
void vbSetColTable() {
	int i;
	for (i = 0; i < 128; i++) {
		CLMN_TBL[i] = colTable[i];
		CLMN_TBL[i + 0x0080] = colTable[127 - i];
		CLMN_TBL[i + 0x0100] = colTable[i];
		CLMN_TBL[i + 0x0180] = colTable[127 - i];
	}
}