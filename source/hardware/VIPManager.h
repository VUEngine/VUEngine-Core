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

#ifndef VIP_MANAGER_H_
#define VIP_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


/* Defines for __INTPND\__INTENB\__INTCLR */
#define	__TIMEERR		0x8000
#define	__XPEND		    0x4000
#define	__SBHIT		    0x2000
#define	__FRAMESTART	0x0010
#define	__GAMESTART	    0x0008
#define	__RFBEND		0x0004
#define	__LFBEND		0x0002
#define	__SCANERR		0x0001

/* Defines for __DPSTTS\__DPCTRL */
#define	__LOCK		    0x0400	// VPU SELECT __CTA
#define	__SYNCE		    0x0200	// L,R_SYNC TO VPU
#define	__RE		    0x0100	// MEMORY REFLASH CYCLE ON
#define	__FCLK		    0x0080
#define	__SCANRDY	    0x0040
#define	__DISP		    0x0002	// DISPLAY ON
#define	__DPRST		    0x0001	// RESET VPU COUNTER AND WAIT __FCLK

/* Defines for __XPSTTS\__XPCTRL */
#define	__SBOUT		    0x8000	// In FrameBuffer drawing included
#define	__OVERTIME	    0x0010	// Processing
#define	__XPBSYR		0x000C	// In the midst of drawing processing reset
#define	__XPBSY1		0x0008	// In the midst of FrameBuffer1 picture editing
#define	__XPBSY0		0x0004	// In the midst of FrameBuffer0 picture editing
#define	__XPEN		    0x0002	// Start of drawing
#define	__XPRST		    0x0001	// Forcing idling


/****** VIP Registers ******/
extern volatile u16* _vipRegisters;

/****** VIP Register Mnemonics ******/
#define	__INTPND	0x00
#define	__INTENB	0x01
#define	__INTCLR	0x02

#define	__DPSTTS	0x10
#define	__DPCTRL	0x11
#define	__BRTA		0x12
#define	__BRTB		0x13
#define	__BRTC		0x14
#define	__REST		0x15

#define	__FRMCYC	0x17
#define	__CTA		0x18

#define	__XPSTTS	0x20
#define	__XPCTRL	0x21
#define	__VER		0x22

#define	__SPT0		0x24
#define	__SPT1		0x25
#define	__SPT2		0x26
#define	__SPT3		0x27

#define	__GPLT0		0x30
#define	__GPLT1		0x31
#define	__GPLT2		0x32
#define	__GPLT3		0x33

#define	__JPLT0		0x34
#define	__JPLT1		0x35
#define	__JPLT2		0x36
#define	__JPLT3		0x37

#define	__BACKGROUND_COLOR	0x38

// Display RAM
/*@null@*/
#define __LEFT_FRAME_BUFFER_0               0x00000000				// Left Frame Buffer 0
#define __LEFT_FRAME_BUFFER_1               0x00008000				// Left Frame Buffer 1
#define __RIGHT_FRAME_BUFFER_0              0x00010000				// Right Frame Buffer 0
#define __RIGHT_FRAME_BUFFER_1              0x00018000				// Right Frame Buffer 1

#define __CHAR_SEGMENT_0_BASE_ADDRESS       0x00006000					// Characters 0-511
#define __CHAR_SEGMENT_1_BASE_ADDRESS		0x0000E000					// Characters 512-1023
#define __CHAR_SEGMENT_2_BASE_ADDRESS		0x00016000					// Characters 1024-1535
#define __CHAR_SEGMENT_3_BASE_ADDRESS		0x0001E000					// Characters 1536-2047

#define __BGMAP_SPACE_BASE_ADDRESS		    0x00020000					// Base address of BGMap Memory
#define __BGMAP_SEGMENT(b)		                    (__BGMAP_SPACE_BASE_ADDRESS + (b * 0x2000))	// Address of BGMap b (0 <= b <= 13)

#define __CHAR_SPACE_BASE_ADDRESS		0x00006000
#define __CHAR_SEGMENT(b)	(__CHAR_SPACE_BASE_ADDRESS + (b*0x8000))		// Address of CharSet b (0 <= b <= 3)

#define __OBJECT_SPACE_BASE_ADDRESS		0x0003E000  // Base address of Object Attribute Memory

#define __WORLD_SPACE_BASE_ADDRESS		0x0003D800  // Base address of World Attribute Memory


// Macro to set the brightness registers
#define	__SET_BRIGHT(a,b,c)       _vipRegisters[__BRTA]=(u16)(a); _vipRegisters[__BRTB]=(u16)(b); _vipRegisters[__BRTC]=(u16)(c)

// Macro to set the GPLT (BGMap palette)
#define	__SET_G_PALETTE(n,pal)         _vipRegisters[__GPLT0+n]=pal

// Macro to set the JPLT (OBJ palette)
#define	__SET_J_PALETTE(n,pal)         _vipRegisters[__JPLT0+n]=pal

