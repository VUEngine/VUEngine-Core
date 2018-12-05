/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef STAGE_EDITOR_H_
#define STAGE_EDITOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Entity.h>
#include <GameState.h>


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
singleton class StageEditor : Object
{
	// Current game state
	GameState gameState;
	// Current in game entity
	VirtualNode currentEntityNode;
	// Current entity's shape
	Shape shape;
	// Mode
	int mode;
	// Actors selector
	OptionsSelector userObjectsSelector;
	// Translation step size
	int translationStepSize;
	// Current user's object's sprite
	Sprite userObjectSprite;

	/// @publicsection
	static StageEditor getInstance();
	void show(GameState gameState);
	void hide();
	void update();
	void processUserInput(u16 pressedKey);
}


#endif
