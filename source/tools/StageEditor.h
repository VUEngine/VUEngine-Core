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
//											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods

/**
 * For level editing
 *
 * @memberof 		StageEditor
 */
typedef struct UserObject
{
	/// Pointer to EntityDefinition
	EntityDefinition* entityDefinition;
	/// Name of the Entity
	char* name;

} UserObject;


singleton class StageEditor : Object
{
	/**
	 * @var GameState		gameState
	 * @brief				Current game state
	 * @memberof			StageEditor
	 */
	GameState gameState;
	/**
	 * @var VirtualNode		currentEntityNode
	 * @brief				Current in game entity
	 * @memberof			StageEditor
	 */
	VirtualNode currentEntityNode;
	/**
	 * @var Shape			shape
	 * @brief				Current entity's shape
	 * @memberof			StageEditor
	 */
	Shape shape;
	/**
	 * @var int				mode
	 * @brief				Mode
	 * @memberof			StageEditor
	 */
	int mode;
	/**
	 * @var OptionsSelector userObjectsSelector
	 * @brief				Actors selector
	 * @memberof			StageEditor
	 */
	OptionsSelector userObjectsSelector;
	/**
	 * @var int				translationStepSize
	 * @brief				Translation step size
	 * @memberof			StageEditor
	 */
	int translationStepSize;
	/**
	 * @var Sprite			userObjectSprite
	 * @brief				Current user's object's sprite
	 * @memberof			StageEditor
	 */
	Sprite userObjectSprite;

	static StageEditor getInstance();
	void show(StageEditor this, GameState gameState);
	void hide(StageEditor this);
	void update(StageEditor this);
	void processUserInput(StageEditor this, u16 pressedKey);
}


#endif
