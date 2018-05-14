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

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>
#include <Container.h>
#include <Shape.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ENTITY_PENDING_PROCESSING		0x00
#define __LIST_EMPTY					0x01
#define __ENTITY_PROCESSED				0x02


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines an entity in ROM memory
typedef struct EntityDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	/// sprites
	const SpriteDefinition** spriteDefinitions;

	/// collision shapes
	const ShapeDefinition* shapeDefinitions;

	/// pixelSize
	// if 0, width and height will be inferred from the first sprite's texture's pixelSize
	PixelSize pixelSize;

	/// object's in-game type
	u32 inGameType;

	/// physical specification
	PhysicalSpecification* physicalSpecification;

} EntityDefinition;

typedef const EntityDefinition EntityROMDef;


// an entity associated with a position
typedef const struct PositionedEntity
{
	// pointer to the entity definition in ROM
	EntityDefinition* entityDefinition;

	// position in the screen coordinates
	ScreenPixelVector onScreenPosition;

	// entity's id
	s16 id;

	// name
	char* name;

	// the children
	struct PositionedEntity* childrenDefinitions;

	// extra info
	void* extraInfo;

	// force load
	bool loadRegardlessOfPosition;

} PositionedEntity;

typedef const PositionedEntity PositionedEntityROMDef;


class EntityFactory : Object
{
	/* the EntityFactory entities to test for streaming */
	VirtualList entitiesToInstantiate;
	/* streaming's uninitialized entities */
	VirtualList entitiesToInitialize;
	/* streaming's non yet transformed entities */
	VirtualList entitiesToTransform;
	/* streaming's non yet transformed entities */
	VirtualList entitiesToMakeReady;
	/* entities loaded */
	VirtualList spawnedEntities;
	/* index for method to execute */
	int streamingPhase;

	void constructor();
	u32 prepareEntities();
	void prepareAllEntities();
	void spawnEntity(PositionedEntity* positionedEntity, Container parent, EventListener callback, s16 id);
	u32 hasEntitiesPending();
}


#endif
