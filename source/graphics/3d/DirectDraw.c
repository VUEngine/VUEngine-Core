/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DirectDraw.h>
#include <Cuboid.h>
#include <Polygon.h>

//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

//left buffer base addresses
#define __LEFT_BUFFER_1 (u32)0x00000000

//right buffer base address
#define __RIGHT_BUFFER_1 (u32)0x00010000


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// DirectDraw.c

#define DirectDraw_ATTRIBUTES					\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* actuar frames per second */				\
	int fps;


// define the DirectDraw
__CLASS_DEFINITION(DirectDraw);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void DirectDraw_constructor(DirectDraw this);

// draw a pixel on the screen (DirectDraw)
static void DirectDraw_putPixel(DirectDraw this, u32 buffer, int x, int y, int pallet);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(DirectDraw);


// class's constructor
static void DirectDraw_constructor(DirectDraw this)
{
	__CONSTRUCT_BASE(Object);

	this->fps = 0;
}

// class's destructor
void DirectDraw_destructor(DirectDraw this)
{
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// draw a pixel on the screen (DirectDraw)
static void DirectDraw_putPixel(DirectDraw this, u32 buffer, int x, int y, int pallet)
{
	// a pointer to the buffer
	//int* pointer = (int*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position each column has 16 words,
	// so 16 * 4 bytes = 64, 8 bytes are 4 pixels
	// pointer += x * 64 + y / 4;
	pointer += ((x << 6) + (y >> 2));

	// calculate the pixel to be draw
	*pointer |= (pallet << ((y & 3) << 1));
}

// line draw algorithm from ....
void DirectDraw_lineFast(DirectDraw this, VBVec2D fromPoint, VBVec2D toPoint, int pallet)
{
	fromPoint.x = FIX19_13TOI(fromPoint.x);
	fromPoint.y = FIX19_13TOI(fromPoint.y);

	toPoint.x = FIX19_13TOI(toPoint.x);
	toPoint.y = FIX19_13TOI(toPoint.y);

	int dx = toPoint.x - fromPoint.x;
	int dy = toPoint.y - fromPoint.y;

	int stepX = 0, stepY = 0;

	int parallax = fromPoint.parallax;

	if (dy < 0) { dy = -dy;  stepY = -1; } else { stepY = 1; }
	if (dx < 0) { dx = -dx;  stepX = -1; } else { stepX = 1; }

	dy <<= 1;
	dx <<= 1;

	if (((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
{
		DirectDraw_putPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, pallet);
		DirectDraw_putPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, pallet);
	}

	int counter = 0;
	if (dx > dy)

{
		int fraction = dy - (dx >> 1);

		int parallaxStep = (dx >> 1)? ((toPoint.parallax - fromPoint.parallax) / abs(dx >> 1)): 0;

		while (fromPoint.x != toPoint.x)

{
			if (++counter > 100) break;
			if (fraction >= 0)
{
				fromPoint.y += stepY;
				fraction -= dx;
			}

			fromPoint.x += stepX;
			fraction += dy;

			parallax += parallaxStep;

			if (((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
{
				DirectDraw_putPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, pallet);
				DirectDraw_putPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, pallet);
			}
		}

	}
	else
{
		int fraction = dx - (dy >> 1);

		int parallaxStep = (dy >> 1)? ((toPoint.parallax - fromPoint.parallax) / abs(dy >> 1)): 0;

		while (fromPoint.y != toPoint.y)
{
			if (++counter > 100) break;

			if (fraction >= 0)

{
				fromPoint.x += stepX;
				fraction -= dy;
			}

			fromPoint.y += stepY;
			fraction += dx;

			parallax += parallaxStep;

			if (((unsigned)(fromPoint.x) < __SCREEN_WIDTH)&&((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
{
				DirectDraw_putPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, pallet);
				DirectDraw_putPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, pallet);
			}
		}
	}
}

