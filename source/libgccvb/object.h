#ifndef LIBGCCVB_OBJECT_H_
#define LIBGCCVB_OBJECT_H_

/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "video.h"

/***** Object Manipulation Macros *****/

#define	OBJ_XSET(n,x)		OAM[ (n) << 2]      = (x)
#define	OBJ_YSET(n,y)		OAM[((n) << 2) + 2] = (y)
#define OBJ_PSET(n,p)		OAM[((n) << 2) + 1] = (OAM[((n) << 2) + 1] & 0xC000) | ((u16)(p) & 0x3FFF)
#define	OBJ_CSET(n,c)		OAM[((n) << 2) + 3] = (OAM[((n) << 2) + 3] & 0xF000) | ((u16)(c) & 0x07FF)
#define	OBJ_HSET(n,h)		OAM[((n) << 2) + 3] = (OAM[((n) << 2) + 3] & 0xDFFF) | (((u16)(h) << 13) & 0x2000)
#define	OBJ_VSET(n,v)		OAM[((n) << 2) + 3] = (OAM[((n) << 2) + 3] & 0xEFFF) | (((u16)(v) << 12) & 0x1000)
#define OBJ_PALSET(n,pal)	OAM[((n) << 2) + 3] = (((pal) << 14) & 0xC000) | (OAM[((n) << 2) + 3] & 0x3FFF)
#define OBJ_VIS(n,v)		OAM[((n) << 2) + 1] = (((v) << 14) & 0xC000) | (OAM[((n) << 2) + 1] & 0x3FFF)

/* Object attributes */
#define	JX			0				// Display pointer X
#define	JP			2				// Paralax
#define	JY			4				// Display pointer Y
#define	JCA			6				// Character No.

/***** "vbSetObject" header flags *****/	// (OR these together to build an Object Header)
#define	JLON		0x8000
#define	JRON		0x4000

#define	OBJ_ON		0x00C0
#define	OBJ_LON		0x0080
#define	OBJ_RON		0x0040

#define	OBJ_PAL0	0x0000
#define	OBJ_PAL1	0x0010
#define	OBJ_PAL2	0x0020
#define	OBJ_PAL3	0x0030

#define	OBJ_HFLIP	0x0008
#define	OBJ_VFLIP	0x0004

#endif
