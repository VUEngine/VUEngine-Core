/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifdef __TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Debug.h>
#include <Game.h>
#include <Optics.h>
#include <FrameRate.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <CharSetManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <HardwareManager.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <ObjectTexture.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <MiscStructs.h>
#include <MBgmapSprite.h>
#include <KeypadManager.h>
#include <SRAMManager.h>
//#include <I18n.h>
#include <Camera.h>
#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
#include <TimerManager.h>

#include <Clock.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <CharSet.h>
#include <Sprite.h>
#include <Texture.h>

#include <Body.h>
#include <Ball.h>
#include <Box.h>
#include <InverseBox.h>
#include <LineField.h>
#include <Shape.h>
#include <Wireframe.h>
#include <Sphere.h>
#include <Polyhedron.h>

#include <Container.h>
#include <Entity.h>
#include <Entity.h>
#include <AnimatedEntity.h>
#include <AnimationCoordinatorFactory.h>
#include <Entity.h>
#include <Actor.h>
#include <Entity.h>
#include <Particle.h>
#include <ParticleBody.h>
#include <ParticleSystem.h>
#include <SolidParticle.h>

#include <GameState.h>
#include <Stage.h>
#include <UIContainer.h>
#include <Mem.h>

#include <SoundWrapper.h>

#include <DebugState.h>
#include <DebugState.h>
#include <GameState.h>
#include <StageEditorState.h>

#include <Debug.h>
#include <Debug.h>
#include <OptionsSelector.h>
#include <StageEditor.h>

#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __CHARS_PER_SEGMENT_TO_SHOW		512
#define __CHARS_PER_ROW_TO_SHOW			32


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

#ifdef __DEBUG_TOOLS
extern ClassSizeData _userClassesSizeData[];
#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Debug::getInstance()
 * @memberof	Debug
 * @public
 * @return		Debug instance
 */


/**
 * Class constructor
 *
 * @private
 */
void Debug::constructor()
{
	Base::constructor();

	this->pages = new VirtualList();
	this->subPages = new VirtualList();
	this->currentPage = NULL;
	this->currentSubPage = NULL;

	this->gameState = NULL;

	this->currentSprite = -1;
	this->bgmapSegment = 0;
	this->objectSegment = 0;
	this->charSegment = 0;
	this->sramPage = 0;

	this->update = NULL;

	this->viewedMapPart = 0;

	Debug::setupPages(this);
}

/**
 * Class destructor
 */
void Debug::destructor()
{
	delete this->pages;
	delete this->subPages;

	// allow a new construct
	Base::destructor();
}

// setup pages
void Debug::setupPages()
{
	VirtualList::pushBack(this->pages, &Debug_generalStatusPage);
	VirtualList::pushBack(this->pages, &Debug_memoryStatusPage);
	VirtualList::pushBack(this->pages, &Debug_gameProfilingPage);
	VirtualList::pushBack(this->pages, &Debug_streamingPage);
	VirtualList::pushBack(this->pages, &Debug_spritesPage);
	VirtualList::pushBack(this->pages, &Debug_texturesPage);
	VirtualList::pushBack(this->pages, &Debug_objectsPage);
	VirtualList::pushBack(this->pages, &Debug_charMemoryPage);
	VirtualList::pushBack(this->pages, &Debug_physicsPage);
	VirtualList::pushBack(this->pages, &Debug_hardwareRegistersPage);
	VirtualList::pushBack(this->pages, &Debug_sramPage);

	this->currentPage = this->pages->head;
}

/**
 * Update
 */
void Debug::update()
{
	if(this->update)
	{
		((void (*)(Debug))this->update)(this);
	}
}

/**
 * Render
 */
void Debug::render()
{
	if(this->currentPage->data == &Debug_texturesPage && 0 <= this->bgmapSegment)
	{
		Debug::showBgmapSegment(this);
	}
}

/**
 * Show debugging screens
 *
 * @param gameState Current game state
 */
void Debug::show()
{
	Printing::clear(Printing::getInstance());
	Printing::setCoordinates(Printing::getInstance(), 0, 0, -64, -2);
	SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);
	SpriteManager::computeTotalPixelsDrawn(SpriteManager::getInstance());

	Debug::showPage(this, 0);
}

/**
 * Hide debugging screens
 */
void Debug::hide()
{
	CollisionManager::hideShapes(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))));
	Printing::clear(Printing::getInstance());
	SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);
}

/**
 * Get the position in the pages list of the current page
 *
 * @return 			Current page's node's position
 */
u8 Debug::getCurrentPageNumber()
{
	return VirtualList::getNodePosition(this->pages, this->currentPage) + 1;
}

