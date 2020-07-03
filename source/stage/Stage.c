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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Stage.h>
#include <Optics.h>
#include <Game.h>
#include <EntityFactory.h>
#include <PhysicalWorld.h>
#include <TimerManager.h>
#include <SoundManager.h>
#include <Camera.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <CharSetManager.h>
#include <BgmapTexture.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <ParticleRemover.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#include <TimerManager.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STREAMING_CYCLES		5

#define __MAXIMUM_PARALLAX		10
#define __LOAD_LOW_X_LIMIT		(-__MAXIMUM_PARALLAX - this->streaming.loadPadding)
#define __LOAD_HIGHT_X_LIMIT	(__SCREEN_WIDTH + __MAXIMUM_PARALLAX + this->streaming.loadPadding)
#define __LOAD_LOW_Y_LIMIT		(-this->streaming.loadPadding)
#define __LOAD_HIGHT_Y_LIMIT	(__SCREEN_HEIGHT + this->streaming.loadPadding)
#define __LOAD_LOW_Z_LIMIT		(-this->streaming.loadPadding)
#define __LOAD_HIGHT_Z_LIMIT	(__SCREEN_DEPTH + this->streaming.loadPadding)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Container;
friend class Entity;
friend class VirtualNode;
friend class VirtualList;

const Transformation neutralEnvironmentTransformation =
{
	// spatial local position
	{0, 0, 0},

	// spatial global position
	{0, 0, 0},

	// local rotation
	{0, 0, 0},

	// global rotation
	{0, 0, 0},

	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

};

#ifdef __PROFILE_STREAMING
extern s16 _renderingProcessTimeHelper;
#endif

typedef bool (*StreamingPhase)(void*, int);

static const StreamingPhase _streamingPhases[] =
{
	&Stage::unloadOutOfRangeEntities,
	&Stage::loadInRangeEntities
};

#ifdef __PROFILE_STREAMING
static u32 unloadOutOfRangeEntitiesHighestTime = 0;
static u32 loadInRangeEntitiesHighestTime = 0;
static u32 processRemovedEntitiesHighestTime = 0;
static u32 entityFactoryHighestTime = 0;
static u32 timeBeforeProcess = 0;
#endif


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Stage::constructor(StageSpec *stageSpec)
{
	// construct base object
	Base::constructor(NULL);

	this->entityFactory = new EntityFactory();
	this->particleRemover = new ParticleRemover();
	this->children = new VirtualList();

	this->stageSpec = stageSpec;
	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->uiContainer = NULL;
	this->focusEntity = NULL;
	this->streamingHeadNode = NULL;
	this->cameraPreviousDistance = 0;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->streamingCycleCounter = 0;
	this->soundWrappers = NULL;
	this->streaming = this->stageSpec->streaming;
}

