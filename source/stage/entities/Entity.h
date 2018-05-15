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

class Entity : Container
{
	/**
	* @var EntityFactory		entityFactory
	* @brief					Entity factory
	* @memberof				Entity
	*/
	EntityFactory entityFactory;
	/**
	* @var VirtualList		 	sprites
	* @brief					Sprites list
	* @memberof				Entity
	*/
	VirtualList sprites;
	/**
	* @var VirtualList			shapes
	* @brief					Shapes for collision detection
	* @memberof				Entity
	*/
	VirtualList shapes;
	/**
	* @var Size				size
	* @brief					Used for collisions and streaming
	* @memberof				Entity
	*/
	Size size;
	/**
	* @var EntityDefinition*	entityDefinition
	* @brief					Entity's definition
	* @memberof				Entity
	*/
	EntityDefinition* entityDefinition;
	/**
	* @var Vector3D*			centerDisplacement
	* @brief					Center displacement
	* @memberof				Entity
	*/
	Vector3D* centerDisplacement;
	/**
	* @var s16				 	id
	* @brief					Entity's id, set by the user
	* @memberof				Entity
	*/
	s16 id;
	/**
	* @var s16				 	internalId
	* @brief					Entity's internal id, set by the engine
	* @memberof				Entity
	*/
	s16 internalId;
	/**
	* @var bool				invalidateSprites
	* @brief					Flag to update sprites' attributes
	* @memberof				Entity
	*/
	bool invalidateSprites;

	static Entity instantiate(const EntityDefinition* const entityDefinition, s16 id, s16 internalId, const char* const name, void* extraInfo);
	static Entity loadEntity(const PositionedEntity* const positionedEntity, s16 internalId);
	static Entity loadEntityDeferred(const PositionedEntity* const positionedEntity, s16 internalId);
	static PixelRightBox getTotalSizeFromDefinition(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition);
	static Vector3D* calculateGlobalPositionFromDefinitionByName(const struct PositionedEntity* childrenDefinitions, Vector3D environmentPosition, const char* childName);

	void constructor(Entity this, EntityDefinition* entityDefinition, s16 id, s16 internalId, const char* const name);
	void addChildEntities(Entity this, const PositionedEntity* childrenDefinitions);
	void addChildEntitiesDeferred(Entity this, const PositionedEntity* childrenDefinitions);
	Entity addChildEntity(Entity this, const EntityDefinition* entityDefinition, int internalId, const char* name, const Vector3D* position, void* extraInfo);
	bool addSpriteFromDefinitionAtIndex(Entity this, int spriteDefinitionIndex);
	void addSprites(Entity this, const SpriteDefinition** spritesDefinitions);
	u32 areAllChildrenInstantiated(Entity this);
	u32 areAllChildrenInitialized(Entity this);
	u32 areAllChildrenTransformed(Entity this);
	u32 areAllChildrenReady(Entity this);
	Entity getChildById(Entity this, s16 id);
	EntityDefinition* getEntityDefinition(Entity this);
	int getMapParallax(Entity this);
	s16 getId(Entity this);
	s16 getInternalId(Entity this);
	VirtualList getSprites(Entity this);
	void transformShapes(Entity this);
	void releaseSprites(Entity this);
	void setAnimation(Entity this, void (*animation)(Entity this));
	bool updateSpritePosition(Entity this);
	bool updateSpriteRotation(Entity this);
	bool updateSpriteScale(Entity this);
	void informShapesThatStartedMoving(Entity this);
	void informShapesThatStoppedMoving(Entity this);
	void disableShapes(Entity this);
	void activateShapes(Entity this, bool value);
	Direction getDirection(Entity this);
	void setDirection(Entity this, Direction direction);
	u32 getShapesLayers(Entity this);
	void setShapesLayers(Entity this, u32 layers);
	u32 getShapesLayersToIgnore(Entity this);
	void setShapesLayersToIgnore(Entity this, u32 layersToIgnore);
	virtual bool isVisible(Entity this, int pad, bool recursive);
	virtual void setExtraInfo(Entity this, void* extraInfo);
	virtual void initialize(Entity this, bool recursive);
	virtual void ready(Entity this, bool recursive);
	virtual bool respawn(Entity this);
	virtual u16 getAxisForFlipping(Entity this);
	virtual void setDefinition(Entity this, void* entityDefinition);
	virtual u16 getAxesForShapeSyncWithDirection(Entity this);
	override void iAmDeletingMyself(Entity this);
	override void initialTransform(Entity this, const Transformation* environmentTransform, u32 recursive);
	override void transform(Entity this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void setLocalPosition(Entity this, const Vector3D* position);
	override void setLocalRotation(Entity this, const Rotation* rotation);
	override void setupGraphics(Entity this);
	override void releaseGraphics(Entity this);
	override void synchronizeGraphics(Entity this);
	override bool handleMessage(Entity this, Telegram telegram);
	override const Vector3D* getPosition(Entity this);
	override const Rotation* getRotation(Entity this);
	override const Scale* getScale(Entity this);
	override fix10_6 getWidth(Entity this);
	override fix10_6 getHeight(Entity this);
	override fix10_6 getDepth(Entity this);
	override VirtualList getShapes(Entity this);
	override void suspend(Entity this);
	override void resume(Entity this);
	override bool isSubjectToGravity(Entity this, Acceleration gravity);
	override void show(Entity this);
	override void hide(Entity this);
	override fix10_6 getBounciness(Entity this);
	override fix10_6 getFrictionCoefficient(Entity this);
	override u32 getInGameType(Entity this);
}


#endif
