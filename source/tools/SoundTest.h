/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_TEST_H_
#define SOUND_TEST_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Tool.h>
#include <SoundWrapper.h>
#include <SoundManager.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup tools
singleton class SoundTest : Tool
{
	SoundWrapper soundWrapper;
	uint16 selectedSound;

	/// @publicsection
	static SoundTest getInstance();
	void printVolumeState();
	void printPlaybackPosition(uint32 elapsedMilliseconds);
	override void update();
	override void show();
	override void hide();
	override void processUserInput(uint16 pressedKey);
}

#endif