// class's destructor
void Stage::destructor()
{
	Stage::setFocusEntity(this, NULL);

	if(!isDeleted(this->soundWrappers))
	{
		VirtualNode node = this->soundWrappers->head;

		for(; node; node = node->next)
		{
			if(!isDeleted(node->data))
			{
				SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);
				SoundWrapper::release(soundWrapper);
			}
		}

		delete this->soundWrappers;
		this->soundWrappers = NULL;
	}

	delete this->particleRemover;
	this->particleRemover = NULL;

	if(this->entityFactory)
	{
		delete this->entityFactory;
		this->entityFactory = NULL;
	}

	if(this->uiContainer)
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(this->stageEntities)
	{
		VirtualNode node = this->stageEntities->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->stageEntities;

		this->stageEntities = NULL;
	}

	if(this->loadedStageEntities)
	{
		delete this->loadedStageEntities;
		this->loadedStageEntities = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// determine if a point is visible
int Stage::isEntityInLoadRange(ScreenPixelVector onScreenPosition, const PixelRightBox* pixelRightBox, const PixelVector* cameraPosition)
{
	onScreenPosition.x -= cameraPosition->x;
	onScreenPosition.y -= cameraPosition->y;
	onScreenPosition.z -= cameraPosition->z;

	// check x visibility
	if(onScreenPosition.x + pixelRightBox->x1 <  __LOAD_LOW_X_LIMIT || onScreenPosition.x + pixelRightBox->x0 >  __LOAD_HIGHT_X_LIMIT)
	{
		return false;
	}

	// check y visibility
	if(onScreenPosition.y + pixelRightBox->y1 <  __LOAD_LOW_Y_LIMIT || onScreenPosition.y + pixelRightBox->y0 >  __LOAD_HIGHT_Y_LIMIT)
	{
		return false;
	}

	// check z visibility
	if(onScreenPosition.z + pixelRightBox->z1 <  __LOAD_LOW_Z_LIMIT || onScreenPosition.z + pixelRightBox->z0 >  __LOAD_HIGHT_Z_LIMIT)
	{
		return false;
	}

	return true;
}

void Stage::setObjectSpritesContainers()
{
	SpriteManager::setupObjectSpriteContainers(SpriteManager::getInstance(), this->stageSpec->rendering.objectSpritesContainersSize, this->stageSpec->rendering.objectSpritesContainersZPosition);
}

void Stage::setupPalettes()
{
	VIPManager::setupPalettes(VIPManager::getInstance(), &this->stageSpec->rendering.paletteConfig);
}

// load stage's entites
void Stage::load(VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	// Setup timer
	Stage::setupTimer(this);

	// set optical values
	Camera::setOptical(Camera::getInstance(), Optical::getFromPixelOptical(this->stageSpec->rendering.pixelOptical));

	// stop all sounds
	SoundManager::stopAllSounds(SoundManager::getInstance());

	if(overrideCameraPosition)
	{
		Camera::reset(Camera::getInstance());
		Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageSpec->level.pixelSize));
		Camera::setPosition(Camera::getInstance(), Vector3D::getFromPixelVector(this->stageSpec->level.cameraInitialPosition));
	}

	Camera::setCameraFrustum(Camera::getInstance(), this->stageSpec->level.cameraFrustum);

	Stage::prepareGraphics(this);

	// setup ui
	Stage::setupUI(this);

	// register all the entities in the stage's spec
	Stage::registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage::loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));

	// set physics
	PhysicalWorld::setFrictionCoefficient(Game::getPhysicalWorld(Game::getInstance()), this->stageSpec->physics.frictionCoefficient);
	PhysicalWorld::setGravity(Game::getPhysicalWorld(Game::getInstance()), this->stageSpec->physics.gravity);

	// load background music
	Stage::setupSounds(this);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	VIPManager::setupBrightness(VIPManager::getInstance(), &this->stageSpec->rendering.colorConfig.brightness);
	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.brightnessRepeat);

	// set particle removal delay
	ParticleRemover::setRemovalDelayCycles(this->particleRemover, this->streaming.particleRemovalDelayCycles);

	// apply transformations
	Container::initialTransform(this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}
}

void Stage::loadPostProcessingEffects()
{
	if(this->stageSpec->postProcessingEffects)
	{
		int i = 0;
		for(; this->stageSpec->postProcessingEffects[i]; i++)
		{
			Game::pushFrontProcessingEffect(Game::getInstance(), this->stageSpec->postProcessingEffects[i], NULL);
		}
	}
}

// retrieve size
Size Stage::getSize()
{
	ASSERT(this->stageSpec, "Stage::getSize: null stageSpec");

	// set world's limits
	return Size::getFromPixelSize(this->stageSpec->level.pixelSize);
}

CameraFrustum Stage::getCameraFrustum()
{
	ASSERT(this->stageSpec, "Stage::getCameraFrustum: null stageSpec");

	// set world's limits
	return this->stageSpec->level.cameraFrustum;
}
// setup ui
void Stage::setupUI()
{
	ASSERT(!this->uiContainer, "Stage::setupUI: UI already exists");

	if(this->uiContainer)
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(this->stageSpec->entities.uiContainerSpec.allocator)
	{
		// call the appropriate allocator to support inheritance
		this->uiContainer = ((UIContainer (*)(UIContainerSpec*)) this->stageSpec->entities.uiContainerSpec.allocator)(&this->stageSpec->entities.uiContainerSpec);
		ASSERT(this->uiContainer, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if(this->uiContainer)
		{
			// apply transformations
			Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
		}
	}
}

// add entity to the stage
Entity Stage::addChildEntity(const PositionedEntity* const positionedEntity, bool permanent)
{
	return Stage::doAddChildEntity(this, positionedEntity, permanent, this->nextEntityId++, true);
}

// add entity to the stage
Entity Stage::doAddChildEntity(const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), s16 internalId, bool makeReady)
{
	if(positionedEntity)
	{
		Entity entity = Entity::loadEntity(positionedEntity, internalId);
		ASSERT(entity, "Stage::doAddChildEntity: entity not loaded");

		if(entity)
		{
			// create the entity and add it to the world
			Stage::addChild(this, Container::safeCast(entity));

			// apply transformations
			Entity::initialTransform(entity, &neutralEnvironmentTransformation, true);

			if(makeReady)
			{
				Stage::makeChildReady(this, entity);
			}
		}
/*
		if(permanent)
		{
			// TODO
		}
*/
		return entity;
	}

	return NULL;
}

