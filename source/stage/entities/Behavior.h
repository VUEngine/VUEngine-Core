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

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Container;

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
abstract class Behavior : Object
{
	bool enabled;

	void constructor(const BehaviorSpec* behaviorSpec);

	bool isEnabled();
	void setEnabled(bool value);

	static Behavior create(const BehaviorSpec* behaviorSpec);

	virtual void start(Container owner);
	virtual void update(Container owner, uint32 elapsedTime);
	virtual void pause(Container owner);
	virtual void resume(Container owner);
}


#endif
