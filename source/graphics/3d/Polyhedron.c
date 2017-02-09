/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Polyhedron.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VirtualList.h>
#include <PolyhedronManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Polyhedron
 * @extends Object
 */
__CLASS_DEFINITION(Polyhedron, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;

static void Polyhedron_constructor(Polyhedron this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(Polyhedron)
__CLASS_NEW_END(Polyhedron);

// class's constructor
static void Polyhedron_constructor(Polyhedron this)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	// don't create the list yet
	this->vertices = NULL;
}

// class's destructor
void Polyhedron_destructor(Polyhedron this)
{
	ASSERT(this, "Polyhedron::destructor: null this");

	Polyhedron_hide(this);

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
void Polyhedron_addVertex(Polyhedron this, fix19_13 x, fix19_13 y, fix19_13 z)
{
	ASSERT(this, "Polyhedron::addVertex: null this");

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

void Polyhedron_show(Polyhedron this)
{
	ASSERT(this, "Polyhedron::show: null this");

	PolyhedronManager_register(PolyhedronManager_getInstance(), this);
}

void Polyhedron_hide(Polyhedron this)
{
	ASSERT(this, "Polyhedron::hide: null this");

	PolyhedronManager_remove(PolyhedronManager_getInstance(), this);
}

// draw Polyhedron to screen
void Polyhedron_draw(Polyhedron this, int calculateParallax)
{
	ASSERT(this, "Polyhedron::draw: null this");

	int color = __COLOR_BRIGHT_RED;

	// if I have some vertex, draw them
	if(this->vertices && 2 < VirtualList_getSize(this->vertices))
	{
		// the nodes which hold the vertices
		VirtualNode fromNode = this->vertices->head;
		VirtualNode toNode = fromNode->next;

		// 3d vertices
		VBVec3D fromVertex3D = {0, 0, 0};
		VBVec3D toVertex3D = {0, 0, 0};

		// 2d vertices
		VBVec2D fromVertex2D = {0, 0, 0, 0};
		VBVec2D toVertex2D = {0, 0, 0, 0};

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
			DirectDraw_drawLine(DirectDraw_getInstance(), fromVertex2D, toVertex2D, color);
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
			DirectDraw_drawLine(DirectDraw_getInstance(), fromVertex2D, toVertex2D, color);
		}
	}
}
