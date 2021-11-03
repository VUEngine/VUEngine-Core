/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_INSPECTOR_STATE_H_
#define ANIMATION_INSPECTOR_STATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ToolState.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup states
singleton class AnimationInspectorState : ToolState
{
	/// @publicsection
	static AnimationInspectorState getInstance();

	override bool isKeyCombination(UserInput userInput);
}

#endif
