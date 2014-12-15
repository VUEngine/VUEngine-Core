/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Stage.h>
#include <Optics.h>
#include <SoundManager.h>
#include <Screen.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STREAM_CYCLE	(__TARGET_FPS >> 2)
#define __STREAM_UNLOAD_CYCLE	(0)
#define __STREAM_LOAD_CYCLE_1	__STREAM_CYCLE / 3
#define __STREAM_LOAD_CYCLE_2	(__STREAM_CYCLE / 3) * 2

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Stage
__CLASS_DEFINITION(Stage);

typedef struct StageEntityDescription
{
	PositionedEntity* positionedEntity;
	s16 id;
	long distance;

}StageEntityDescription;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern VBVec3D * _screenPosition;
extern VBVec3D * _screenDisplacement;

//class's constructor
static void Stage_constructor(Stage this);

// setup ui
static void Stage_setupUI(Stage this);

// register an entity in the streaming list
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this);

// delete removed entities
static void Stage_processRemovedEntities(Stage this);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively);

// preload textures
static void Stage_loadTextures(Stage this);

// load all visible entities
static void Stage_loadInRangeEntities(Stage this);

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this);

// load and retrieve a texture (for internal usage: use TextureManager_get)
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Stage)
__CLASS_NEW_END(Stage);


// class's constructor
static void Stage_constructor(Stage this)
{
	ASSERT(this, "Stage::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Container, __ARGUMENTS(-1));

	this->stageEntities = NULL;
	this->stageEntitiesToTest = NULL;
	this->loadedStageEntities = NULL;
	this->removedEntities = __NEW(VirtualList);

	this->ui = NULL;
	this->stageDefinition = NULL;

	this->flushCharGroups = true;
	this->focusEntity = NULL;
}

// class's destructor
void Stage_destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	if (this->ui)
{
		__DELETE(this->ui);
		this->ui = NULL;
	}

	if (this->stageEntities)
{
		VirtualNode node = VirtualList_begin(this->stageEntities);

		for (; node; node = VirtualNode_getNext(node))
{
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		__DELETE(this->stageEntities);

		this->stageEntities = NULL;
	}

	if (	this->stageEntitiesToTest)
{
		__DELETE(this->stageEntitiesToTest);
		this->stageEntitiesToTest = NULL;
	}

	if (	this->loadedStageEntities)
{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}

	if (this->removedEntities)
{
		VirtualNode node = VirtualList_begin(this->removedEntities);

		for (; node; node = VirtualNode_getNext(node))
{
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		__DELETE(this->removedEntities);

		this->removedEntities = NULL;
	}

	// destroy the super object
	__DESTROY_BASE(Container);
}

// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(Stage this, int *actor,int x,int y, int z)
{
	// TODO
	ASSERT(this, "Stage::setupObjActor: null this");
}

// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D* position3D, u8 width, u8 height)
{
	ASSERT(this, "Stage::inLoadRange: null this");

	Scale scale;

	scale.x = scale.y = FIX19_13TOFIX7_9(ITOFIX19_13(1) -
		       FIX19_13_DIV(position3D->z , _optical->maximunViewDistance));

	return Optics_isVisible(*position3D,
			Optics_calculateRealSize(((u16)width) << 3, WRLD_BGMAP, scale.x),
			Optics_calculateRealSize(((u16)height) << 3, WRLD_BGMAP, scale.y),
			Optics_calculateParallax(position3D->x, position3D->z),
			__ENTITY_LOAD_PAD);
}

// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities)
{
	ASSERT(this, "Stage::load: null this");

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());

	// set world's definition
	this->stageDefinition = stageDefinition;

	// set world's limits
	Screen_setStageSize(Screen_getInstance(), stageDefinition->size);

	// set screen's position
	Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);

	// preload textures
	Stage_loadTextures(this);

	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;

	// register all the entities in the stage's definition
	Stage_registerEntities(this);

	// load entities
	Stage_loadInRangeEntities(this);

	// retrieve focus entity for streaming
	this->focusEntity = (Entity)Screen_getFocusInGameEntity(Screen_getInstance());

	// setup ui
	Stage_setupUI(this);

	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	SoundManager_loadBGM(SoundManager_getInstance(), (u16 (*)[6])stageDefinition->bgm);

	//setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
}


