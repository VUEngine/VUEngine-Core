/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MUTATOR_H_
#define MUTATOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Component.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A Mutator Spec
/// @memberof Mutator
typedef struct MutatorSpec
{
	/// Component spec
	ComponentSpec componentSpec;

	/// Mutation target class
	void* (*targetClass)();

	/// Enabled?
	bool enabled;

} MutatorSpec;

/// A Mutator spec that is stored in ROM
/// @memberof Mutator
typedef const MutatorSpec MutatorROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Mutator
///
/// Inherits from Component
///
/// Right now, this is only used by the Vehicle class in the plugins.
/// Eventually, this will serve to avoid the need to inherit from Actor.
class Mutator : Component
{
	/// @protectedsection

	/// Flag to allow or prohibit the mutator to perform its operations
	bool enabled;

	/// @publicsection

	/// Class' constructor
	/// @param owner: Entity to which the mutator attaches to
	/// @param mutatorSpec: Specification that determines how to configure the mutator
	void constructor(Entity owner, const MutatorSpec* mutatorSpec);

	/// Enable the mutator's operations.
	void enable();

	/// Disable the mutator's operations.
	void disable();

	/// Check if the mutator's operations are enabled.
	/// @return True if the mutator's operations are enabled; false otherwise
	bool isEnabled();
}

#endif