// initialize child
void Stage::makeChildReady(Entity entity)
{
	ASSERT(entity, "Stage::setChildReady: null entity");
	ASSERT(entity->parent == Container::safeCast(this), "Stage::setChildReady: I'm not its parent");

	if(entity->parent == Container::safeCast(this))
	{
		Entity::synchronizeGraphics(entity);
		Entity::ready(entity, true);
	}
}

bool Stage::registerEntityId(s16 internalId, EntitySpec* entitySpec)
{
	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(entitySpec == stageEntityDescription->positionedEntity->entitySpec)
		{
			stageEntityDescription->internalId = internalId;
			return true;
		}
	}

	return false;
}

void Stage::spawnEntity(PositionedEntity* positionedEntity, Container requester, EventListener callback)
{
	EntityFactory::spawnEntity(this->entityFactory, positionedEntity, requester, callback, this->nextEntityId++);
}

// remove entity from the stage
void Stage::removeChild(Container child, bool deleteChild)
{
	ASSERT(child, "Stage::removeEntity: null child");

	if(!child)
	{
		return;
	}

	Base::removeChild(this, child, deleteChild);

	s16 internalId = Entity::getInternalId(child);

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;
			break;
		}
	}

	if(node)
	{
		if(this->streamingHeadNode == node)
		{
			this->streamingHeadNode = this->streamingHeadNode->previous;
		}

		VirtualList::removeElement(this->stageEntities, node->data);
		VirtualList::removeElement(this->loadedStageEntities, node->data);
		delete node->data;
	}
}

// unload entity from the stage
void Stage::unloadChild(Container child)
{
	ASSERT(child, "Stage::unloadChild: null child");

	if(!child)
	{
		return;
	}

	Base::removeChild(this, child, true);
	Container::fireEvent(child, kEventStageChildStreamedOut);
	NM_ASSERT(!isDeleted(child), "Stage::unloadChild: deteled child during kEventStageChildStreamedOut");
	Object::removeAllEventListeners(child, kEventStageChildStreamedOut);
	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(child));
	MessageDispatcher::discardAllDelayedMessagesForReceiver(MessageDispatcher::getInstance(), Object::safeCast(child));

	s16 internalId = Entity::getInternalId(child);

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;

			// remove from list of entities that are to be loaded by the streaming,
			// if the entity is not to be respawned
			if(!Entity::respawn(child))
			{
				VirtualList::removeElement(this->stageEntities, node->data);
			}

			break;
		}
	}
}

