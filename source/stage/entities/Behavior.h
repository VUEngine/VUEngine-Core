/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BEHAVIOR_H_
#define BEHAVIOR_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Component.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A Behavior Spec
/// @memberof Behavior
typedef struct BehaviorSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// enabled
	bool enabled;

} BehaviorSpec;

/// A Behavior spec that is stored in ROM
/// @memberof Behavior
typedef const BehaviorSpec BehaviorROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Behavior
///
/// Inherits from Component
///
/// Implements a container whose children are always fixed to the camera
abstract class Behavior : Component
{
	/// @protectedsection

	/// Flag to allow or prohibit the behavior to perform its operations
	bool enabled;

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the behavior attaches to
	/// @param behaviorSpec: Specification that determines how to configure the behavior
	void constructor(SpatialObject owner, const BehaviorSpec* behaviorSpec);

	/// Enable the behavior's operations.
	void enable();

	/// Disable the behavior's operations.
	void disable();

	/// Check if the behavior's operations are enabled.
	/// @return True if the behavior's operations are enabled; false otherwise
	bool isEnabled();
}


#endif
