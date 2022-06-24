/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef POLYHEDRON_H_
#define POLYHEDRON_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct PolyhedronSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

} PolyhedronSpec;

/// @ingroup graphics-3d
class Polyhedron : Wireframe
{
	// Vertices
	VirtualList vertices;

	/// @publicsection
	void constructor(PolyhedronSpec* polyhedronSpec);
	void addVertex(fix10_6 x, fix10_6 y, fix10_6 z);
	override void draw(bool calculateParallax);
}


#endif