void Debug::setBlackBackground()
{
	SpriteManager::hideSprites(SpriteManager::getInstance(), NULL, false);
}

/**
 * Process user input
 *
 * @param pressedKey	User input
 */
void Debug::processUserInput(u16 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Debug::showPreviousPage(this);
	}
	else if(pressedKey & K_LR)
	{
		Debug::showNextPage(this);
	}
	else if(pressedKey & K_LU)
	{
		Debug::showPreviousSubPage(this);
	}
	else if(pressedKey & K_LD)
	{
		Debug::showNextSubPage(this);
	}
	else if(pressedKey & K_RL)
	{
		Debug::displaceLeft(this);
	}
	else if(pressedKey & K_RR)
	{
		Debug::displaceRight(this);
	}
	else if(pressedKey & K_RU)
	{
		Debug::displaceUp(this);
	}
	else if(pressedKey & K_RD)
	{
		Debug::displaceDown(this);
	}
}

/**
 * Show previous debugging page
 */
void Debug::showPreviousPage()
{
	SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);

	this->currentPage = VirtualNode::getPrevious(this->currentPage);

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->tail;
	}

	Debug::showPage(this, -1);
}

/**
 * Show next debugging page
 */
void Debug::showNextPage()
{
	SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);

	this->currentPage = this->currentPage->next;

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->head;
	}

	Debug::showPage(this, 1);
}

/**
 * Show previous debugging sub-page
 */
void Debug::showPreviousSubPage()
{
	if(!this->currentSubPage)
	{
		return;
	}

	this->currentSubPage = VirtualNode::getPrevious(this->currentSubPage);

	if(NULL == this->currentSubPage)
	{
		this->currentSubPage = this->subPages->tail;
	}

	Debug::showSubPage(this, -1);
}

/**
 * Show next debugging sub-page
 */
void Debug::showNextSubPage()
{
	if(!this->currentSubPage)
	{
		return;
	}

	this->currentSubPage = this->currentSubPage->next;

	if(NULL == this->currentSubPage)
	{
		this->currentSubPage = this->subPages->head;
	}

	Debug::showSubPage(this, 1);
}

/**
 * Print header
 *
 * @private
 */
void Debug::printHeader()
{
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(Printing::getInstance(), " DEBUG SYSTEM ", 1, 0, NULL);
	Printing::text(Printing::getInstance(), "   /   ", 16, 0, NULL);
	Printing::int(Printing::getInstance(), Debug::getCurrentPageNumber(this), Debug::getCurrentPageNumber(this) < 10 ? 18 : 17, 0, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->pages), 20, 0, NULL);
}

/**
 * Show current debugging page
 *
 * @param increment		Increment
 */
void Debug::showPage(int increment)
{
	if(this->currentPage && this->currentPage->data)
	{
		this->update = NULL;

		Printing::clear(Printing::getInstance());
		SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);

		Debug::printHeader(this);
		Printing::text(Printing::getInstance(), " \x1E\x1C\x1D ", 42, 0, NULL);

		Debug::setBlackBackground(this);

		CollisionManager::hideShapes(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))));

		((void (*)(Debug, int, int, int))this->currentPage->data)(this, increment, 1, 2);
	}

	Printing::show(Printing::getInstance());
}

/**
 * Show current debugging sub-page
 *
 * @param increment		Increment
 */
void Debug::showSubPage(int increment)
{
	if(this->currentSubPage && VirtualNode::getData(this->currentSubPage))
	{
		this->update = NULL;

		Printing::clear(Printing::getInstance());

		Debug::printHeader(this);
		Printing::text(Printing::getInstance(), " \x1E\x1A\x1B\x1C\x1D ", 40, 0, NULL);

		((void (*)(Debug, int, int, int))VirtualNode::getData(this->currentSubPage))(this, increment, 1, 2);
	}

	Printing::show(Printing::getInstance());
}

/**
 * Displace window to the left
 */
void Debug::displaceLeft()
{
	if(this->viewedMapPart % 2 == 1)
	{
		this->viewedMapPart--;
	}
	Debug::showDebugBgmap(this);
}

/**
 * Displace window to the right
 */
void Debug::displaceRight()
{
	if(this->viewedMapPart % 2 == 0)
	{
		this->viewedMapPart++;
	}
	Debug::showDebugBgmap(this);
}

/**
 * Displace window up
 */
void Debug::displaceUp()
{
	if(this->viewedMapPart > 1)
	{
		this->viewedMapPart -= 2;
	}
	Debug::showDebugBgmap(this);
}

/**
 * Displace window down
 */
void Debug::displaceDown()
{
	if(this->viewedMapPart < 4)
	{
		this->viewedMapPart += 2;
	}
	Debug::showDebugBgmap(this);
}

