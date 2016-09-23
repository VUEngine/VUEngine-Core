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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DirectDraw.h>
#include <Cuboid.h>
#include <Polygon.h>
#include <VirtualList.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// left buffer base addresses
#define __LEFT_BUFFER_1 	(u32)0x00000000

// right buffer base address
#define __RIGHT_BUFFER_1 	(u32)0x00010000


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class       DirectDraw
 * @extends     Object
 */

#define DirectDraw_ATTRIBUTES																			\
        Object_ATTRIBUTES																				\

__CLASS_DEFINITION(DirectDraw, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void DirectDraw_constructor(DirectDraw this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(DirectDraw);

/**
 * Class constructor
 *
 * @memberof    DirectDraw
 * @private
 *
 * @param this  Function scope
 */
static void __attribute__ ((noinline)) DirectDraw_constructor(DirectDraw this)
{
	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof    DirectDraw
 * @public
 *
 * @param this  Function scope
 */
void DirectDraw_destructor(DirectDraw this)
{
	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Draws a pixel on the screen.
 * This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
 * instead.
 *
 * @brief           Draws a pixel on the screen
 * @memberof        DirectDraw
 * @public
 *
 * @param this      Function scope
 * @param buffer    Buffer base address
 * @param x         Screen x coordinate
 * @param y         Screen y coordinate
 * @param color     The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
 */
void DirectDraw_drawPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, int x, int y, int color)
{
	// a pointer to the buffer
	//int* pointer = (int*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	pointer += ((x << 6) + (y >> 2));

	// draw the pixel
    *pointer |= (color << ((y & 3) << 1));
}

/**
 * Draws a black pixel on the screen.
 * We have a separate function for black pixes since nulling bits requires a slightly different way than
 * simply writing 1's. Adding an if clause instead to the putPixel method instead would be too heavy on
 * the processor when used inside a loop due to the branching.
 *
 * @brief           Draws a black pixel on the screen
 * @memberof        DirectDraw
 * @public
 *
 * @param this      Function scope
 * @param buffer    Buffer base address
 * @param x         Screen x coordinate
 * @param y         Screen y coordinate
 */
void DirectDraw_drawBlackPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, int x, int y)
{
	// a pointer to the buffer
	//int* pointer = (int*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	pointer += ((x << 6) + (y >> 2));

	// draw the pixel
	*pointer &= ~(0b11 << ((y & 3) << 1));
}

/**
 * Draws a line between two given 2D points
 *
 * @memberof        DirectDraw
 * @public
 *
 * @param this      Function scope
 * @param fromPoint Point 1
 * @param toPoint   Point 2
 * @param color     The color to draw (0-3)
 */
void DirectDraw_drawLine(DirectDraw this, VBVec2D fromPoint, VBVec2D toPoint, int color)
{
	fromPoint.x = FIX19_13TOI(fromPoint.x);
	fromPoint.y = FIX19_13TOI(fromPoint.y);

	toPoint.x = FIX19_13TOI(toPoint.x);
	toPoint.y = FIX19_13TOI(toPoint.y);

	int dx = toPoint.x - fromPoint.x;
	int dy = toPoint.y - fromPoint.y;

	int stepX = 0, stepY = 0;

	int parallax = fromPoint.parallax;

	if(dy < 0)
	{
	    dy = -dy;
	    stepY = -1;
	}
	else
	{
	    stepY = 1;
	}

	if(dx < 0)
	{
	    dx = -dx;
	    stepX = -1;
	}
	else
	{
	    stepX = 1;
	}

	dy <<= 1;
	dx <<= 1;

    // duplicating code here since it is much lighter on the cpu
    if(color == __COLOR_BLACK)
    {
        if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
        {
            DirectDraw_drawBlackPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y);
            DirectDraw_drawBlackPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y);
        }

        if(dx > dy)
        {
            int fraction = dy - (dx >> 1);

            int parallaxStep = (dx >> 1) ? ((toPoint.parallax - fromPoint.parallax) / abs(dx >> 1)) : 0;

            while(fromPoint.x != toPoint.x)
            {
                if(fraction >= 0)
                {
                    fromPoint.y += stepY;
                    fraction -= dx;
                }

                fromPoint.x += stepX;
                fraction += dy;

                parallax += parallaxStep;

                if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
                {
                    DirectDraw_drawBlackPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y);
                    DirectDraw_drawBlackPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y);
                }
            }
        }
        else
        {
            int fraction = dx - (dy >> 1);

            int parallaxStep = (dy >> 1) ? ((toPoint.parallax - fromPoint.parallax) / abs(dy >> 1)) : 0;

            while(fromPoint.y != toPoint.y)
            {
                if(fraction >= 0)
                {
                    fromPoint.x += stepX;
                    fraction -= dy;
                }

                fromPoint.y += stepY;
                fraction += dx;

                parallax += parallaxStep;

                if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH)&&((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
                {
                    DirectDraw_drawBlackPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y);
                    DirectDraw_drawBlackPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y);
                }
            }
        }

    }
    else
    {
        if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
        {
            DirectDraw_drawPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, color);
            DirectDraw_drawPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, color);
        }

        if(dx > dy)
        {
            int fraction = dy - (dx >> 1);

            int parallaxStep = (dx >> 1) ? ((toPoint.parallax - fromPoint.parallax) / abs(dx >> 1)) : 0;

            while(fromPoint.x != toPoint.x)
            {
                if(fraction >= 0)
                {
                    fromPoint.y += stepY;
                    fraction -= dx;
                }

                fromPoint.x += stepX;
                fraction += dy;

                parallax += parallaxStep;

                if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH) && ((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
                {
                    DirectDraw_drawPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, color);
                    DirectDraw_drawPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, color);
                }
            }
        }
        else
        {
            int fraction = dx - (dy >> 1);

            int parallaxStep = (dy >> 1) ? ((toPoint.parallax - fromPoint.parallax) / abs(dy >> 1)) : 0;

            while(fromPoint.y != toPoint.y)
            {
                if(fraction >= 0)
                {
                    fromPoint.x += stepX;
                    fraction -= dy;
                }

                fromPoint.y += stepY;
                fraction += dx;

                parallax += parallaxStep;

                if(((unsigned)(fromPoint.x) < __SCREEN_WIDTH)&&((unsigned)(fromPoint.y) < __SCREEN_HEIGHT))
                {
                    DirectDraw_drawPixel(this, __LEFT_BUFFER_1, fromPoint.x - parallax, fromPoint.y, color);
                    DirectDraw_drawPixel(this, __RIGHT_BUFFER_1, fromPoint.x + parallax, fromPoint.y, color);
                }
            }
        }
    }
}
