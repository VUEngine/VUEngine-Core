/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TOOL_H_
#define TOOL_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup tools
abstract class Tool : Object
{
	GameState gameState;

	/// @publicsection
	void constructor();

	static Tool getInstance();

	void setGameState(GameState gameState);

	virtual void update() = 0;
	virtual void render();
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void processUserInput(uint16 pressedKey) = 0;
	virtual void dimmGame();
	virtual void lightUpGame();
}

#endif
