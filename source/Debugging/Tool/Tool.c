/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printer.h>
#include <ToolState.h>

#include "Tool.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->toolState = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::dimmGame()
{
	PaletteConfig paletteConfig =
	{
		{0x50, 0x50, 0x50, 0x50},
		{0x50, 0x50, 0x50, 0x50}
	};

	if(4 > (unsigned)__PRINTING_PALETTE)
	{
		((uint8*)&paletteConfig.bgmap)[__PRINTING_PALETTE] = 0x90;
	}

	VIPManager::configurePalettes(&paletteConfig);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::lightUpGame()
{
	if(!isDeleted(ToolState::getCurrentStage(this->toolState)))
	{
		Stage::configurePalettes(ToolState::getCurrentStage(this->toolState));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Tool::setToolState(ToolState toolState)
{
	this->toolState = toolState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