// setup ui
static void Stage_setupUI(Stage this)
{
	ASSERT(this, "Stage::setupUI: null this");
	ASSERT(!this->ui, "Stage::setupUI: UI already exists");

	if (this->ui)
{
		__DELETE(this->ui);
		this->ui = NULL;
	}

	if (this->stageDefinition->uiDefinition.allocator)
{
		// call the appropiate allocator to support inheritance!
		this->ui = (UI)((UI (*)(UIDefinition*, ...)) this->stageDefinition->uiDefinition.allocator)(0, &this->stageDefinition->uiDefinition);
		ASSERT(this->ui, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if (this->ui)
{
			VBVec3D position = Screen_getPosition(Screen_getInstance());

			Transformation environmentTransform =
{
					// local position
					{0, 0, 0},
					// global position
					{0, 0, 0},
					// scale
					{1, 1},
					// rotation
					{0, 0, 0}
			};

			Container_setLocalPosition((Container)this->ui, position);

			__VIRTUAL_CALL(void, Container, initialTransform, (Container)this->ui, __ARGUMENTS(&environmentTransform));
		}
	}
}

// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D* position, void *extraInfo, int permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	static s16 id = 0;

	if (entityDefinition)
{
		Entity entity = Entity_load(entityDefinition, id++, extraInfo);

		// create the entity and add it to the world
		Container_addChild((Container)this, (Container)entity);

		// static to avoid call to _memcpy
		static Transformation environmentTransform =
{
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// scale
				{1, 1},
				// rotation
				{0, 0, 0}
		};

		// set spatial position
		__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(position));

		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, (Container)entity, __ARGUMENTS(&environmentTransform));

		if (permanent)
{
			// TODO
		}
		return entity;
	}

	return NULL;
}

// add entity to the stage
void Stage_removeEntity(Stage this, Entity entity, int permanent)
{
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(entity, "Stage::removeEntity: null entity");

	if (!entity)
{
		return;
	}

	// hide until effectively deleted
	Entity_hide(entity);

	VirtualList_pushBack(this->removedEntities, entity);

	VirtualNode node = VirtualList_begin(this->stageEntities);

	s16 id = Container_getId((Container)entity);

	for (; node; node = VirtualNode_getNext(node))
{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (stageEntityDescription->id == id)

{
			stageEntityDescription->id = -1;
			break;
		}
	}

	if (permanent)
{
		ASSERT(entity, "Stage::removeEntity: null node");

		VirtualList_removeElement(this->stageEntities, VirtualNode_getData(node));
	}
}

// preload textures
static void Stage_loadTextures(Stage this)
{
	ASSERT(this, "Entity::loadTextures: null this");

	int i = 0;

	for (; this->stageDefinition->textures[i]; i++)
{
		TextureManager_loadTexture(TextureManager_getInstance(), this->stageDefinition->textures[i], this->flushCharGroups);
	}
}

