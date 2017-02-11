/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ManagedEntity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ManagedEntity
 * @extends Entity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(ManagedEntity, Entity);
__CLASS_FRIEND_DEFINITION(Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const VBVec3D* _screenDisplacement;
extern const Optical* _optical;

static void ManagedEntity_registerSprites(ManagedEntity this, Entity child);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ManagedEntity, ManagedEntityDefinition* managedEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ManagedEntity, managedEntityDefinition, id, internalId, name);

// class's constructor
void ManagedEntity_constructor(ManagedEntity this, ManagedEntityDefinition* managedEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ManagedEntity::constructor: null this");

	// construct base Entity
	__CONSTRUCT_BASE(Entity, (EntityDefinition*)managedEntityDefinition, id, internalId, name);

	// the sprite must be initialized in the derived class
	this->managedSprites = __NEW(VirtualList);

	this->previous2DPosition.x = 0;
	this->previous2DPosition.y = 0;
	this->previous2DPosition.z = 0;
	this->previous2DPosition.parallax = 0;
}

// class's destructor
void ManagedEntity_destructor(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::destructor: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	// destroy the super Entity
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

static void ManagedEntity_registerSprites(ManagedEntity this, Entity child)
{
	ASSERT(this, "ManagedEntity::registerSprites: null this");
	ASSERT(child, "ManagedEntity::registerSprites: null child");

	if(child)
	{
		if(child->sprites)
		{
			VirtualNode spriteNode = child->sprites->head;

			for(; spriteNode; spriteNode = spriteNode->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
				VirtualList_pushBack(this->managedSprites, sprite);
			}
		}

		if(child->children)
		{
			VirtualNode childNode = child->children->head;

			for(; childNode; childNode = childNode->next)
			{
				ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, childNode->data));
			}
		}
	}
}

// transform class
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "ManagedEntity::initialTransform: null this");

	Entity_initialTransform(__SAFE_CAST(Entity, this), environmentTransform, recursive);

	// save new global position
	VBVec3D position3D = this->transform.globalPosition;
	VBVec2D position2D;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	position2D.parallax = 0;

	this->previous2DPosition = position2D;

	this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;
}

void ManagedEntity_ready(ManagedEntity this, u32 recursive)
{
	ASSERT(this, "ManagedEntity::ready: null this");

	Entity_ready(__SAFE_CAST(Entity, this), recursive);

	if(!VirtualList_getSize(this->managedSprites))
	{
		ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, this));
	}
}

// transform class
void ManagedEntity_transform(ManagedEntity this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::transform: null this");

	// allow normal transformation while not visible to avoid projection errors
	// at the initial transformation
//	if(!Entity_isVisible(__SAFE_CAST(Entity, this), 0, false) || Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	if(Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	{
		Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);

		// save the 2d position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		position2D.parallax = 0;

		this->previous2DPosition = position2D;

		this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;

		return;
	}

	this->updateSprites = (_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z) || (__INVALIDATE_POSITION & this->invalidateGlobalTransformation)
		? __UPDATE_SPRITE_POSITION
		: 0;

	// call base class's transform method
	Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
}

void ManagedEntity_updateVisualRepresentation(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::updateVisualRepresentation: null this");

	if(this->updateSprites)
	{
		if(this->updateSprites & __UPDATE_SPRITE_SCALE)
		{
			Entity_updateVisualRepresentation(__SAFE_CAST(Entity, this));
		}

		// save new global position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;
		position2D.parallax = 0;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		VirtualNode spriteNode = this->managedSprites->head;

		VBVec2D displacement;

		displacement.x = position2D.x - this->previous2DPosition.x;
		displacement.y = position2D.y - this->previous2DPosition.y;
		displacement.z = 0;
		displacement.parallax = 0;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);

			__VIRTUAL_CALL(Sprite, addDisplacement, sprite, &displacement);
		}

		this->previous2DPosition = position2D;
	}

	this->updateSprites = 0;
}

int ManagedEntity_passMessage(ManagedEntity this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "ManagedEntity::passMessage: null this");

	return false;
}

void ManagedEntity_resume(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::resume: null this");

	Entity_resume(__SAFE_CAST(Entity, this));

	ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, this));
}

void ManagedEntity_suspend(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::suspend: null this");

	Entity_suspend(__SAFE_CAST(Entity, this));

	VirtualList_clear(this->managedSprites);
}