// preload fonts, charsets and textures
void Stage::preloadAssets()
{
	// fonts
	Printing::loadFonts(Printing::getInstance(), this->stageSpec->assets.fontSpecs);

	// charsets
	if(this->stageSpec->assets.charSetSpecs)
	{
		int i = 0;

		for(; this->stageSpec->assets.charSetSpecs[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageSpec->assets.charSetSpecs[i]->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageSpec->assets.charSetSpecs[i]->allocationType)
			{
				CharSetManager::getCharSet(CharSetManager::getInstance(), this->stageSpec->assets.charSetSpecs[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: preloading an animated single char set");
			}
		}
	}

	// textures
	if(this->stageSpec->assets.textureSpecs)
	{
		VirtualList recyclableTextures = new VirtualList();
		int i = 0;

		for(; this->stageSpec->assets.textureSpecs[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageSpec->assets.textureSpecs[i]->charSetSpec->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageSpec->assets.textureSpecs[i]->charSetSpec->allocationType)
			{
				BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), this->stageSpec->assets.textureSpecs[i], 0, false);

				NM_ASSERT(!isDeleted(bgmapTexture), "Stage::preloadAssets: failed to load bgmapTexture");

				if(!isDeleted(bgmapTexture))
				{
					if(this->stageSpec->assets.textureSpecs[i]->recyclable)
					{
						VirtualList::pushBack(recyclableTextures, bgmapTexture);
					}
					else
					{
						Texture::write(bgmapTexture);
					}
				}
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an Object texture");
			}
		}

		VirtualNode node = VirtualList::begin(recyclableTextures);

		for(; node; node = node->next)
		{
			BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), BgmapTexture::safeCast(node->data));
		}

		delete recyclableTextures;
	}

	ParamTableManager::calculateParamTableBase(ParamTableManager::getInstance(), this->stageSpec->rendering.paramTableSegments);
}

// register an entity in the streaming list
StageEntityDescription* Stage::registerEntity(PositionedEntity* positionedEntity)
{
	ASSERT(positionedEntity, "Stage::registerEntities: null positionedEntity");

	StageEntityDescription* stageEntityDescription = new StageEntityDescription;

	stageEntityDescription->internalId = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	PixelVector environmentPosition = {0, 0, 0, 0};
	stageEntityDescription->pixelRightBox = Entity::getTotalSizeFromSpec(stageEntityDescription->positionedEntity, &environmentPosition);

	int x = stageEntityDescription->positionedEntity->onScreenPosition.x - (stageEntityDescription->pixelRightBox.x1 - stageEntityDescription->pixelRightBox.x0) / 2;
	int y = stageEntityDescription->positionedEntity->onScreenPosition.y - (stageEntityDescription->pixelRightBox.y1 - stageEntityDescription->pixelRightBox.y0) / 2;
	int z = stageEntityDescription->positionedEntity->onScreenPosition.z - (stageEntityDescription->pixelRightBox.z1 - stageEntityDescription->pixelRightBox.z0) / 2;

	stageEntityDescription->distance = x * x + y * y + z * z;

	return stageEntityDescription;
}

// register the stage's spec entities in the streaming list
void Stage::registerEntities(VirtualList positionedEntitiesToIgnore)
{
	if(!isDeleted(this->stageEntities))
	{
		return;
	}

	this->stageEntities = new VirtualList();

	if(this->loadedStageEntities)
	{
		delete this->loadedStageEntities;
	}

	this->loadedStageEntities = new VirtualList();

	// register entities ordering them according to their distances to the origin
	int i = 0;

	for(;this->stageSpec->entities.children[i].entitySpec; i++)
	{
		if(positionedEntitiesToIgnore)
		{
			VirtualNode node = positionedEntitiesToIgnore->head;

			for(; node; node = node->next)
			{
				if(&this->stageSpec->entities.children[i] == (PositionedEntity*)node->data)
				{
					break;
				}
			}

			if(node)
			{
				continue;
			}
		}

		StageEntityDescription* stageEntityDescription = Stage::registerEntity(this, &this->stageSpec->entities.children[i]);

		VirtualNode auxNode = this->stageEntities->head;

		for(; auxNode; auxNode = auxNode->next)
		{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

			if(stageEntityDescription->distance > auxStageEntityDescription->distance)
			{
				continue;
			}

			VirtualList::insertBefore(this->stageEntities, auxNode, stageEntityDescription);
			break;
		}

		if(!auxNode)
		{
			VirtualList::pushBack(this->stageEntities, stageEntityDescription);
		}
	}
}

// load all visible entities
void Stage::loadInitialEntities()
{
	extern const Vector3D* _cameraPosition;
	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	// need a temporary list to remove and delete entities
	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(-1 == stageEntityDescription->internalId)
		{
			// if entity in load range
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox, &cameraPosition))
			{
				stageEntityDescription->internalId = this->nextEntityId++;
				Entity entity = Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, false);
				ASSERT(entity, "Stage::loadInitialEntities: entity not loaded");

				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					this->streamingHeadNode = node;
				}

				stageEntityDescription->internalId = Entity::getInternalId(entity);

				VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);
			}
		}
	}

	node = this->children->head;

	for(; node; node = node->next)
	{
		Stage::makeChildReady(this, Entity::safeCast(node->data));
	}
}

