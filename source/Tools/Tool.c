/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printing.h>

#include "Tool.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

// This is a hack to prevent linker errors when the tools are not enabled.

#ifndef __TOOLS

#ifndef __SOUND_TEST
void SoundTest::setVTable(){}
#endif

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


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stage = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::destructor()
{
	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::setStage(Stage stage)
{
	this->stage = stage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::lightUpGame()
{
	if(!isDeleted(this->stage))
	{
		Stage::configurePalettes(this->stage);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