/**
 * Remove sub-pages
 *
 * @private
 */
void Debug::removeSubPages()
{
	VirtualList::clear(this->subPages);
	this->currentSubPage = NULL;
}

/**
 * Setup Game's general status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::generalStatusPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug::removeSubPages(this);

	Printing::text(Printing::getInstance(), "GAME STATUS", 1, y++, NULL);
	Printing::text(Printing::getInstance(), "Current State:", 1, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this->gameState), 20, y, NULL);

	Printing::text(Printing::getInstance(), "Save Data Manager:", 1, ++y, NULL);
	if(Game::getSaveDataManager(Game::getInstance()))
	{
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(Game::getSaveDataManager(Game::getInstance())), 20, y, NULL);
	}
	else
	{
		Printing::text(Printing::getInstance(), "none", 20, y, NULL);
	}
/*
	Printing::text(Printing::getInstance(), "Active Language:", 1, ++y, NULL);
	Printing::text(Printing::getInstance(), I18n::getActiveLanguageName(I18n::getInstance()), 20, y, NULL);
*/
	y += 3;

	Printing::text(Printing::getInstance(), "CLOCKS STATUS", 1, y++, NULL);
	Printing::text(Printing::getInstance(), "General clock time: ", 1, ++y, NULL);
	Clock::print(Game::getClock(Game::getInstance()), 26, y, NULL);
	Printing::text(Printing::getInstance(), "In game clock's time: ", 1, ++y, NULL);
	Clock::print(GameState::getMessagingClock(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), 26, y, NULL);
	Printing::text(Printing::getInstance(), "Animations clock's time: ", 1, ++y, NULL);
	Clock::print(GameState::getUpdateClock(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), 26, y, NULL);
	Printing::text(Printing::getInstance(), "Physics clock's time: ", 1, ++y, NULL);
	Clock::print(GameState::getPhysicsClock(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), 26, y, NULL);
	y+=3;
}