// unload non visible entities
bool Stage::unloadOutOfRangeEntities(int defer)
{
	if(!this->children)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool unloadedEntities = false;

	// need a temporary list to remove and delete entities
	VirtualNode node = this->children->head;

	// check which actors must be unloaded
	for(; node; node = node->next)
	{
		// get next entity
		Entity entity = Entity::safeCast(node->data);

		// if the entity isn't visible inside the view field, unload it
		if(!entity->deleteMe && entity->parent == Container::safeCast(this) && !Entity::isVisible(entity, (this->streaming.loadPadding + this->streaming.unloadPadding + __MAXIMUM_PARALLAX), true))
		{
			s16 internalId = Entity::getInternalId(entity);

			VirtualNode auxNode = this->loadedStageEntities->head;
			StageEntityDescription* stageEntityDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				stageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(stageEntityDescription->internalId == internalId)
				{
					break;
				}
			}

			bool unloaded = false;

			if(stageEntityDescription)
			{
				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					// unload it
					Stage::unloadChild(this, Container::safeCast(entity));
					VirtualList::removeElement(this->loadedStageEntities, stageEntityDescription);

					unloaded = true;
				}
			}
			else
			{
				// unload it
				Stage::unloadChild(this, Container::safeCast(entity));

				unloaded = true;
			}

			unloadedEntities = unloadedEntities || unloaded;

			if(unloaded && defer)
			{
#ifdef __PROFILE_STREAMING
				u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
				unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif
				return true;
			}
		}
	}

#ifdef __PROFILE_STREAMING
		u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
		unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif

	return unloadedEntities;
}

bool Stage::loadInRangeEntities(int defer __attribute__ ((unused)))
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool loadedEntities = false;

	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	u32 cameraDistance = (cameraPosition.x * cameraPosition.x +
							cameraPosition.y * cameraPosition.y +
							cameraPosition.z * cameraPosition.z);

	static int advancing __INITIALIZED_DATA_SECTION_ATTRIBUTE = true;
	u16 amplitude = this->streaming.streamingAmplitude;

	if(this->cameraPreviousDistance != cameraDistance)
	{
		advancing = this->cameraPreviousDistance < cameraDistance;
	}

	VirtualNode node = this->streamingHeadNode ? this->streamingHeadNode : advancing? this->stageEntities->head : this->stageEntities->tail;

	int counter = 0;

	this->streamingHeadNode = NULL;

	if(advancing)
	{
		for(; node && counter < amplitude >> 1; node = node->previous, counter++);

		node = node ? node : this->stageEntities->head;

		for(counter = 0; node && (!this->streamingHeadNode || counter < amplitude); node = node->next)
		{
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

			if(0 > stageEntityDescription->internalId)
			{
				counter++;

				if(!this->streamingHeadNode)
				{
					if(cameraDistance < stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox, &cameraPosition))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, true);
					}
				}
			}
		}
	}
	else
	{
		for(; node && counter < amplitude >> 1; node = node->next, counter++);

		node = node ? node : this->stageEntities->tail;

		for(counter = 0; node && (!this->streamingHeadNode || counter < amplitude); node = node->previous)
		{
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

			if(0 > stageEntityDescription->internalId)
			{
				counter++;

				if(!this->streamingHeadNode)
				{
					if(cameraDistance > stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox, &cameraPosition))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, true);
					}
				}
			}
		}
	}

	this->cameraPreviousDistance = cameraDistance;

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif

	return loadedEntities;
}

Entity Stage::findChildByInternalId(s16 internalId)
{
	VirtualNode node = this->children->head;

	for(; node; node = node->next)
	{
		if(Entity::getInternalId(Entity::safeCast(node->data)) == internalId)
		{
			return Entity::safeCast(node->data);
		}
	}

	return NULL;
}


// process removed children
bool Stage::purgeChildrenProgressively()
{
	if(!this->removedChildren || !this->removedChildren->head)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	Container child = Container::safeCast(VirtualList::front(this->removedChildren));

	VirtualList::popFront(this->removedChildren);
	VirtualList::removeElement(this->children, child);

	if(!isDeleted(child))
	{
		if(child->deleteMe)
		{
			delete child;

#ifdef __PROFILE_STREAMING
			u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
			processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif
			return true;
		}
	}

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif

	return false;
}