// register an entity in the streaming list
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity)
{
	ASSERT(this, "Stage::registerEntities: null this");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->id = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	return stageEntityDescription;
}

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this)
{
	ASSERT(this, "Stage::registerEntities: null this");

	if (this->stageEntities)

{
		__DELETE(this->stageEntities);
		this->stageEntities = NULL;
	}
	else
{
		this->stageEntities = __NEW(VirtualList);
	}

	if (this->stageEntitiesToTest)

{
		__DELETE(this->stageEntitiesToTest);
		this->stageEntitiesToTest = NULL;
	}
	else
{
		this->stageEntitiesToTest = __NEW(VirtualList);
	}

	if (this->loadedStageEntities)

{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}
	else
{
		this->loadedStageEntities = __NEW(VirtualList);
	}

	// register entities ordering them according to their distances to the origin
	// givin increasing weight (more distance) to the objects according to their
	// position in the stage's definition
	int weightIncrement = Math_squareRoot(2* __SCREEN_WIDTH * __SCREEN_WIDTH + __SCREEN_HEIGHT * __SCREEN_HEIGHT);
	int i = 0;
	for (;this->stageDefinition->entities[i].entityDefinition; i++)
{
		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities[i]);

		u8 width = 0;
		u8 height = 0;

		int i = 0;
		for (; i < stageEntityDescription->positionedEntity->entityDefinition->numberOfSprites; i++)

{
			const SpriteDefinition* spriteDefinition = &stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[i];

			if (spriteDefinition)

{
				if (spriteDefinition->textureDefinition)
{
					if (width < spriteDefinition->textureDefinition->cols << 3)
{
						width = spriteDefinition->textureDefinition->cols << 3;
					}

					if (height < spriteDefinition->textureDefinition->rows << 3)
{
						height = spriteDefinition->textureDefinition->rows << 3;
					}
				}
			}
		}

		stageEntityDescription->distance = (stageEntityDescription->positionedEntity->position.x - (width >> 1)) * (stageEntityDescription->positionedEntity->position.x - (width >> 1)) +
		(stageEntityDescription->positionedEntity->position.y - (height >> 1)) * (stageEntityDescription->positionedEntity->position.y - (height >> 1)) +
		stageEntityDescription->positionedEntity->position.z * stageEntityDescription->positionedEntity->position.z;

		VirtualNode auxNode = VirtualList_begin(this->stageEntities);

		for (; auxNode; auxNode = VirtualNode_getNext(auxNode))
{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

			if (stageEntityDescription->distance + weightIncrement * i > auxStageEntityDescription->distance)

{
				continue;
			}

			VirtualList_insertBefore(this->stageEntities, auxNode, stageEntityDescription);
			break;
		}

		if (!auxNode)

{
			VirtualList_pushBack(this->stageEntities, stageEntityDescription);
		}
	}
}

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively)
{
	ASSERT(this, "Stage::loadEntities: null this");

	VBVec3D focusEntityPosition = Container_getGlobalPosition((Container)this->focusEntity);
	focusEntityPosition.x = FIX19_13TOI(focusEntityPosition.x);
	focusEntityPosition.y = FIX19_13TOI(focusEntityPosition.y);
	focusEntityPosition.z = FIX19_13TOI(focusEntityPosition.z);

	long focusEntityDistance = focusEntityPosition.x * focusEntityPosition.x +
	focusEntityPosition.y * focusEntityPosition.y +
	focusEntityPosition.z * focusEntityPosition.z;

	static long previousFocusEntityDistance = 0;

	u8 direction = previousFocusEntityDistance <= focusEntityDistance;

	static VirtualNode savedNode = NULL;

	VirtualNode node = savedNode? savedNode: VirtualList_begin(this->stageEntities);
	int counter = 0;

	for (; node && counter < __STREAMING_AMPLITUDE / 4; node = direction? VirtualNode_getPrevious(node): VirtualNode_getNext(node), counter++);

	node = node? node: direction? VirtualList_begin(this->stageEntities): VirtualList_end(this->stageEntities);
	savedNode = NULL;

	int entityLoaded = false;

	for (counter = 0; node && (!savedNode || counter < __STREAMING_AMPLITUDE); node = direction? VirtualNode_getNext(node): VirtualNode_getPrevious(node), counter++)

{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (!savedNode)
{
			if (direction)
{
				if (focusEntityDistance < stageEntityDescription->distance)
{
					savedNode = node;
				}
			}
			else
{
				if (focusEntityDistance > stageEntityDescription->distance)
{
					savedNode = node;
				}
			}
		}

		if (loadProgressively && entityLoaded)
{
			if (savedNode)

{
				break;
			}

			continue;
		}

		if (0 > stageEntityDescription->id)
{
			VBVec3D position3D =
{
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};

			// if entity in load range
			if (!loadOnlyInRangeEntities || Stage_inLoadRange(this, &position3D,
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols,
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows))
{
				Entity entity = Stage_addEntity(this, stageEntityDescription->positionedEntity->entityDefinition, &position3D, stageEntityDescription->positionedEntity->extraInfo, false);
				stageEntityDescription->id = Container_getId((Container)entity);

				VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);

				entityLoaded = true;
			}
		}
	}

	previousFocusEntityDistance = focusEntityDistance;
}