/**
 * Setup Memory Pool's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

#ifdef __DEBUG_TOOLS

	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowZeroPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowFirstPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowSecondPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowThirdPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowFourthPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowFifthPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowSixthPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowSeventhPage);
	VirtualList::pushBack(this->subPages, &Debug_memoryStatusShowUserDefinedClassesSizes);

#endif

	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

#ifdef __DEBUG_TOOLS

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowZeroPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Game_getObjectSize, 					"Game"},
		{&DirectDraw_getObjectSize, 			"DirectDraw"},
		{&Error_getObjectSize, 					"Error"},
		{&FrameRate_getObjectSize, 				"FrameRate"},
		//{&I18n_getObjectSize, 					"I18n"},
		{&MemoryPool_getObjectSize, 			"MemoryPool"},
		{&MessageDispatcher_getObjectSize, 		"MessageDispatcher"},
		{&Printing_getObjectSize, 				"Printing"},
		{&Camera_getObjectSize, 				"Camera"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowFirstPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&BgmapTextureManager_getObjectSize, 			"BgmapTextureManager"},
		{&CharSetManager_getObjectSize, 				"CharSetManager"},
		{&ClockManager_getObjectSize, 					"ClockManager"},
		{&CollisionManager_getObjectSize, 				"CollisionManager"},
		{&HardwareManager_getObjectSize, 				"HardwareManager"},
		{&KeypadManager_getObjectSize, 					"KeypadManager"},
		{&ParamTableManager_getObjectSize, 				"ParamTableManager"},
		{&CameraEffectManager_getObjectSize, 			"CameraEff.Manager"},
		{&CameraMovementManager_getObjectSize, 			"CameraMov.Manager"},
		{&SpriteManager_getObjectSize, 					"SpriteManager"},
		{&SoundManager_getObjectSize, 					"SoundManager"},
		{&SRAMManager_getObjectSize, 					"SRAMManager"},
		{&TimerManager_getObjectSize, 					"TimerManager"},
		{&VIPManager_getObjectSize, 					"VIPManager"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowSecondPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Clock_getObjectSize, 			"Clock"},
		{&Object_getObjectSize, 		"Object"},
		{&State_getObjectSize, 			"State"},
		{&StateMachine_getObjectSize, 	"StateMachine"},
		{&Telegram_getObjectSize, 		"Telegram"},
		{&VirtualList_getObjectSize, 	"VirtualList"},
		{&VirtualNode_getObjectSize, 	"VirtualNode"},
		{&SoundWrapper_getObjectSize, 	"SoundWrapper"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowThirdPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&AnimationController_getObjectSize, 			"AnimationController"},
		{&AnimationCoordinator_getObjectSize, 			"AnimationCoordinat."},
		{&AnimationCoordinatorFactory_getObjectSize,	"AnimationCoor.Fact."},
		{&BgmapAnimatedSprite_getObjectSize,			"BgmapAnim. Sprite"},
		{&BgmapSprite_getObjectSize, 					"BgmapSprite"},
		{&BgmapTexture_getObjectSize, 					"BgmapTexture"},
		{&CharSet_getObjectSize, 						"CharSet"},
		{&MBgmapSprite_getObjectSize, 					"MBgmapSprite"},
		{&ObjectAnimatedSprite_getObjectSize,			"ObjectAnim. Sprite"},
		{&ObjectSprite_getObjectSize,					"ObjectSprite"},
		{&ObjectSpriteContainer_getObjectSize,			"ObjectSpriteCont."},
		{&ObjectTexture_getObjectSize,					"ObjectTexture"},
		{&Texture_getObjectSize, 						"Texture"},
		{&Sprite_getObjectSize, 						"Sprite"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowFourthPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&PhysicalWorld_getObjectSize, 	"PhysicalWorld"},
		{&Body_getObjectSize, 			"Body"},
		{&Shape_getObjectSize, 			"Shape"},
		{&Ball_getObjectSize, 			"Ball"},
		{&Box_getObjectSize,			"Box"},
		{&InverseBox_getObjectSize,		"InverseBox"},
		{&LineField_getObjectSize,		"LineField"},
		{&Wireframe_getObjectSize, 		"Wireframe"},
		{&Polyhedron_getObjectSize, 	"Polyhedron"},
		{&Sphere_getObjectSize, 		"Sphere"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowFifthPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Actor_getObjectSize,							"Actor"},
		{&AnimatedEntity_getObjectSize,					"AnimatedEntity"},
		{&Container_getObjectSize,						"Container"},
		{&Entity_getObjectSize,							"Entity"},
		{&EntityFactory_getObjectSize,					"EntityFactory"},
		{&GameState_getObjectSize,						"GameState"},
		{&GameState_getObjectSize,						"Stage"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowSixthPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Particle_getObjectSize,						"Particle"},
		{&ParticleBody_getObjectSize,					"ParticleBody"},
		{&ParticleRemover_getObjectSize,				"ParticleRemover"},
		{&ParticleSystem_getObjectSize,					"ParticleSystem"},
		{&SolidParticle_getObjectSize,					"SolidParticle"},
		{&SpatialObject_getObjectSize,					"SpatialObject"},
		{&Stage_getObjectSize,							"Stage"},
		{&UIContainer_getObjectSize,					"UIContainer"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowSeventhPage(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&DebugState_getObjectSize,			"DebugState"},
		{&GameState_getObjectSize,			"GameState"},
		{&StageEditorState_getObjectSize,	"StageEditorState"},
		{&Debug_getObjectSize,				"Debug"},
		{&OptionsSelector_getObjectSize,	"OptionsSelector"},
		{&StageEditor_getObjectSize,		"StageEditor"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::memoryStatusShowUserDefinedClassesSizes(int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool::printDetailedUsage(MemoryPool::getInstance(), x, y);
	Debug::printClassSizes(this, _userClassesSizeData, 0, x + 21, y, "User defined classes:");
}

/**
 * Print classes' sizes
 *
 * @private
 * @param classesSizeData		Array with a class names and their sizes
 * @param count					Number of entries to print
 * @param x						Camera's x coordinate
 * @param y						Camera's y coordinate
 * @param message				Message to add to the output
 */
void Debug::printClassSizes(ClassSizeData* classesSizeData, int count, int x, int y, char* message)
{
	int columnIncrement = 20;

	Printing::text(Printing::getInstance(), "CLASSES MEMORY USAGE (B) ", x, y++, NULL);

	if(message)
	{
		Printing::text(Printing::getInstance(), message, x, ++y, NULL);
		y++;
	}

	Printing::text(Printing::getInstance(), "Name                Size", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, ++y, NULL);

	int i = 0;
	for(; classesSizeData[i].classSizeFunction && (0 == count || i < count); i++)
	{
		Printing::text(Printing::getInstance(), classesSizeData[i].name, x, ++y, NULL);
		Printing::int(Printing::getInstance(), ((int (*)(void))classesSizeData[i].classSizeFunction)(), x + columnIncrement, y, NULL);
	}
}
#endif

