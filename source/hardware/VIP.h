#ifndef VIP_H_
#define VIP_H_
/*---------------------------------INCLUDES--------------------------------*/
#include <Types.h>


/* Defines for INTPND\INTENB\INTCLR */
#define	TIMEERR		0x8000
#define	XPEND		0x4000
#define	SBHIT		0x2000
#define	FRAMESTART	0x0010
#define	GAMESTART	0x0008
#define	RFBEND		0x0004
#define	LFBEND		0x0002
#define	SCANERR		0x0001

/* Defines for DPSTTS\DPCTRL */
#define	LOCK		0x0400	// VPU SELECT CTA
#define	SYNCE		0x0200	// L,R_SYNC TO VPU
#define	RE			0x0100	// MEMORY REFLASH CYCLE ON
#define	FCLK		0x0080
#define	SCANRDY		0x0040
#define	DISP		0x0002	// DISPLAY ON
#define	DPRST		0x0001	// RESET VPU COUNTER AND WAIT FCLK

/* Defines for XPSTTS\XPCTRL */
#define	SBOUT		0x8000	// In FrameBuffer drawing included
#define	OVERTIME	0x0010	// Processing
#define	XPBSYR		0x000C	// In the midst of drawing processing reset
#define	XPBSY1		0x0008	// In the midst of FrameBuffer1 picture editing
#define	XPBSY0		0x0004	// In the midst of FrameBuffer0 picture editing
#define	XPEN		0x0002	// Start of drawing
#define	XPRST		0x0001	// Forcing idling


/****** VIP Registers ******/
extern volatile u16* VIP_REGS;

/****** VIP Register Mnemonics ******/
#define	INTPND	0x00
#define	INTENB	0x01
#define	INTCLR	0x02

#define	DPSTTS	0x10
#define	DPCTRL	0x11
#define	BRTA	0x12
#define	BRTB	0x13
#define	BRTC	0x14
#define	REST	0x15

#define	FRMCYC	0x17
#define	CTA		0x18

#define	XPSTTS	0x20
#define	XPCTRL	0x21
#define	VER		0x22

#define	SPT0	0x24
#define	SPT1	0x25
#define	SPT2	0x26
#define	SPT3	0x27

#define	GPLT0	0x30
#define	GPLT1	0x31
#define	GPLT2	0x32
#define	GPLT3	0x33

#define	JPLT0	0x34
#define	JPLT1	0x35
#define	JPLT2	0x36
#define	JPLT3	0x37

#define	BKCOL	0x38

#endif