typedef struct WorldAttributes
{
	u16 head;
	s16 gx;
	s16 gp;
	s16 gy;
	u16 mx;
	s16 mp;
	u16 my;
	u16 w;
	u16 h;
	u16 param;
	u16 ovr;
	u16 spacer[5];

} WorldAttributes;

// pointers to access the VRAM base address
static WorldAttributes* const _worldAttributesBaseAddress   =   (WorldAttributes*)__WORLD_SPACE_BASE_ADDRESS;
static u16* const	_columnTableBaseAddressLeft             =   (u16*)0x0003DC00; // base address of Column Table (Left Eye)
static u16* const	_columnTableBaseAddressRight            =   (u16*)0x0003DE00; // base address of Column Table (Right Eye)
static u16* const	_objecAttributesBaseAddress             =   (u16*)__OBJECT_SPACE_BASE_ADDRESS;					// Pointer to _objecAttributesBaseAddress

// "vbSetWorld" header flags
// (OR these together to build a World Header)

#define	__WORLD_OFF		0x3FFF
#define	__WORLD_ON		0xC000
#define	__WORLD_LON		0x8000
#define	__WORLD_RON		0x4000
#define	__WORLD_OBJ		0x3000
#define	__WORLD_AFFINE	0x2000
#define	__WORLD_HBIAS	0x1000
#define	__WORLD_BGMAP	0x0000

#define	__WORLD_1x1		0x0000
#define	__WORLD_1x2		0x0100
#define	__WORLD_1x4		0x0200
#define	__WORLD_1x8		0x0300
#define	__WORLD_2x1		0x0400
#define	__WORLD_2x2		0x0500
#define	__WORLD_2x4		0x0600
#define	__WORLD_4x2		0x0900
#define	__WORLD_4x1		0x0800
#define	__WORLD_8x1		0x0C00

#define	__WORLD_OVR		0x0080
#define	__WORLD_END		0x0040

// param table for affine and hbias render
extern u32 _dram_data_start;
#define __PARAM_TABLE_END 		((u32)&_dram_data_start)

#define	__COLOR_BLACK			0x00
#define	__COLOR_DARK_RED		0x01
#define	__COLOR_MEDIUM_RED		0x02
#define	__COLOR_BRIGHT_RED		0x03

typedef struct ColumnTableDefinition
{
	// defines whether the definition's first half should be mirrored (true)
	// or if a full 256 entry table is provided (false)
	bool mirror;

	// column table definition
	BYTE columnTable[];

} ColumnTableDefinition;

typedef const ColumnTableDefinition ColumnTableROMDef;

typedef struct BrightnessRepeatDefinition
{
	// defines whether the definition's first half should be mirrored (true)
	// or if a full 96 entry table is provided (false)
	bool mirror;

	// brightness repeat values
	u8 brightnessRepeat[];

} BrightnessRepeatDefinition;

typedef const BrightnessRepeatDefinition BrightnessRepeatROMDef;

typedef struct ColorConfig
{
    // background color
	u8 backgroundColor;

    // brightness config
    Brightness brightness;

    // brightness repeat values
    BrightnessRepeatDefinition* brightnessRepeat;

} ColorConfig;

typedef struct PaletteConfig
{
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
#define VIPManager_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define VIPManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(VIPManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

VIPManager VIPManager_getInstance();

void VIPManager_destructor(VIPManager this);
void VIPManager_enableDrawing(VIPManager this);
void VIPManager_disableDrawing(VIPManager this);
void VIPManager_enableInterrupt(VIPManager this);
void VIPManager_disableInterrupt(VIPManager this);
void VIPManager_displayOn(VIPManager this);
void VIPManager_displayOff(VIPManager this);
void VIPManager_setupPalettes(VIPManager this, PaletteConfig* paletteConfig);
void VIPManager_upBrightness(VIPManager this);
void VIPManager_lowerBrightness(VIPManager this);
void VIPManager_displayHide(VIPManager this);
void VIPManager_clearScreen(VIPManager this);
void VIPManager_clearBgmap(VIPManager this, int bgmap, int size);
void VIPManager_setupColumnTable(VIPManager this, ColumnTableDefinition* columnTableDefinition);
void VIPManager_useInternalColumnTable(VIPManager this, bool internal);
void VIPManager_setupBrightnessRepeat(VIPManager this, BrightnessRepeatDefinition* brightnessRepeat);
void VIPManager_setBackgroundColor(VIPManager this, u8 color);
void VIPManager_addPostProcessingEffect(VIPManager this, void (*postProcessingEffect) (u32));
void VIPManager_removePostProcessingEffect(VIPManager this, void (*postProcessingEffect) (u32));
void VIPManager_removePostProcessingEffects(VIPManager this);
void VIPManager_registerCurrentDrawingframeBufferSet(VIPManager this);


#endif
