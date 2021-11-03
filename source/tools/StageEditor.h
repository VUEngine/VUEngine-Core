/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STAGE_EDITOR_H_
#define STAGE_EDITOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Tool.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * For level editing
 *
 * @memberof 		StageEditor
 */
typedef struct UserObject
{
	/// Pointer to EntitySpec
	EntitySpec* entitySpec;
	/// Name of the Entity
	char* name;

} UserObject;


//---------------------------------------------------------------------------------------------------------
//											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

/// In-game stage editor for debug and productivity purposes
/// @ingroup tools
singleton class StageEditor : Tool
{
	// Current in game entity
	VirtualNode currentEntityNode;
	// Current entity's shape
	Shape shape;
	// Mode
	int32 mode;
	// Actors selector
	OptionsSelector userObjectsSelector;
	// Translation step size
	int32 translationStepSize;
	// Current user's object's sprite
	Sprite userObjectSprite;

	/// @publicsection
	static StageEditor getInstance();
	override void show();
	override void hide();
	override void update();
	override void processUserInput(uint16 pressedKey);
}

#endif