// load all visible entities
static void Stage_loadInRangeEntities(Stage this)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	// need a temporal list to remove and delete entities
	VirtualNode node = VirtualList_begin(this->stageEntities);;

	for (; node; node = VirtualNode_getNext(node))
{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (-1 == stageEntityDescription->id)

{
			VBVec3D position3D =
{
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};

			// if entity in load range
			if (Stage_inLoadRange(this, &position3D,
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols,
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows))

{
				Entity entity = Stage_addEntity(this, stageEntityDescription->positionedEntity->entityDefinition, &position3D, stageEntityDescription->positionedEntity->extraInfo, false);
				stageEntityDescription->id = Container_getId((Container)entity);

				VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
			}
		}
	}
}

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if (!this->children)

{
		return;
	}

	CACHE_ENABLE;

	// need a temporal list to remove and delete entities
	VirtualNode node = VirtualList_begin(this->children);

	// check which actors must be unloaded
	for (; node; node = VirtualNode_getNext(node))
{
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);

		// if the entity isn't visible inside the view field, unload it
		if (!__VIRTUAL_CALL(int, Entity, isVisible, entity, __ARGUMENTS(__ENTITY_UNLOAD_PAD)))
{
			s16 id = Container_getId((Container)entity);

			VirtualNode auxNode = VirtualList_begin(this->loadedStageEntities);

			for (; auxNode; auxNode = VirtualNode_getNext(auxNode))
{
				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

				if (stageEntityDescription->id == id)

{
					stageEntityDescription->id = -1;

					VirtualList_removeElement(this->loadedStageEntities, stageEntityDescription);
					break;
				}
			}

			// register entity to remove
			VirtualList_pushBack(this->removedEntities, (const BYTE* const )entity);
		}
	}

	CACHE_DISABLE;
}

// execute stage's logic
static void Stage_processRemovedEntities(Stage this)
{
	ASSERT(this, "Stage::processRemovedEntities: null this");

	VirtualNode node = VirtualList_begin(this->removedEntities);

	for (; node; node = VirtualNode_getNext(node))
{
		// don't need to remove manually from children list
		// since the entity will do it by itself on its
		// destructor
		// destroy it
		__DELETE(VirtualNode_getData(node));
	}

	VirtualList_clear(this->removedEntities);
}

// execute stage's logic
void Stage_update(Stage this)
{
	ASSERT(this, "Stage::update: null this");

	Stage_processRemovedEntities(this);

	Container_update((Container)this);

	if (this->ui)
{
		Container_update((Container)this->ui);
	}
}

// stream entities according to screen's position
void Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

	// if the screen is moving
	static int load = __STREAM_CYCLE;
	if (!--load)
{
		// unload not visible objects
		Stage_unloadOutOfRangeEntities(this);

		load = __STREAM_CYCLE;
	}
	else if (__STREAM_LOAD_CYCLE_1 == load || __STREAM_LOAD_CYCLE_2 == load)
{
		if (VirtualList_getSize(this->removedEntities))
{
			Stage_processRemovedEntities(this);
		}
		else if (this->focusEntity)
{
			// load visible objects
			Stage_loadEntities(this, true, true);
		}
		else
{
			this->focusEntity = (Entity)Screen_getFocusInGameEntity(Screen_getInstance());
		}
	}
}

// stream entities according to screen's position
void Stage_streamAll(Stage this)
{
	// must make sure there are not pending entities for removal
	Stage_processRemovedEntities(this);
	Stage_unloadOutOfRangeEntities(this);
	Stage_loadInRangeEntities(this);
}

// if set to false, the char set memory is flushed when
// a char defintion is no longer used
// only useful to false when preloading textures
// otherwise it doesn't have any effect add flushing is the default
// behvior
void Stage_setFlushCharGroups(Stage this, int flushCharGroups)
{
	this->flushCharGroups = flushCharGroups;

	/*
	if (!flushCharGroups)

{
		if (this->stageDefinition->textures[0])
{
			this->flushCharGroups = flushCharGroups;
		}
	}
	else
{
		this->flushCharGroups = flushCharGroups;
	}
	*/
}

// retrieve ui
UI Stage_getUI(Stage this)
{
	return this->ui;
}
