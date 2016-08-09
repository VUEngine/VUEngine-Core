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

#include <Polygon.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Polygon, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;

static void Polygon_constructor(Polygon this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(Polygon)
__CLASS_NEW_END(Polygon);

// class's constructor
static void Polygon_constructor(Polygon this)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	// don't create the list yet
	this->vertices = NULL;
}

// class's destructor
void Polygon_destructor(Polygon this)
{
	// delete the vertices list
	if(this->vertices)
	{
		VirtualNode node = this->vertices->head;

		// delete each vertex
		for(; node ; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		// delete the list
		__DELETE(this->vertices);
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// add a vertex
void Polygon_addVertex(Polygon this, fix19_13 x, fix19_13 y, fix19_13 z)
{
	// create the vertex
	VBVec3D* vertex = __NEW_BASIC(VBVec3D);
	vertex->x = x;
	vertex->y = y;
	vertex->z = z;

	// if not vertices list yet created
	// delete the vertices list
	if(!this->vertices)
	{
		this->vertices = __NEW(VirtualList);
	}

	// add vertex to the end of the list
	VirtualList_pushBack(this->vertices, vertex);
}

// draw polygon to screen
void Polygon_draw(Polygon this, int calculateParallax)
{
	int palette = 0x03;

	// if I have some vertex, draw them
	if(this->vertices && 2 < VirtualList_getSize(this->vertices))
	{
		// the node's which hold the vertices
		VirtualNode fromNode = this->vertices->head;
		VirtualNode toNode = fromNode->next;

		// 3d vertices
		VBVec3D fromVertex3D = {0, 0, 0};
		VBVec3D toVertex3D = {0, 0, 0};

		// 2d vertices
		VBVec2D fromVertex2D = {0, 0, 0};
		VBVec2D toVertex2D = {0, 0, 0};

		// draw the lines
		for(; toNode ; fromNode = fromNode->next, toNode = toNode->next)
		{
			// normalize vertex to screen coordinates
			fromVertex3D = *((VBVec3D*)fromNode->data);
			toVertex3D = *((VBVec3D*)toNode->data);
			__OPTICS_NORMALIZE(fromVertex3D);
			__OPTICS_NORMALIZE(toVertex3D);

			// project to 2d coordinates
			__OPTICS_PROJECT_TO_2D(fromVertex3D, fromVertex2D);
			__OPTICS_PROJECT_TO_2D(toVertex3D, toVertex2D);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertex2D.parallax = Optics_calculateParallax(fromVertex3D.x, fromVertex3D.z);
				toVertex2D.parallax = Optics_calculateParallax(toVertex3D.x, toVertex3D.z);
			}

			// draw the line in both buffers
			DirectDraw_lineFast(DirectDraw_getInstance(), fromVertex2D, toVertex2D, palette);
		}

		if(fromNode && toNode && 2 < VirtualList_getSize(this->vertices))
		{
			fromVertex3D = *((VBVec3D*)fromNode->data);
			toVertex3D = *((VBVec3D*)toNode->data);
			__OPTICS_NORMALIZE(fromVertex3D);
			__OPTICS_NORMALIZE(toVertex3D);

			// project to 2d coordinates
			__OPTICS_PROJECT_TO_2D(fromVertex3D, fromVertex2D);
			__OPTICS_PROJECT_TO_2D(toVertex3D, toVertex2D);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertex2D.parallax = Optics_calculateParallax(fromVertex3D.x, fromVertex3D.z);
				toVertex2D.parallax = Optics_calculateParallax(toVertex3D.x, toVertex3D.z);
			}

			// draw the line in both buffers
			DirectDraw_lineFast(DirectDraw_getInstance(), fromVertex2D, toVertex2D, palette);
		}
	}
	//VIP_REGS[__XPCTRL] = VIP_REGS[__XPSTTS] & ~XPEN;
}
