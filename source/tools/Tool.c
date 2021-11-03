/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Tool.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Tool::getInstance()
 * @memberof	Tool
 * @public
 * @return		Tool instance
 */


/**
 * Class constructor
 *
 * @private
 */
void Tool::constructor()
{
	Base::constructor();

	this->gameState = NULL;
}

/**
 * Class destructor
 */
void Tool::destructor()
{
	// allow a new construct
	Base::destructor();
}

void Tool::render()
{
}

void Tool::setGameState(GameState gameState)
{
	this->gameState = gameState;
}

void Tool::dimmGame()
{
	// Doens't work that well
	/*
	_vipRegisters[__GPLT0] = 0x50;
	_vipRegisters[__GPLT1] = 0x50;
	_vipRegisters[__GPLT2] = 0x50;
	_vipRegisters[__GPLT3] = 0x50;
	_vipRegisters[__JPLT0] = 0x50;
	_vipRegisters[__JPLT1] = 0x50;
	_vipRegisters[__JPLT2] = 0x50;
	_vipRegisters[__JPLT3] = 0x50;

	_vipRegisters[0x30 | __PRINTING_PALETTE] = 0x90;
	*/
}

void Tool::lightUpGame()
{
	Stage::setupPalettes(GameState::getStage(this->gameState));
}

#ifndef __TOOLS

void SoundTest::setVTable(){}
void StageEditor::setVTable(){}
void AnimationInspector::setVTable(){}
void Debug::setVTable(){}
void DebugState::setVTable(){}
void SoundTestState::setVTable(){}
void AnimationInspectorState::setVTable(){}
void StageEditorState::setVTable(){}

#endif

#ifndef __ENABLE_PROFILER
void Profiler::setVTable(){}
#endif