/**
 * Show Game's profiling
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::gameProfilingPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	// TODO: show profiler
	PRINT_TEXT("TODO: Show profiler", 1, 6);
}

/**
 * Setup streaming's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::streamingPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_streamingShowStatus);
	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

/**
 * Show State's streaming status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::streamingShowStatus(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Stage::showStreamingProfiling(GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), x, y);
}

/**
 * Setup CHAR memory's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::charMemoryPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_charMemoryShowStatus);
	VirtualList::pushBack(this->subPages, &Debug_charMemoryShowStatus);
	this->currentSubPage = this->subPages->head;

	this->charSegment = -1;

	Debug::showSubPage(this, 0);
}

/**
 * Show CHAR memory's state
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::charMemoryShowStatus(int increment __attribute__ ((unused)), int x, int y)
{
	this->charSegment += increment;

	int charSegments = __CHAR_MEMORY_TOTAL_CHARS / __CHARS_PER_SEGMENT_TO_SHOW;

	if(-1 > this->charSegment)
	{
		this->charSegment = charSegments - 1;
	}

	if(-1 == this->charSegment)
	{
		Debug::setBlackBackground(this);
		CharSetManager::print(CharSetManager::getInstance(), x, y);
	}
	else if(charSegments > this->charSegment)
	{
		Printing::text(Printing::getInstance(), "CHAR MEMORY INSPECTOR", x, y++, NULL);
		Printing::text(Printing::getInstance(), "Segment:  / ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), this->charSegment + 1, x + 9, y, NULL);
		Printing::int(Printing::getInstance(), charSegments, x + 11, y, NULL);
		Printing::text(Printing::getInstance(), "Chars:       -    ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW, x + 9, y, NULL);
		Printing::int(Printing::getInstance(), this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + __CHARS_PER_SEGMENT_TO_SHOW - 1, x + 14, y, NULL);

		Debug::charMemoryShowMemory(this, increment, x, y);
	}
	else
	{
		this->charSegment = -1;
		Debug::setBlackBackground(this);
		CharSetManager::print(CharSetManager::getInstance(), x, y);
	}
}

/**
 * Show CHAR memory segments's state
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::charMemoryShowMemory(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug::setBlackBackground(this);

	int i = 0;
	int yOffset = y + 3;

	// print box
	Printing::text(Printing::getInstance(), "\x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x04", 1, yOffset-1, NULL);
	Printing::text(Printing::getInstance(), "\x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x06", 1, yOffset+16, NULL);

	for(i = 0; i < __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW && i < __SCREEN_HEIGHT / 8; i++)
	{
		Printing::text(Printing::getInstance(), "\x07                                \x07", 1, yOffset+i, NULL);
	}

	const HWORD charMemoryMap[] =
	{
		0,	1,	2,	3,	4,	5,	6,	7,
		8,	9,	10,	11,	12,	13,	14,	15,
		16,	17,	18,	19,	20,	21,	22,	23,
		24,	25,	26,	27,	28,	29,	30,	31
	};

/*
		Mem::copyHWORD((HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (printingBgmap + 1) - __PRINTABLE_BGMAP_AREA) + ((row + topBorder) << 6) + offsetDisplacement]),
				(const HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (this->bgmapSegment)) + ((row + myDisplacement) << 6) + mxDisplacement]), 
				numberOfHWORDS

*/
	u32 printingBgmap = BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());;
	u16* const bgmapSpaceBaseAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	// put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW; i++)
	{
		Mem::addHWORD
		(
			(HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (printingBgmap + 1) - __PRINTABLE_BGMAP_AREA) + ((yOffset + i) << 6) + 2]),
			(HWORD*)charMemoryMap,
			__CHARS_PER_ROW_TO_SHOW,
			this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + i * __CHARS_PER_ROW_TO_SHOW
		);
	}
}

/**
 * Setup BGMAP memory's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::texturesPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_texturesShowStatus);
	VirtualList::pushBack(this->subPages, &Debug_texturesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->bgmapSegment = -1;
	this->viewedMapPart = 0;

	Debug::showSubPage(this, 0);
}

/**
 * Show BGMAP memory's status
 *
 * @private
 */
void Debug::showDebugBgmap()
{
	if(this->currentPage->data != &Debug_texturesPage ||
		0 > this->bgmapSegment
	)
	{
		return;
	}

	Debug::setBlackBackground(this);
	Debug::lightUpGame(this);
	Debug::showBgmapSegment(this);
}

/**
 * Force display BGMAP memory's status
 *
 * @private
 */
