#ifndef VIDEO_H_
#define VIDEO_H_
/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "vip.h"
#include "mem.h"
#include "timer.h"
#include "affine.h"

/*-----------------------------------MACROS--------------------------------*/

/***** Display RAM *****/
/*@null@*/
static u32* const	L_FRAME0 =	(u32*)0x00000000;				// Left Frame Buffer 0
#define		CharSeg0		 0x00006000					// Characters 0-511
static u32* const	L_FRAME1 =	(u32*)0x00008000;				// Left Frame Buffer 1
#define		CharSeg1		 0x0000E000					// Characters 512-1023
static u32* const	R_FRAME0 =	(u32*)0x00010000;				// Right Frame Buffer 0
#define		CharSeg2		 0x00016000					// Characters 1024-1535
static u32* const	R_FRAME1 =	(u32*)0x00018000;				// Right Frame Buffer 1
#define		CharSeg3		 0x0001E000					// Characters 1536-2047
#define		BGMMBase		 0x00020000					// Base address of BGMap Memory
static u16* const	BGMM =		(u16*)BGMMBase;					// Pointer to BGMM
#define		BGMap(b)		 (BGMMBase + (b * 0x2000))	// Address of BGMap b (0 <= b <= 13)

#define		CHARBase		0x00006000
//#define		CHARBase		0x00078000
static u16* const	CHARB =		(u16*)CHARBase;
#define		CharSegs(b)	(CHARBase + (b*0x8000))		// Address of CharSet b (0 <= b <= 3)

//#define		PARAMBase		0x00034000
#define		PARAMBase		0x00032000
static u16* const	PARAM =		(u16*)PARAMBase;
#define		PARAM(b)	(PARAMBase + (b&0xFFF0))		// Address of CharSet b (0 <= b <= 3)
//#define		PARAM(b)	(PARAMBase + (b&0xFFF0)*1)		// Address of CharSet b (0 <= b <= 3)


#define		WAMBase			 0x0003D800					// Base address of World Attribute Memory
static u16* const	WAM =		(u16*)WAMBase;					// Pointer to WAM
#define		World(w)		 (WAMBase + (w * 0x0020))	// Address of World w (0 <= w <= 31)
static u16* const	CLMN_TBL =	(u16*)0x0003DC00;				// Base address of Column Tables
#define		OAMBase			 0x0003E000					// Base address of Object Attribute Memory
static u16* const	OAM =		(u16*)OAMBase;					// Pointer to OAM
#define		Object(o)		 (OAMBase + (o * 0x0008))	// Address of Obj o (0 <= o <= 1023)

/* Macro to set the brightness registers */
#define	SET_BRIGHT(a,b,c)       VIP_REGS[BRTA]=(u16)(a); VIP_REGS[BRTB]=(u16)(b); VIP_REGS[BRTC]=(u16)(c)

/* Macro to set the GPLT (BGMap palette) */
#define	SET_GPLT(n,pal)         VIP_REGS[GPLT0+n]=pal

/* Macro to set the JPLT (OBJ palette) */
#define	SET_JPLT(n,pal)         VIP_REGS[JPLT0+n]=pal



/*---------------------------------FUNCTIONS-------------------------------*/

void vbWaitFrame1(u16 count);
void vbWaitFrame(u16 count);
void vbDisplayOn();
void vbDisplayOff();
void vbDisplayShow();
void vbDisplayHide();
void vbFXFadeIn1(u16 wait);
void vbFXFadeOut1(u16 wait);
void vbFXFadeIn(u16 wait);
void vbFXFadeOut(u16 wait);
void vbClearBGMap(int bgMapSegment,int numChars);
void vbClearScreen();
void vbSetColTable();

#endif
