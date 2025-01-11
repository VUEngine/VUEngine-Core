/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STAGE_EDITOR_H_
#define STAGE_EDITOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Actor.h>
#include <Tool.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A struct to map an actor spec to a name
/// @memberof StageEditor
typedef struct UserObject
{
	/// Specification for an actor
	ActorSpec* actorSpec;

	/// Actor spec's name
	char* name;

} UserObject;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SoundTest
///
/// Inherits from Tool
///
/// Implements a tool that permits to manipulate a stage's actors.
singleton class StageEditor : Tool
{
	/// Node of the stage's selected actor
	VirtualNode actorNode;

	/// Current actor's wireframe
	Wireframe wireframe;

	/// Sprite to display new actors to be added to the stage
	Sprite userActorSprite;

	// Selector of user defined actors
	OptionsSelector userActorSelector;

	/// Editor's state
	int32 state;

	// Translation step size
	int32 translationStepSize;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return StageEditor singleton
	static StageEditor getInstance(ClassPointer requesterClass);

	/// Update the tool's state.
	override void update();

	/// Show the tool.
	override void show();

	/// Hide the tool.
	override void hide();

	/// Process the provided user pressed key.
	/// @param pressedKey: User pressed key
	override void processUserInput(uint16 pressedKey);
}

#endif
