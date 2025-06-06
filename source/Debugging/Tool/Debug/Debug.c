/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Actor.h>
#include <AnimationController.h>
#include <AnimationCoordinator.h>
#include <AnimationCoordinatorFactory.h>
#include <Ball.h>
#include <BgmapSprite.h>
#include <BgmapTextureManager.h>
#include <Body.h>
#include <BodyManager.h>
#include <Box.h>
#include <Camera.h>
#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
#include <CharSet.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <Collider.h>
#include <ColliderManager.h>
#include <Container.h>
#include <DebugState.h>
#include <FrameBufferManager.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <InverseBox.h>
#include <LineField.h>
#include <MBgmapSprite.h>
#include <Mem.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <OptionsSelector.h>
#include <ParamTableManager.h>
#include <Particle.h>
#include <ParticleSystem.h>
#include <Printer.h>
#include <SoundManager.h>
#include <Sphere.h>
#include <Singleton.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <SRAMManager.h>
#include <Stage.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <State.h>
#include <StateMachine.h>
#include <Sound.h>
#include <Telegram.h>
#include <Texture.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>
#include <Wireframe.h>

#include "Debug.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

#ifdef __DEBUG_TOOL
extern ClassSizeData _userClassesSizeData[];
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __CHARS_PER_SEGMENT_TO_SHOW			512
#define __CHARS_PER_ROW_TO_SHOW				32

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::update()
{
	if(this->currentPage->data == &Debug::texturesPage && 0 <= this->bgmapSegment)
	{
		Debug::showBgmapSegment(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::show()
{
	Printer::clear();

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::showAllSprites(spriteManager, NULL, true);
	SpriteManager::computeTotalPixelsDrawn(spriteManager, );

	Debug::showPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::hide()
{
	ColliderManager::hideColliders
	(
		Debug::getColliderManager(this)
	);

	Printer::clear();

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::showAllSprites(spriteManager, NULL, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::processUserInput(uint16 pressedKey)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATTE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->pages = new VirtualList();
	this->subPages = new VirtualList();
	this->currentPage = NULL;
	this->currentSubPage = NULL;
	this->spriteIndex = -1;
	this->bgmapSegment = 0;
	this->objectSegment = 0;
	this->charSegment = 0;
	this->sramPage = 0;
	this->bgmapSegmentDiplayedSection = 0;

	Debug::setupPages(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::destructor()
{
	delete this->pages;
	delete this->subPages;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

ColliderManager Debug::getColliderManager()
{
	return ColliderManager::safeCast(ToolState::getComponentManager(this->toolState, kColliderComponent));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::setupPages()
{
	VirtualList::pushBack(this->pages, &Debug::generalStatusPage);
	VirtualList::pushBack(this->pages, &Debug::memoryStatusPage);
	VirtualList::pushBack(this->pages, &Debug::gameProfilingPage);
	VirtualList::pushBack(this->pages, &Debug::streamingPage);
	VirtualList::pushBack(this->pages, &Debug::spritesPage);
	VirtualList::pushBack(this->pages, &Debug::objectsPage);
	VirtualList::pushBack(this->pages, &Debug::texturesPage);
	VirtualList::pushBack(this->pages, &Debug::charMemoryPage);
	VirtualList::pushBack(this->pages, &Debug::physicsPage);
	VirtualList::pushBack(this->pages, &Debug::hardwareRegistersPage);
	VirtualList::pushBack(this->pages, &Debug::sramPage);

	this->currentPage = this->pages->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 Debug::getCurrentPageNumber()
{
	return VirtualList::getNodeIndex(this->pages, this->currentPage) + 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::setBlackBackground()
{
	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::hideAllSprites(spriteManager, NULL, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showPreviousPage()
{
	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::showAllSprites(spriteManager, NULL, true);

	this->currentPage = VirtualNode::getPrevious(this->currentPage);

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->tail;
	}

	Debug::showPage(this, -1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showNextPage()
{
	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::showAllSprites(spriteManager, NULL, true);

	this->currentPage = this->currentPage->next;

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->head;
	}

	Debug::showPage(this, 1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::printHeader()
{
	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL
	);

	Printer::text(" DEBUG SYSTEM ", 1, 0, NULL);
	Printer::text("   /   ", 16, 0, NULL);
	Printer::int32
	(
		Debug::getCurrentPageNumber(this), Debug::getCurrentPageNumber(this) < 10 ? 18 : 17, 0, NULL
	);
	
	Printer::int32(VirtualList::getCount(this->pages), 20, 0, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showPage(int32 increment)
{
	if(this->currentPage && this->currentPage->data)
	{
		Printer::clear();

		SpriteManager spriteManager = 
			SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

		SpriteManager::showAllSprites(spriteManager, NULL, true);

		Debug::printHeader(this);
		Printer::text(" \x1E\x1C\x1D ", 42, 0, NULL);

		Debug::setBlackBackground(this);

		ColliderManager::hideColliders(Debug::getColliderManager(this));

		((void (*)(Debug, int32, int32, int32))this->currentPage->data)(this, increment, 1, 2);
	}

	Printer::show(Printer::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showSubPage(int32 increment)
{
	if(this->currentSubPage && VirtualNode::getData(this->currentSubPage))
	{
		Printer::clear();

		Debug::printHeader(this);
		Printer::text(" \x1E\x1A\x1B\x1C\x1D ", 40, 0, NULL);

		((void (*)(Debug, int32, int32, int32))VirtualNode::getData(this->currentSubPage))(this, increment, 1, 2);
	}

	Printer::show(Printer::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::displaceLeft()
{
	if(this->bgmapSegmentDiplayedSection % 2 == 1)
	{
		this->bgmapSegmentDiplayedSection--;
	}
	Debug::showDebugBgmap(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::displaceRight()
{
	if(this->bgmapSegmentDiplayedSection % 2 == 0)
	{
		this->bgmapSegmentDiplayedSection++;
	}
	Debug::showDebugBgmap(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::displaceUp()
{
	if(this->bgmapSegmentDiplayedSection > 1)
	{
		this->bgmapSegmentDiplayedSection -= 2;
	}
	Debug::showDebugBgmap(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::displaceDown()
{
	if(this->bgmapSegmentDiplayedSection < 4)
	{
		this->bgmapSegmentDiplayedSection += 2;
	}
	Debug::showDebugBgmap(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::removeSubPages()
{
	VirtualList::clear(this->subPages);
	this->currentSubPage = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::generalStatusPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y)
{
	Debug::removeSubPages(this);

	Printer::text("GAME STATUS", 1, y++, NULL);
	Printer::text("Current State:", 1, ++y, NULL);
	Printer::text(__GET_CLASS_NAME(ToolState::getCurrentGameState(this->toolState)), 20, y, NULL);

	Printer::text("Save Data Manager:", 1, ++y, NULL);
	if(VUEngine::getSaveDataManager())
	{
		Printer::text
		(
			__GET_CLASS_NAME(VUEngine::getSaveDataManager()), 20, y, NULL
		);
	}
	else
	{
		Printer::text("none", 20, y, NULL);
	}
/*
	Printer::text("Active Language:", 1, ++y, NULL);
	Printer::text(I18n::getActiveLanguageName(I18n::getInstance()), 20, y, NULL);
*/
	y += 3;

	Printer::text("CLOCKS STATUS", 1, y++, NULL);
	GameState::printClocks(ToolState::getCurrentGameState(this->toolState), 1, y);
	y+=3;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusPage
(
	int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused))
)
{
	Debug::removeSubPages(this);

#ifdef __DEBUG_TOOL

	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowZeroPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowFirstPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowSecondPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowThirdPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowFourthPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowFifthPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowSixthPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowSeventhPage);
	VirtualList::pushBack(this->subPages, &Debug::memoryStatusShowUserDefinedClassesSizes);

#endif

	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

#ifdef __DEBUG_TOOL

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowZeroPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&VUEngine_getObjectSize, 						"VUEngine"},
		{&FrameBufferManager_getObjectSize, 					"FrameBufferManager"},
		{&FrameRate_getObjectSize, 						"FrameRate"},
		//{&I18n_getObjectSize, 						"I18n"},
		{&MemoryPool_getObjectSize, 					"MemoryPool"},
		{&MessageDispatcher_getObjectSize, 				"MessageDispatcher"},
		{&Printer_getObjectSize, 						"Printer"},
		{&Camera_getObjectSize, 						"Camera"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowFirstPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&BgmapTextureManager_getObjectSize, 			"BgmapTextureManager"},
		{&CharSetManager_getObjectSize, 				"CharSetManager"},
		{&ClockManager_getObjectSize, 					"ClockManager"},
		{&ColliderManager_getObjectSize, 				"ColliderManager"},
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowSecondPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Clock_getObjectSize, 							"Clock"},
		{&Object_getObjectSize, 						"ListenerObject"},
		{&State_getObjectSize, 							"State"},
		{&StateMachine_getObjectSize,				 	"StateMachine"},
		{&Telegram_getObjectSize, 						"Telegram"},
		{&VirtualList_getObjectSize, 					"VirtualList"},
		{&VirtualNode_getObjectSize, 					"VirtualNode"},
		{&Sound_getObjectSize, 							"Sound"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowThirdPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&AnimationController_getObjectSize, 			"AnimationController"},
		{&AnimationCoordinator_getObjectSize, 			"AnimationCoordinat."},
		{&AnimationCoordinatorFactory_getObjectSize,	"AnimationCoor.Fact."},
		{&BgmapSprite_getObjectSize,			"BgmapAnim. Sprite"},
		{&BgmapSprite_getObjectSize, 					"BgmapSprite"},
		{&BgmapTexture_getObjectSize, 					"BgmapTexture"},
		{&CharSet_getObjectSize, 						"CharSet"},
		{&MBgmapSprite_getObjectSize, 					"MBgmapSprite"},
		{&ObjectSprite_getObjectSize,			"ObjectAnim. Sprite"},
		{&ObjectSprite_getObjectSize,					"ObjectSprite"},
		{&ObjectSpriteContainer_getObjectSize,			"ObjectSpriteCont."},
		{&ObjectTexture_getObjectSize,					"ObjectTexture"},
		{&Texture_getObjectSize, 						"Texture"},
		{&Sprite_getObjectSize, 						"Sprite"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowFourthPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&BodyManager_getObjectSize, 					"BodyManager"},
		{&Body_getObjectSize, 							"Body"},
		{&Collider_getObjectSize, 						"Collider"},
		{&Ball_getObjectSize, 							"Ball"},
		{&Box_getObjectSize,							"Box"},
		{&InverseBox_getObjectSize,						"InverseBox"},
		{&LineField_getObjectSize,						"LineField"},
		{&Wireframe_getObjectSize, 						"Wireframe"},
		{&Sphere_getObjectSize, 						"Sphere"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowFifthPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Actor_getObjectSize,					"Actor"},
		{&Actor_getObjectSize,							"Actor"},
		{&Container_getObjectSize,						"Container"},
		{&Actor_getObjectSize,							"Actor"},
		{&ActorFactory_getObjectSize,					"ActorFactory"},
		{&GameState_getObjectSize,						"GameState"},
		{&GameState_getObjectSize,						"Stage"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowSixthPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Particle_getObjectSize,						"Particle"},
		{&ParticleSystem_getObjectSize,					"ParticleSystem"},
		{&Entity_getObjectSize,							"Entity"},
		{&Stage_getObjectSize,							"Stage"},
		{&UIContainer_getObjectSize,					"UIContainer"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowSeventhPage(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);

	ClassSizeData classesSizeData[] =
	{
		{&DebugState::getObjectSize,					"DebugState"},
		{&GameState::getObjectSize,						"GameState"},
		{&StageEditorState::getObjectSize,				"StageEditorState"},
		{&Debug::getObjectSize,							"Debug"},
		{&OptionsSelector::getObjectSize,				"OptionsSelector"},
		{&StageEditor::getObjectSize,					"StageEditor"},
	};

	Debug::printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::memoryStatusShowUserDefinedClassesSizes(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	MemoryPool::printDetailedUsage(x, y);
	Debug::printClassSizes(this, _userClassesSizeData, 0, x + 21, y, "User defined classes:");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::printClassSizes(ClassSizeData* classesSizeData, int32 count, int32 x, int32 y, char* message)
{
	int32 columnIncrement = 20;

	Printer::text("CLASSES MEMORY USAGE (B) ", x, y++, NULL);

	if(message)
	{
		Printer::text(message, x, ++y, NULL);
		y++;
	}

	Printer::text("Name                Size", x, ++y, NULL);
	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 
		x, ++y, NULL
	);

	int32 i = 0;
	for(; classesSizeData[i].classSizeFunction && (0 == count || i < count); i++)
	{
		Printer::text(classesSizeData[i].name, x, ++y, NULL);
		Printer::int32(((int32 (*)(void))classesSizeData[i].classSizeFunction)(), x + columnIncrement, y, NULL);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::gameProfilingPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	// TODO: show profiler
	PRINT_TEXT("TODO: Show profiler", 1, 6);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::streamingPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::streamingShowStatus);
	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::streamingShowStatus
(
	int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused))
)
{
	Stage::print(ToolState::getCurrentStage(this->toolState), x, y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::charMemoryPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::charMemoryShowStatus);
	VirtualList::pushBack(this->subPages, &Debug::charMemoryShowStatus);
	this->currentSubPage = this->subPages->head;

	this->charSegment = -1;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::charMemoryShowStatus(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	this->charSegment += increment;

	int32 charSegments = __CHAR_MEMORY_TOTAL_CHARS / __CHARS_PER_SEGMENT_TO_SHOW;

	if(-1 > this->charSegment)
	{
		this->charSegment = charSegments - 1;
	}

	if(-1 == this->charSegment)
	{
		Debug::setBlackBackground(this);
		CharSetManager::print(x, y);
	}
	else if(charSegments > this->charSegment)
	{
		Printer::text("CHAR MEMORY INSPECTOR", x, y++, NULL);
		Printer::text("Segment:  / ", x, ++y, NULL);
		Printer::int32(this->charSegment + 1, x + 9, y, NULL);
		Printer::int32(charSegments, x + 11, y, NULL);
		Printer::text("Chars:       -    ", x, ++y, NULL);
		Printer::int32(this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW, x + 9, y, NULL);
		Printer::int32
		(
			this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + __CHARS_PER_SEGMENT_TO_SHOW - 1, x + 14, y, NULL
		);

		Debug::charMemoryShowMemory(this, increment, x, y);
	}
	else
	{
		this->charSegment = -1;
		Debug::setBlackBackground(this);
		CharSetManager::print(x, y);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::charMemoryShowMemory(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y)
{
	Debug::setBlackBackground(this);

	int32 i = 0;
	int32 yOffset = y + 3;

	// Print box
	Printer::text
	(
		"\x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x04", 1, yOffset-1, NULL
	);
	
	Printer::text
	(
		"\x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x06", 1, yOffset+16, NULL
	);

	for(i = 0; i < __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW && i < __SCREEN_HEIGHT / 8; i++)
	{
		Printer::text("\x07                                \x07", 1, yOffset+i, NULL);
	}

	const HWORD charMemoryMap[] =
	{
		0,	1,	2,	3,	4,	5,	6,	7,
		8,	9,	10,	11,	12,	13,	14,	15,
		16,	17,	18,	19,	20,	21,	22,	23,
		24,	25,	26,	27,	28,	29,	30,	31
	};

	// Put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW; i++)
	{
		Mem::addOffsetToHWORD
		(
			Printer::getPrintingBgmapAddress() + ((yOffset + i) << 6) + 2,
			(HWORD*)charMemoryMap,
			__CHARS_PER_ROW_TO_SHOW,
			this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + i * __CHARS_PER_ROW_TO_SHOW
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::texturesPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::texturesShowStatus);
	VirtualList::pushBack(this->subPages, &Debug::texturesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->bgmapSegment = -1;
	this->bgmapSegmentDiplayedSection = 0;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showDebugBgmap()
{
	if(this->currentPage->data != &Debug::texturesPage ||
		0 > this->bgmapSegment
	)
	{
		return;
	}

	Debug::setBlackBackground(this);
	Debug::lightUpGame(this);
	Debug::showBgmapSegment(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showBgmapSegment()
{
	int32 topBorder = 0;
	int32 bottomBorder = 0;
	int32 leftBorder = 0;
	int32 rightBorder = 0;
	int32 mxDisplacement = 0;
	int32 myDisplacement = 0;

	uint8 i = 0;
	uint8 yOffset = 4;

	// Print box
	switch(this->bgmapSegmentDiplayedSection)
	{
		case 0:
		{
			Printer::text
			(
				" \x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, yOffset, NULL
			);

			Printer::text("                                                ", 0, 26, NULL);

			for(i = yOffset + 1; i < 28; i++)
			{
				Printer::text(" \x07", 0, i, NULL);
				Printer::text(" ", 46, i, NULL);
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
			Printer::text
			(
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x04 ", 0, yOffset, NULL
			);

			Printer::text("                                                ", 0, 26, NULL);

			for(i = yOffset + 1; i < 28; i++)
			{
				Printer::text(" ", 1, i, NULL);
				Printer::text("\x07 ", 46, i, NULL);
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
			Printer::text("                                                ", 0, yOffset, NULL);
			Printer::text("                                                ", 0, 26, NULL);

			for(i = yOffset; i < 28; i++)
			{
				Printer::text(" \x07", 0, i, NULL);
				Printer::text(" ", 46, i, NULL);
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
			Printer::text("                                                ", 0, yOffset, NULL);
			Printer::text("                                                ", 0, 26, NULL);

			for(i = yOffset; i < 28; i++)
			{
				Printer::text(" ", 1, i, NULL);
				Printer::text("\x07 ", 46, i, NULL);
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
			Printer::text("                                                ", 0, yOffset, NULL);
			Printer::text
			(
				" \x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 26, NULL
			);

			Printer::text("                                                ", 0, 27, NULL);

			for(i = yOffset; i < 26; i++)
			{
				Printer::text(" \x07", 0, i, NULL);
				Printer::text(" ", 46, i, NULL);
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
			Printer::text("                                                ", 0, yOffset, NULL);
			Printer::text
			(
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
				"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x06 ", 0, 26, NULL
			);

			Printer::text("                                                ", 0, 27, NULL);

			for(i = yOffset; i < 26; i++)
			{
				Printer::text(" ", 1, i, NULL);
				Printer::text("\x07 ", 46, i, NULL);
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

	uint32 numberOfHWORDS = __SCREEN_WIDTH_IN_CHARS - leftBorder - rightBorder;
	uint32 offsetDisplacement = leftBorder;

	uint16* const bgmapSpaceBaseAddress = (uint16*)__BGMAP_SPACE_BASE_ADDRESS;

	for(int32 row = 0; row < __SCREEN_HEIGHT_IN_CHARS - topBorder - bottomBorder; row++)
	{
		Mem::copyHWORD
		(
			Printer::getPrintingBgmapAddress() + ((row + topBorder) << 6) + offsetDisplacement,
			(const HWORD*)(&bgmapSpaceBaseAddress[(0x1000 * (this->bgmapSegment)) + ((row + myDisplacement) << 6) + mxDisplacement]), 
			numberOfHWORDS
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::texturesShowStatus(int32 increment, int32 x, int32 y)
{
	this->bgmapSegment += increment;

	if(-1 > this->bgmapSegment)
	{
		this->bgmapSegment = BgmapTextureManager::getAvailableBgmapSegmentsForTextures(BgmapTextureManager::getInstance()) - 1;
	}

	if(-1 == this->bgmapSegment)
	{
		Debug::setBlackBackground(this);
		BgmapTextureManager::print(x, y);
		ParamTableManager::print(x + 27, y);
	}
	else if(BgmapTextureManager::getAvailableBgmapSegmentsForTextures(BgmapTextureManager::getInstance()) > this->bgmapSegment)
	{
		Printer::text(" \x1E\x1A\x1B\x1C\x1D\x1F\x1A\x1B\x1C\x1D ", 35, 0, NULL);
		Printer::text("BGMAP TEXTURES INSPECTOR           Segment: ", x, y, NULL);
		Printer::int32(this->bgmapSegment, x + 44, y, NULL);

		this->bgmapSegmentDiplayedSection = 0;

		Debug::showDebugBgmap(this);
	}
	else
	{
		this->bgmapSegment = -1;
		Debug::setBlackBackground(this);
		BgmapTextureManager::print(x, y);
		ParamTableManager::print(x + 27, y);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::objectsPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::objectsShowStatus);
	VirtualList::pushBack(this->subPages, &Debug::objectsShowStatus);
	this->currentSubPage = this->subPages->head;

	this->objectSegment = 0;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::objectsShowStatus(int32 increment, int32 x, int32 y)
{
	this->objectSegment += increment;

	Debug::dimmGame(this);

	if(0 > this->objectSegment)
	{
		this->objectSegment = __TOTAL_OBJECT_SEGMENTS - 1;
	}
	else if(__TOTAL_OBJECT_SEGMENTS <= this->objectSegment)
	{
		this->objectSegment = 0;
	}

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	Printer::text("OBJECTS INSPECTOR", x, y++, NULL);
	Debug::setBlackBackground(this);
	SpriteManager::printSPTInfo(spriteManager, this->objectSegment, x, ++y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::spritesPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::spritesShowStatus);
	VirtualList::pushBack(this->subPages, &Debug::spritesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->spriteIndex = -1;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::spritesShowStatus(int32 increment, int32 x, int32 y)
{
	this->spriteIndex -= increment;

	Debug::dimmGame(this);

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	int32 numberOfSprites = SpriteManager::getNumberOfSprites(spriteManager);

	if(this->spriteIndex > numberOfSprites)
	{
		this->spriteIndex = 0;
	}

	if(numberOfSprites == this->spriteIndex)
	{
		Debug::setBlackBackground(this);
		SpriteManager::print(spriteManager, x, y, false);
	}
	else if(0 <= this->spriteIndex && this->spriteIndex < numberOfSprites)
	{
		Sprite sprite = SpriteManager::getSpriteAtIndex(spriteManager, this->spriteIndex);
		SpriteManager::hideAllSprites(spriteManager, sprite, false);
		SpriteManager::renderAndDraw(spriteManager);
		Printer::text("SPRITES INSPECTOR", x, y++, NULL);
		Sprite::print(sprite, x, ++y);
	}
	else
	{
		this->spriteIndex = numberOfSprites;

		Debug::setBlackBackground(this);
		SpriteManager::print(spriteManager, x, y, false);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::physicsPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::physicStatusShowStatistics);
	VirtualList::pushBack(this->subPages, &Debug::physicStatusShowColliders);
	this->currentSubPage = this->subPages->head;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::physicStatusShowStatistics(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	BodyManager::print(BodyManager::safeCast(ToolState::getComponentManager(this->toolState, kPhysicsComponent)), x, y);
	ColliderManager::print(Debug::getColliderManager(this), x, y + 6);
	ColliderManager::hideColliders(Debug::getColliderManager(this));

	Debug::setBlackBackground(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::physicStatusShowColliders(int32 increment __attribute__ ((unused)), int32 x, int32 y)
{
	Printer::text("COLLISION SHAPES", x, y++, NULL);

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(this->toolState, kSpriteComponent));

	SpriteManager::showAllSprites(spriteManager, NULL, true);
	Debug::dimmGame(this);
	ColliderManager::showColliders(Debug::getColliderManager(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::hardwareRegistersPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y)
{
	Debug::removeSubPages(this);

	HardwareManager::print(x, y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::sramPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y __attribute__ ((unused)))
{
	Debug::removeSubPages(this);

	VirtualList::pushBack(this->subPages, &Debug::showSramPage);
	VirtualList::pushBack(this->subPages, &Debug::showSramPage);
	this->currentSubPage = this->subPages->head;

	this->sramPage = 0;

	Debug::showSubPage(this, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Debug::showSramPage(int32 increment __attribute__ ((unused)), int32 x __attribute__ ((unused)), int32 y)
{
	uint8 value;
	int32 i, j, totalPages;
	char word[9];

	totalPages = __TOTAL_SAVE_RAM >> 7;

	extern uint32 _sramBssStart;

	this->sramPage += increment;

	if(this->sramPage < 0)
	{
		this->sramPage = totalPages - 1;
	}
	else if(this->sramPage >= totalPages)
	{
		this->sramPage = 0;
	}

	// Get sram base address
	uint16* startAddress = (uint16*)&_sramBssStart;

	// Print status header
	Printer::text("SRAM STATUS", 1, y++, NULL);
	Printer::text("Total (kb):", 1, ++y, NULL);
	Printer::int32(__TOTAL_SAVE_RAM >> 10, 13, y, NULL);
	y+=2;

	// Print inspector header
	Printer::text("SRAM INSPECTOR", 1, ++y, NULL);
	Printer::text("Page     /", 33, y, NULL);
	Printer::int32(totalPages, 43, y, NULL);
	Printer::int32(this->sramPage + 1, 38, y++, NULL);
	Printer::text("Address     00 01 02 03 04 05 06 07 Word", 1, ++y, NULL);
	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 1, ++y, NULL
	);

	// Print values
	for(i = 0; i < 16; i++)
	{
		// Print address
		Printer::text("0x00000000: ", 1, ++y, NULL);
		Printer::hex((int32)startAddress + (this->sramPage << 7) + (i << 3), 3, y, 8, NULL);

		// Values
		for(j = 0; j < 8; j++)
		{
			// Read byte from sram
			value = startAddress[(this->sramPage << 7) + (i << 3) + j];

			// Print byte
			Printer::hex(value, 13 + (j*3), y, 2, NULL);

			// Add current character to line word
			// If outside of extended ascii range, print whitespace
			word[j] = (value >= 32) ? (char)value : (char)32;
			//word[j] = value ? (char)value : (char)32;
		}

		// Add termination character to string
		word[8] = (char)0;

		// Print word
		Printer::text(word, 37, y, NULL);

		// Print scroll bar
		Printer::text(__CHAR_MEDIUM_RED_BOX, 46, y, NULL);
	}

	// Mark scroll bar position
	Printer::text(__CHAR_BRIGHT_RED_BOX, 46, y - 15 + (this->sramPage / (totalPages / 16)), NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