void Debug::showBgmapSegment()
{
	u32 printingBgmap = BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());;
	int topBorder = 0;
	int bottomBorder = 0;
	int leftBorder = 0;
	int rightBorder = 0;
	int mxDisplacement = 0;
	int myDisplacement = 0;

	u8 i = 0;
	u8 yOffset = 4;

	// print box
	switch(this->viewedMapPart)
	{
		case 0:
		{
			Printing::text(Printing::getInstance(), " \x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 26, NULL);

        	for(i = yOffset + 1; i < 28; i++)
        	{
        		Printing::text(Printing::getInstance(), " \x07", 0, i, NULL);
        		Printing::text(Printing::getInstance(), " ", 46, i, NULL);
        	}

			topBorder = 5;
			bottomBorder = 0;
			leftBorder = 2;
			rightBorder = 0;
			mxDisplacement = 0;
			myDisplacement = 0;
			break;
		}
		case 1:
		{
			Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x04 ", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 26, NULL);

        	for(i = yOffset + 1; i < 28; i++)
        	{
        		Printing::text(Printing::getInstance(), " ", 1, i, NULL);
        		Printing::text(Printing::getInstance(), "\x07 ", 46, i, NULL);
        	}

			topBorder = 5;
			bottomBorder = 0;
			leftBorder = 0;
			rightBorder = 2;
			mxDisplacement = 64 - (__SCREEN_WIDTH_IN_CHARS - (leftBorder + rightBorder));
			myDisplacement = 0;
			break;
		}
		case 2:
		{
			Printing::text(Printing::getInstance(), "                                                ", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 26, NULL);

        	for(i = yOffset; i < 28; i++)
        	{
        		Printing::text(Printing::getInstance(), " \x07", 0, i, NULL);
        		Printing::text(Printing::getInstance(), " ", 46, i, NULL);
        	}

			topBorder = 4;
			bottomBorder = 0;
			leftBorder = 2;
			rightBorder = 0;
			mxDisplacement = 0;
			myDisplacement = __SCREEN_HEIGHT_IN_CHARS - (topBorder + bottomBorder) - 2;
			break;
		}
		case 3:
		{
			Printing::text(Printing::getInstance(), "                                                ", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 26, NULL);

        	for(i = yOffset; i < 28; i++)
        	{
        		Printing::text(Printing::getInstance(), " ", 1, i, NULL);
        		Printing::text(Printing::getInstance(), "\x07 ", 46, i, NULL);
        	}

			topBorder = 4;
			bottomBorder = 0;
			leftBorder = 0;
			rightBorder = 2;
			mxDisplacement = 64 - (__SCREEN_WIDTH_IN_CHARS - (leftBorder + rightBorder));
			myDisplacement = __SCREEN_HEIGHT_IN_CHARS - (topBorder + bottomBorder) - 2;
			break;
		}
		case 4:
		{
			Printing::text(Printing::getInstance(), "                                                ", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), " \x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 26, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 27, NULL);

        	for(i = yOffset; i < 26; i++)
        	{
        		Printing::text(Printing::getInstance(), " \x07", 0, i, NULL);
        		Printing::text(Printing::getInstance(), " ", 46, i, NULL);
        	}

			topBorder = 4;
			bottomBorder = 2;
			leftBorder = 2;
			rightBorder = 0;
			mxDisplacement = 0;
			myDisplacement = 64 - (__SCREEN_HEIGHT_IN_CHARS - (topBorder + bottomBorder));
			break;
		}
		case 5:
		{
			Printing::text(Printing::getInstance(), "                                                ", 0, yOffset, NULL);
			Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x06 ", 0, 26, NULL);
			Printing::text(Printing::getInstance(), "                                                ", 0, 27, NULL);

        	for(i = yOffset; i < 26; i++)
        	{
        		Printing::text(Printing::getInstance(), " ", 1, i, NULL);
        		Printing::text(Printing::getInstance(), "\x07 ", 46, i, NULL);
        	}

			topBorder = 4;
			bottomBorder = 2;
			leftBorder = 0;
			rightBorder = 2;
			mxDisplacement = 64 - (__SCREEN_WIDTH_IN_CHARS - (leftBorder + rightBorder));
			myDisplacement = 64 - (__SCREEN_HEIGHT_IN_CHARS - (topBorder + bottomBorder));
			break;
		}
	}

	u32 numberOfHWORDS = __SCREEN_WIDTH_IN_CHARS - leftBorder - rightBorder;
	u32 offsetDisplacement = leftBorder;

	u16* const bgmapSpaceBaseAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	for(int row = 0; row < __SCREEN_HEIGHT_IN_CHARS - topBorder - bottomBorder; row++)
	{
		Mem::copyHWORD((HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (printingBgmap + 1) - __PRINTABLE_BGMAP_AREA) + ((row + topBorder) << 6) + offsetDisplacement]),
				(const HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (this->bgmapSegment)) + ((row + myDisplacement) << 6) + mxDisplacement]), 
				numberOfHWORDS
		);
	}
}

