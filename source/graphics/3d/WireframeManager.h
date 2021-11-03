/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef WIREFRAME_MANAGER_H_
#define WIREFRAME_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Wireframe.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-3d

singleton class WireframeManager : Object
{
	// Wireframes
	VirtualList wireframes;

	/// @publicsection
	static WireframeManager getInstance();
	void print(int32 x, int32 y);
	void register(Wireframe wireframe);
	void remove(Wireframe wireframe);
	void reset();
}


#endif
