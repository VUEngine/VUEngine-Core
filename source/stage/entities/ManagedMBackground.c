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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackground.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <MBgmapSprite.h>
#include <ManagedMBackground.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ManagedMBackground, MBackground);

__CLASS_FRIEND_DEFINITION(Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern const VBVec3D* _screenPosition;
extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ManagedMBackground, MBackgroundDefinition* definition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ManagedMBackground, definition, id, internalId, name);

// class's constructor
void ManagedMBackground_constructor(ManagedMBackground this, MBackgroundDefinition* definition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ManagedMBackground::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(MBackground, definition, id, internalId, name);

	// the sprite must be initialized in the derived class
	this->managedSprites = __NEW(VirtualList);

	this->previous2DPosition.x = 0;
	this->previous2DPosition.y = 0;
	this->previous2DPosition.z = 0;
	this->previous2DPosition.parallax = 0;
}

// class's destructor
void ManagedMBackground_destructor(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::destructor: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	// delete the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

static void ManagedMBackground_registerSprites(ManagedMBackground this, Entity child)
{
	ASSERT(this, "ManagedMBackground::registerSprites: null this");
	ASSERT(child, "ManagedMBackground::registerSprites: null child");

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
				ManagedMBackground_registerSprites(this, __SAFE_CAST(Entity, childNode->data));
			}
		}
	}
}

// transform class
void ManagedMBackground_initialTransform(ManagedMBackground this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "ManagedMBackground::initialTransform: null this");

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

void ManagedMBackground_ready(ManagedMBackground this, u32 recursive)
{
	ASSERT(this, "ManagedMBackground::ready: null this");

    Entity_ready(__SAFE_CAST(Entity, this), recursive);

	if(!VirtualList_getSize(this->managedSprites))
	{
	    ManagedMBackground_registerSprites(this, __SAFE_CAST(Entity, this));
    }
}

// transform class
void ManagedMBackground_transform(ManagedMBackground this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedMBackground::transform: null this");

	// allow normal transformation while not visible to avoid projection errors
	// at the initial transformation
//	if(!Entity_isVisible(__SAFE_CAST(Entity, this), 0, false) || Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	if(Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	{
		Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);
		this->invalidateGlobalTransformation = 0;

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

	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation ||
		this->children)
	{
		// call base class's transform method
		Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
	}

	// apply environment transform
	Container_applyEnvironmentToTransformation(__SAFE_CAST(Container, this), environmentTransform);

    this->updateSprites |= __UPDATE_SPRITE_POSITION;

	this->invalidateGlobalTransformation = 0;
}

void ManagedMBackground_updateVisualRepresentation(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::updateVisualRepresentation: null this");

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

// execute logic
void ManagedMBackground_update(ManagedMBackground this __attribute__ ((unused)), u32 elapsedTime __attribute__ ((unused)))
{
	ASSERT(this, "ManagedMBackground::update: null this");
}

int ManagedMBackground_passMessage(ManagedMBackground this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "ManagedMBackground::passMessage: null this");

	return false;
}

void ManagedMBackground_resume(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::resume: null this");

    MBackground_resume(__SAFE_CAST(MBackground, this));

    ManagedMBackground_registerSprites(this, __SAFE_CAST(Entity, this));
}

void ManagedMBackground_suspend(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::suspend: null this");

    MBackground_suspend(__SAFE_CAST(MBackground, this));

    VirtualList_clear(this->managedSprites);
}
