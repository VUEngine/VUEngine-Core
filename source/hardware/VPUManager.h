/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef VPU_MANAGER_H_
#define VPU_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// Display RAM
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

#define		WAMBase			 0x0003D800					// Base address of World Attribute Memory
static u16* const	WAM =		(u16*)WAMBase;					// Pointer to WAM
#define		World(w)		 (WAMBase + (w * 0x0020))	// Address of World w (0 <= w <= 31)
static u16* const	CLMN_TBL =	(u16*)0x0003DC00;				// Base address of Column Tables
#define		OAMBase			 0x0003E000					// Base address of Object Attribute Memory
static u16* const	OAM =		(u16*)OAMBase;					// Pointer to OAM
#define		Object(o)		 (OAMBase + (o * 0x0008))	// Address of Obj o (0 <= o <= 1023)

// Macro to set the brightness registers
#define	SET_BRIGHT(a,b,c)       VIP_REGS[BRTA]=(u16)(a); VIP_REGS[BRTB]=(u16)(b); VIP_REGS[BRTC]=(u16)(c)

// Macro to set the GPLT (BGMap palette)
#define	SET_GPLT(n,pal)         VIP_REGS[GPLT0+n]=pal

// Macro to set the JPLT (OBJ palette)
#define	SET_JPLT(n,pal)         VIP_REGS[JPLT0+n]=pal

#define __ROT_LEFT 0x00
#define __ROT_RIGHT !__ROT_LEFT

typedef struct WORLD
{
	u16 head;
	u16 gx;
	s16 gp;
	u16 gy;
	u16 mx;
	s16 mp;
	u16 my;
	u16 w;
	u16 h;
	u16 param;
	u16 ovr;
	u16 spacer[5];

} WORLD;

static WORLD* const WA = (WORLD*)0x0003D800;

// "vbSetWorld" header flags
// (OR these together to build a World Header)

#define	WRLD_OFF	0x3FFF
#define	WRLD_ON		0xC000
#define	WRLD_LON	0x8000
#define	WRLD_RON	0x4000
#define	WRLD_OBJ	0x3000
#define	WRLD_AFFINE	0x2000
#define	WRLD_HBIAS	0x1000
#define	WRLD_BGMAP	0x0000

#define	WRLD_1x1	0x0000
#define	WRLD_1x2	0x0100
#define	WRLD_1x4	0x0200
#define	WRLD_1x8	0x0300
#define	WRLD_2x1	0x0400
#define	WRLD_2x2	0x0500
#define	WRLD_2x4	0x0600
#define	WRLD_4x2	0x0900
#define	WRLD_4x1	0x0800
#define	WRLD_8x1	0x0C00

#define	WRLD_OVR	0x0080
#define	WRLD_END	0x0040

// Macros for world manipulation
// (Obsoleted by the WA array of WORLD structures...)

#define	WORLD_HEAD(n,head)		WAM[(n << 4)    ] = head
#define	WORLD_GSET(n,gx,gp,gy)	WAM[(n << 4) + 1] = gx;WAM[(n << 4) + 2] = gp;WAM[(n << 4) + 3] = gy
#define	WORLD_MSET(n,mx,mp,my)	WAM[(n << 4) + 4] = mx;WAM[(n << 4) + 5] = mp;WAM[(n << 4) + 6] = my
#define	WORLD_SIZE(n,w,h)		WAM[(n << 4) + 7] = w;WAM[(n << 4) + 8] = h
#define WORLD_PARAM(n,p)		WAM[(n << 4) + 9] = ((p - 0x20000) >> 1) & 0xFFF0
#define WORLD_OVER(n,o)			WAM[(n << 4) + 10] = o

#define WORLD_SPACER(n,x,o)		WAM[(n << 4) + 11+x] = o

#define	__PARAM_BASE	(__PARAM_TABLE_END - 0x00002000 * (__TOTAL_NUMBER_OF_BGMAPS_SEGMENTS - BgmapTextureManager_getAvailableBgmapSegmentForParamTable(BgmapTextureManager_getInstance())) - __PRINTABLE_BGMAP_AREA)
#define	__PARAM_DISPLACEMENT(param)	(__PARAM_BASE + (param & 0xFFF0))

#define	__COLOR_BLACK			0x00
#define	__COLOR_DARK_RED		0x01
#define	__COLOR_MEDIUM_RED		0x02
#define	__COLOR_BRIGHT_RED		0x03

typedef struct PaletteConfig
{
	u8 backgroundColor;

	struct Bgmap
	{
		u8 gplt0;
		u8 gplt1;
		u8 gplt2;
		u8 gplt3;
	} bgmap;

	struct Object
	{
		u8 jplt0;
		u8 jplt1;
		u8 jplt2;
		u8 jplt3;
	} object;

} PaletteConfig;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a struct that is not defined here and so is not accessible to the outside world

// declare the virtual methods
#define VPUManager_METHODS																				\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define VPUManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(VPUManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

VPUManager VPUManager_getInstance();

void VPUManager_destructor(VPUManager this);
void VPUManager_enableDrawing(VPUManager this);
void VPUManager_disableDrawing(VPUManager this);
void VPUManager_enableInterrupt(VPUManager this);
void VPUManager_disableInterrupt(VPUManager this);
void VPUManager_displayOn(VPUManager this);
void VPUManager_displayOff(VPUManager this);
void VPUManager_setupPalettes(VPUManager this, PaletteConfig* paletteConfig);
void VPUManager_upBrightness(VPUManager this);
void VPUManager_lowerBrightness(VPUManager this);
void VPUManager_displayHide(VPUManager this);
void VPUManager_clearScreen(VPUManager this);
void VPUManager_clearBgmap(VPUManager this, int bgmap, int size);
void VPUManager_setupColumnTable(VPUManager this);
void VPUManager_setBackgroundColor(VPUManager this, u8 color);
void VPUManager_addPostProcessingEffect(VPUManager this, void (*postProcessingEffect) (u32));
void VPUManager_removePostProcessingEffect(VPUManager this, void (*postProcessingEffect) (u32));
void VPUManager_removePostProcessingEffects(VPUManager this);
void VPUManager_registerCurrentDrawingframeBufferSet(VPUManager this);

#endif