/**
 * Setup BGMAP segment's status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::texturesShowStatus(int increment, int x, int y)
{
	this->bgmapSegment += increment;

	if(-1 > this->bgmapSegment)
	{
		this->bgmapSegment = BgmapTextureManager::getAvailableBgmapSegmentsForTextures(BgmapTextureManager::getInstance()) - 1;
	}

	if(-1 == this->bgmapSegment)
	{
		Debug::setBlackBackground(this);
		BgmapTextureManager::print(BgmapTextureManager::getInstance(), x, y);
		ParamTableManager::print(ParamTableManager::getInstance(), x + 27, y);
	}
	else if(BgmapTextureManager::getAvailableBgmapSegmentsForTextures(BgmapTextureManager::getInstance()) > this->bgmapSegment)
	{
		Printing::text(Printing::getInstance(), " \x1E\x1A\x1B\x1C\x1D\x1F\x1A\x1B\x1C\x1D ", 35, 0, NULL);
		Printing::text(Printing::getInstance(), "BGMAP TEXTURES INSPECTOR           Segment: ", x, y, NULL);
		Printing::int(Printing::getInstance(), this->bgmapSegment, x + 44, y, NULL);

		this->viewedMapPart = 0;

		Debug::showDebugBgmap(this);
	}
	else
	{
		this->bgmapSegment = -1;
		Debug::setBlackBackground(this);
		BgmapTextureManager::print(BgmapTextureManager::getInstance(), x, y);
		ParamTableManager::print(ParamTableManager::getInstance(), x + 27, y);
	}
}

/**
 * Setup OBJECT memory's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::objectsPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_objectsShowStatus);
	VirtualList::pushBack(this->subPages, &Debug_objectsShowStatus);
	this->currentSubPage = this->subPages->head;

	this->objectSegment = -1;

	Debug::showSubPage(this, 0);
}

/**
 * Show OBJECT memory's status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::objectsShowStatus(int increment, int x, int y)
{
	this->objectSegment += increment;

	Debug::dimmGame(this);

	if(-1 > this->objectSegment)
	{
		this->objectSegment = __TOTAL_OBJECT_SEGMENTS - 1;
	}

	if(-1 == this->objectSegment)
	{
		Debug::setBlackBackground(this);
		SpriteManager::printObjectSpriteContainersStatus(SpriteManager::getInstance(), x, y);
	}
	else if(__TOTAL_OBJECT_SEGMENTS > this->objectSegment)
	{
		Printing::text(Printing::getInstance(), "OBJECTS INSPECTOR", x, y++, NULL);

		ObjectSpriteContainer objectSpriteContainer = SpriteManager::getObjectSpriteContainerBySegment(SpriteManager::getInstance(), this->objectSegment);

		while(NULL == objectSpriteContainer && (this->objectSegment >= 0 && __TOTAL_OBJECT_SEGMENTS > this->objectSegment))
		{
			objectSpriteContainer = SpriteManager::getObjectSpriteContainerBySegment(SpriteManager::getInstance(), this->objectSegment);

			if(!objectSpriteContainer)
			{
				this->objectSegment += increment;
			}
		}

		if(objectSpriteContainer)
		{
			SpriteManager::hideSprites(SpriteManager::getInstance(), Sprite::safeCast(objectSpriteContainer), false);
			ObjectSpriteContainer::print(objectSpriteContainer, x, ++y);
		}
		else
		{
			this->objectSegment = -1;
			Debug::setBlackBackground(this);
			SpriteManager::printObjectSpriteContainersStatus(SpriteManager::getInstance(), x, y);
		}
	}
	else
	{
		this->objectSegment = -1;
		Debug::setBlackBackground(this);
		SpriteManager::printObjectSpriteContainersStatus(SpriteManager::getInstance(), x, y);
	}
}

/**
 * Setup WORLD memory's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::spritesPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_spritesShowStatus);
	VirtualList::pushBack(this->subPages, &Debug_spritesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->currentSprite = -1;

	Debug::showSubPage(this, 0);
}

/**
 * Show WORLD memory's status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::spritesShowStatus(int increment, int x, int y)
{
	this->currentSprite -= increment;

	Debug::dimmGame(this);

	int numberOfSprites = SpriteManager::getNumberOfSprites(SpriteManager::getInstance());

	if(this->currentSprite > numberOfSprites)
	{
		this->currentSprite = 0;
	}

	if(numberOfSprites == this->currentSprite)
	{
		Debug::setBlackBackground(this);
		SpriteManager::print(SpriteManager::getInstance(), x, y, false);
	}
	else if(0 <= this->currentSprite && this->currentSprite < numberOfSprites)
	{
		Sprite sprite = SpriteManager::getSpriteAtPosition(SpriteManager::getInstance(), this->currentSprite);
		SpriteManager::hideSprites(SpriteManager::getInstance(), sprite, false);
		Sprite::render(sprite, 31, 0);
		Printing::text(Printing::getInstance(), "SPRITES INSPECTOR", x, y++, NULL);
		Sprite::print(sprite, x, ++y);
	}
	else
	{
		this->currentSprite = numberOfSprites;

		Debug::setBlackBackground(this);
		SpriteManager::print(SpriteManager::getInstance(), x, y, false);
	}
}

/**
 * Setup physics' status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::physicsPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_physicStatusShowStatistics);
	VirtualList::pushBack(this->subPages, &Debug_physicStatusShowShapes);
	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

/**
 * Show physics' status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::physicStatusShowStatistics(int increment __attribute__ ((unused)), int x, int y)
{
	PhysicalWorld::print(GameState::getPhysicalWorld(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), x, y);
	CollisionManager::print(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))), x, y + 6);
	CollisionManager::hideShapes(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))));

	Debug::setBlackBackground(this);
}

/**
 * Setup physics sub-pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::physicStatusShowShapes(int increment __attribute__ ((unused)), int x, int y)
{
	Printing::text(Printing::getInstance(), "COLLISION SHAPES", x, y++, NULL);
	this->update = (void (*)(void *))&Debug_showCollisionShapes;

	SpriteManager::showSprites(SpriteManager::getInstance(), NULL, true);
	Debug::dimmGame(this);
}

/**
 * Show collision boxes
 *
 * @private
 */
