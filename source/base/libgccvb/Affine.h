/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef AFFINE_H_
#define AFFINE_H_


//---------------------------------------------------------------------------------------------------------
//											INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>
#include <Math.h>
#include <MiscStructs.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Affine : Object
{
	/// @publicsection
	static int16 applyAll(uint32 param, int16 paramTableRow, fix10_6 x, fix10_6 y, fix13_3 mx, fix13_3 my, fix10_6 halfWidth, fix10_6 halfHeight, const Scale* scale, const Rotation* rotation);
	static int16 rotate(uint32 param, int16 paramTableRow, fix10_6 x, fix10_6 y, fix13_3 mx, fix13_3 my, fix10_6 halfWidth, fix10_6 halfHeight, const Rotation* rotation)
}


#endif