//
bool Stage::updateEntityFactory()
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool preparingEntities = EntityFactory::prepareEntities(this->entityFactory);

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif

	return preparingEntities;
}

VirtualList Stage::getSoundWrappers()
{
	return this->soundWrappers;
}

bool Stage::stream()
{
#ifdef __SHOW_STREAMING_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		EntityFactory::showStatus(this->entityFactory, 25, 3);
	}
#endif

	if(Stage::purgeChildrenProgressively(this) && this->streaming.deferred)
	{
		return true;
	}

	if(Stage::updateEntityFactory(this) && this->streaming.deferred)
	{
		return true;
	}

	int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

	return _streamingPhases[this->streamingPhase](this, this->streaming.deferred);
}

void Stage::streamAll()
{
	while(Stage::stream(this));
}

// execute stage's logic
void Stage::update(u32 elapsedTime)
{
	Base::update(this, elapsedTime);

	if(this->uiContainer)
	{
		Container::update(this->uiContainer, elapsedTime);
	}

	ParticleRemover::update(this->particleRemover);
}

// transformation state
void Stage::transform(const Transformation* environmentTransform __attribute__ ((unused)), u8 invalidateTransformationFlag)
{
	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	if(this->uiContainer)
	{
		Container::transform(this->uiContainer, environmentTransform, invalidateTransformationFlag);
	}
}

void Stage::synchronizeGraphics()
{
	Base::synchronizeGraphics(this);

	if(this->uiContainer)
	{
		Container::synchronizeGraphics(this->uiContainer);
	}
}

// retrieve ui
UIContainer Stage::getUIContainer()
{
	return this->uiContainer;
}

// suspend for pause
void Stage::suspend()
{
	// stream all pending entities to avoid having manually recover
	// the stage entity registries
	while(EntityFactory::prepareEntities(this->entityFactory));
//	EntityFactory::prepareAllEntities(this->entityFactory); // It seems buggy

	Base::suspend(this);

	if(this->uiContainer)
	{
		Container::suspend(this->uiContainer);
	}

	// relinquish camera focus priority
	if(this->focusEntity && Camera::getFocusEntity(Camera::getInstance()))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			// relinquish focus entity
			Camera::setFocusGameEntity(Camera::getInstance(), NULL);
		}
	}
	else
	{
		Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));
	}

	delete this->entityFactory;
	this->entityFactory = NULL;
	ParticleRemover::reset(this->particleRemover);
}

// resume after pause
void Stage::resume()
{
	// set back optical values
	Camera::setOptical(Camera::getInstance(), Optical::getFromPixelOptical(this->stageSpec->rendering.pixelOptical));

	// set physics
	PhysicalWorld::setFrictionCoefficient(Game::getPhysicalWorld(Game::getInstance()), this->stageSpec->physics.frictionCoefficient);
	PhysicalWorld::setGravity(Game::getPhysicalWorld(Game::getInstance()), this->stageSpec->physics.gravity);

	Stage::prepareGraphics(this);

	if(this->focusEntity)
	{
		// recover focus entity
		Camera::setFocusGameEntity(Camera::getInstance(), Entity::safeCast(this->focusEntity));
	}

	// Setup timer
	Stage::setupTimer(this);

	// load background sounds
	Stage::setupSounds(this);

	Base::resume(this);

	// apply transformations
	Container::initialTransform(this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		Container::resume(this->uiContainer);
		Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}

	this->entityFactory = new EntityFactory();
}

void Stage::prepareGraphics()
{
	// Must clean DRAM
	SpriteManager::reset(SpriteManager::getInstance());
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());
	CharSetManager::reset(CharSetManager::getInstance());

	// set palettes
	Stage::setupPalettes(this);

	// setup OBJs
	Stage::setObjectSpritesContainers(this);

	// preload textures
	Stage::preloadAssets(this);

	// setup SpriteManager's configuration
	SpriteManager::setCyclesToWaitForTextureWriting(SpriteManager::getInstance(), this->stageSpec->rendering.cyclesToWaitForTextureWriting);
	SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager::getInstance(), this->stageSpec->rendering.texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance(), this->stageSpec->rendering.maximumAffineRowsToComputePerCall);
}

