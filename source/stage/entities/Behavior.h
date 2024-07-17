/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BEHAVIOR_H_
#define BEHAVIOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Component.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Entity;

// defines an entity in ROM memory
typedef struct BehaviorSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// enabled
	bool enabled;

} BehaviorSpec;

typedef const BehaviorSpec BehaviorROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
abstract class Behavior : Component
{
	bool enabled;

	static Behavior create(SpatialObject owner, const BehaviorSpec* behaviorSpec);

	void constructor(SpatialObject owner, const BehaviorSpec* behaviorSpec);

	bool isEnabled();
	void setEnabled(bool value);

}


#endif
