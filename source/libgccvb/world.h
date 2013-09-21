#ifndef WORLD_H_
#define WORLD_H_
/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "video.h"


typedef struct WORLD {
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

/* "vbSetWorld" header flags */
/* (OR these together to build a World Header) */

#define	WRLD_OFF		0x3FFF	
#define	WRLD_ON		0xC000	// There_are_two_screens!__USE_THEM!!!
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

/* Macros for world manipulation */
// (Obsoleted by the WA array of WORLD structures...)

#define	WORLD_HEAD(n,head)		WAM[(n << 4)    ] = head
#define	WORLD_GSET(n,gx,gp,gy)	WAM[(n << 4) + 1] = gx;WAM[(n << 4) + 2] = gp;WAM[(n << 4) + 3] = gy
#define	WORLD_MSET(n,mx,mp,my)	WAM[(n << 4) + 4] = mx;WAM[(n << 4) + 5] = mp;WAM[(n << 4) + 6] = my
#define	WORLD_SIZE(n,w,h)		WAM[(n << 4) + 7] = w;WAM[(n << 4) + 8] = h
#define WORLD_PARAM(n,p)		WAM[(n << 4) + 9] = ((p - 0x20000) >> 1) & 0xFFF0
#define WORLD_OVER(n,o)			WAM[(n << 4) + 10] = o

#define WORLD_SPACER(n,x,o)			WAM[(n << 4) + 11+x] = o

/***** World Functions *****/

void vbActivateWorldShow();

void vbDesactivateWorldShow();

#endif