void Stage::setupSounds()
{
	SoundManager::deferMIDIPlayback(SoundManager::getInstance(), this->stageSpec->sound.MIDIPlaybackCounterPerInterrupt);

	SoundManager::setTargetPlaybackFrameRate(SoundManager::getInstance(), this->stageSpec->sound.pcmTargetPlaybackFrameRate);

	int i = 0;

	for(; this->stageSpec->assets.sounds[i]; i++)
	{
		SoundWrapper soundWrapper = SoundManager::getSound(SoundManager::getInstance(), this->stageSpec->assets.sounds[i], kPlayAll, (EventListener)Stage::onSoundWrapperReleased, Object::safeCast(this));

		if(!isDeleted(soundWrapper))
		{
			SoundWrapper::play(soundWrapper, NULL, kSoundWrapperPlaybackFadeIn);

			if(isDeleted(this->soundWrappers))
			{
				this->soundWrappers = new VirtualList();
			}

			VirtualList::pushBack(this->soundWrappers, soundWrapper);
		}
	}
}

void Stage::onSoundWrapperReleased(Object eventFirer __attribute__((unused)))
{
}

void Stage::setupTimer()
{
	HardwareManager::setupTimer(HardwareManager::getInstance(), this->stageSpec->timer.resolution, this->stageSpec->timer.timePerInterrupt, this->stageSpec->timer.timePerInterruptUnits);
}

bool Stage::handlePropagatedMessage(int message)
{
	if(this->uiContainer)
	{
		// propagate message to ui
		return Container::propagateMessage(this->uiContainer, Container::onPropagatedMessage, message);
	}

	return false;
}

void Stage::onFocusEntityDeleted(Object eventFirer __attribute__ ((unused)))
{
	this->focusEntity = NULL;

	if(this->focusEntity && Camera::getFocusEntity(Camera::getInstance()))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			Camera::setFocusGameEntity(Camera::getInstance(), NULL);
		}
	}
}

void Stage::setFocusEntity(Entity focusEntity)
{
	if(this->focusEntity)
	{
		Object::removeEventListener(this->focusEntity, Object::safeCast(this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);
	}

	this->focusEntity = focusEntity;

	if(this->focusEntity)
	{
		Object::addEventListener(this->focusEntity, Object::safeCast(this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);

		Vector3D focusEntityPosition = *Container::getGlobalPosition(this->focusEntity);
		focusEntityPosition.x = __METERS_TO_PIXELS(focusEntityPosition.x);
		focusEntityPosition.y = __METERS_TO_PIXELS(focusEntityPosition.y);
		focusEntityPosition.z = __METERS_TO_PIXELS(focusEntityPosition.z);

		this->cameraPreviousDistance = (long)focusEntityPosition.x * (long)focusEntityPosition.x +
											(long)focusEntityPosition.y * (long)focusEntityPosition.y +
											(long)focusEntityPosition.z * (long)focusEntityPosition.z;
	}
}

// get stage spec
StageSpec* Stage::getStageSpec()
{
	return this->stageSpec;
}

ParticleRemover Stage::getParticleRemover()
{
	return this->particleRemover;
}

void Stage::showStreamingProfiling(int x, int y)
{
	Printing::text(Printing::getInstance(), "STREAMING STATUS", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Stage's status", x, ++y, NULL);

	int originalY __attribute__ ((unused)) = y;
	int xDisplacement = 21;
	y++;

	Printing::text(Printing::getInstance(), "Registered entities:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->stageEntities), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Loaded entities:                ", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->loadedStageEntities), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Child entities:                 ", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->children), x + xDisplacement, y++, NULL);

#ifdef __PROFILE_STREAMING

	xDisplacement = 10;

	Printing::text(Printing::getInstance(), "Process duration (ms)", x, ++y, NULL);
	y++;

	Printing::text(Printing::getInstance(), "Unload:           ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), unloadOutOfRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Load:             ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), loadInRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Removing:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), processRemovedEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Factory:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), entityFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	processRemovedEntitiesHighestTime = 0;
	entityFactoryHighestTime = 0;
#endif
}
