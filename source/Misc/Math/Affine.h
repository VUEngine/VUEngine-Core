/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef AFFINE_H_
#define AFFINE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Affine
///
/// Inherits from Object
///
/// Implements various affine transformations.
static class Affine : Object
{
	/// @publicsection

	/// Translate, scale and rotate the affine matrix specified by param.
	/// @param maximumParamTableRowsToComputePerCall: Used to defer param table computations 
	/// (-1 to compute the whole table)
	/// @param param: Displacement within param tables space
	/// @param paramTableRow: Displacement within the specified param table 
	/// @param targetHalfWidth: Image's target half width
	/// @param targetHalfHeight: Image's target half height
	/// @param mx: Image's x coordinate
	/// @param my: Image's y coordinate
	/// @param halfWidth: Image's half width
	/// @param halfHeight: Image's half height
	/// @param rotation: Target rotation
	static int16 transform
	(
		int32 maximumParamTableRowsToComputePerCall, uint32 param, int16 paramTableRow, fixed_t targetHalfWidth, 
		fixed_t targetHalfHeight, fix13_3 mx, fix13_3 my, fixed_t halfWidth, fixed_t halfHeight, const Rotation* rotation
	);

	/// Rotate the affine matrix specified by param.
	/// @param maximumParamTableRowsToComputePerCall: Used to defer param table computations 
	/// (-1 to compute the whole table)
	/// @param param: Displacement within param tables space
	/// @param paramTableRow: Displacement within the specified param table 
	/// @param targetHalfWidth: Image's target half width
	/// @param targetHalfHeight: Image's target half height
	/// @param mx: Image's x coordinate
	/// @param my: Image's y coordinate
	/// @param halfWidth: Image's half width
	/// @param halfHeight: Image's half height
	/// @param rotation: Target rotation
	static int16 rotate
	(
		int32 maximumParamTableRowsToComputePerCall, uint32 param, int16 paramTableRow, fixed_t targetHalfWidth, 
		fixed_t targetHalfHeight, fix13_3 mx, fix13_3 my, fixed_t halfWidth, fixed_t halfHeight, const Rotation* rotation
	);	
}

#endif
