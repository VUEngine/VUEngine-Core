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

#ifndef ENTITY_H_
#define ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <EntityFactory.h>
#include <BgmapSprite.h>
#include <ObjectSprite.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class Entity : Container
{
	// Entity factory
	EntityFactory entityFactory;
	// Sprites list
	VirtualList sprites;
	// Shapes for collision detection
	VirtualList shapes;
	// Used for collisions and streaming
	Size size;
	// Entity's definition
	EntityDefinition* entityDefinition;
	// Center displacement
	Vector3D* centerDisplacement;
	// Entity's id, set by the user
	s16 id;
	// Entity's internal id, set by the engine
	s16 internalId;
	// Flag to update sprites' attributes
	bool invalidateSprites;

	/// @publicsection
	static Entity instantiate(const EntityDefinition* const entityDefinition, s16 id, s16 internalId, const char* const name, void* extraInfo);
	static Entity loadEntity(const PositionedEntity* const positionedEntity, s16 internalId);
	static Entity loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 internalId);
	static PixelRightBox getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition);
	static Vector3D* calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, Vector3D environmentPosition, const char* childName);
	void constructor(EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name);
	void addChildEntities(const PositionedEntity* childrenDefinitions);
	void addChildEntitiesDeferred(const PositionedEntity* childrenDefinitions);
	Entity addChildEntity(const EntityDefinition* entityDefinition, int internalId, const char* name, const Vector3D* position, void* extraInfo);
	bool addSpriteFromDefinitionAtIndex(int spriteDefinitionIndex);
	void addSprites(const SpriteDefinition** spritesDefinitions);
	u32 areAllChildrenInstantiated();
	u32 areAllChildrenInitialized();
	u32 areAllChildrenTransformed();
	u32 areAllChildrenReady();
	Entity getChildById(s16 id);
	EntityDefinition* getEntityDefinition();
	int getMapParallax();
	s16 getId();
	s16 getInternalId();
	VirtualList getSprites();
	void transformShapes();
	void releaseSprites();
	void setAnimation(void (*animation)());
	bool updateSpritePosition();
	bool updateSpriteRotation();
	bool updateSpriteScale();
	void informShapesThatStartedMoving();
	void informShapesThatStoppedMoving();
	void disableShapes();
	void activateShapes(bool value);
	Direction getDirection();
	void setDirection(Direction direction);
	u32 getShapesLayers();
	void setShapesLayers(u32 layers);
	u32 getShapesLayersToIgnore();
	void setShapesLayersToIgnore(u32 layersToIgnore);
	virtual bool isVisible(int pad, bool recursive);
	virtual void setExtraInfo(void* extraInfo);
	virtual void initialize(bool recursive);
	virtual void ready(bool recursive);
	virtual bool respawn();
	virtual u16 getAxisForFlipping();
	virtual void setDefinition(void* entityDefinition);
	virtual u16 getAxesForShapeSyncWithDirection();
	override void iAmDeletingMyself();
	override void initialTransform(const Transformation* environmentTransform, u32 recursive);
	override void transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void setLocalPosition(const Vector3D* position);
	override void setLocalRotation(const Rotation* rotation);
	override void setupGraphics();
	override void releaseGraphics();
	override void synchronizeGraphics();
	override bool handleMessage(Telegram telegram);
	override const Vector3D* getPosition();
	override const Rotation* getRotation();
	override const Scale* getScale();
	override fix10_6 getWidth();
	override fix10_6 getHeight();
	override fix10_6 getDepth();
	override VirtualList getShapes();
	override void suspend();
	override void resume();
	override bool isSubjectToGravity(Acceleration gravity);
	override void show();
	override void hide();
	override fix10_6 getBounciness();
	override fix10_6 getFrictionCoefficient();
	override u32 getInGameType();
}


#endif