void Debug::showCollisionShapes()
{
	CollisionManager::showShapes(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))));
}

/**
 * Setup hardware register's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::hardwareRegistersPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug::removeSubPages(this);

	HardwareManager::print(HardwareManager::getInstance(), 1, y);
}

/**
 * Setup SRAM's status pages
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::sramPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug_showSramPage);
	VirtualList::pushBack(this->subPages, &Debug_showSramPage);
	this->currentSubPage = this->subPages->head;

	this->sramPage = 0;

	Debug::showSubPage(this, 0);
}

/**
 * Show SRAM's status
 *
 * @private
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
void Debug::showSramPage(int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	u8 value;
	int i, j, totalPages;
	char word[9];

	totalPages = __TOTAL_SAVE_RAM >> 7;

	extern u32 _sram_bss_end;

	this->sramPage += increment;

	if(this->sramPage < 0)
	{
		this->sramPage = totalPages - 1;
	}
	else if(this->sramPage >= totalPages)
	{
		this->sramPage = 0;
	}

	// get sram base address
	u16* startAddress = (u16*)&_sram_bss_end;

	// print status header
	Printing::text(Printing::getInstance(), "SRAM STATUS", 1, y++, NULL);
	Printing::text(Printing::getInstance(), "Total (kb):", 1, ++y, NULL);
	Printing::int(Printing::getInstance(), __TOTAL_SAVE_RAM >> 10, 13, y, NULL);
	y+=2;

	// print inspector header
	Printing::text(Printing::getInstance(), "SRAM INSPECTOR", 1, ++y, NULL);
	Printing::text(Printing::getInstance(), "Page     /", 33, y, NULL);
	Printing::int(Printing::getInstance(), totalPages, 43, y, NULL);
	Printing::int(Printing::getInstance(), this->sramPage + 1, 38, y++, NULL);
	Printing::text(Printing::getInstance(), "Address     00 01 02 03 04 05 06 07 Word", 1, ++y, NULL);
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 1, ++y, NULL);

	// print values
	for(i = 0; i < 16; i++)
	{
		// print address
		Printing::text(Printing::getInstance(), "0x00000000: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int)startAddress + (this->sramPage << 7) + (i << 3), 3, y, 8, NULL);

		// values
		for(j = 0; j < 8; j++)
		{
			// read byte from sram
			value = startAddress[(this->sramPage << 7) + (i << 3) + j];

			// print byte
			Printing::hex(Printing::getInstance(), value, 13 + (j*3), y, 2, NULL);

			// add current character to line word
			// if outside of extended ascii range, print whitespace
			word[j] = (value >= 32) ? (char)value : (char)32;
			//word[j] = value ? (char)value : (char)32;
		}

		// add termination character to string
		word[8] = (char)0;

		// print word
		Printing::text(Printing::getInstance(), word, 37, y, NULL);

		// print scroll bar
		Printing::text(Printing::getInstance(), __CHAR_MEDIUM_RED_BOX, 46, y, NULL);
	}

	// mark scroll bar position
	Printing::text(Printing::getInstance(), __CHAR_BRIGHT_RED_BOX, 46, y - 15 + (this->sramPage / (totalPages / 16)), NULL);
}

#endif
