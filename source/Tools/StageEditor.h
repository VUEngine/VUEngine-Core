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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Entity.h>
#include <Tool.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A struct to map an entity spec to a name
/// @memberof StageEditor
typedef struct UserObject
{
	/// Specification for an entity
	EntitySpec* entitySpec;

	/// Entity spec's name
	char* name;

} UserObject;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class SoundTest
///
/// Inherits from Tool
///
/// Implements a tool that permits to manipulate a stage's entities.
singleton class StageEditor : Tool
{
	/// Node of the stage's selected entity
	VirtualNode entityNode;

	/// Current entity's wireframe
	Wireframe wireframe;

	/// Sprite to display new entities to be added to the stage
	Sprite userEntitySprite;

	// Selector of user defined entities
	OptionsSelector userEntitySelector;

	/// Editor's state
	int32 state;

	// Translation step size
	int32 translationStepSize;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return StageEditor singleton
	static StageEditor getInstance();

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
