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

#include <ManagedStaticImage.h>
#include <Optics.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ManagedStaticImage
 * @extends RecyclableImage
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(ManagedStaticImage, RecyclableImage);
__CLASS_FRIEND_DEFINITION(Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ManagedStaticImage, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ManagedStaticImage, definition, id, internalId, name);

// class's constructor
void ManagedStaticImage_constructor(ManagedStaticImage this, RecyclableImageDefinition* definition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ManagedStaticImage::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(RecyclableImage, definition, id, internalId, name);

	// the sprite must be initialized in the derived class
	this->managedSprites = __NEW(VirtualList);

	this->previous2DPosition.x = 0;
	this->previous2DPosition.y = 0;
	this->previous2DPosition.z = 0;
	this->previous2DPosition.parallax = 0;
}

// class's destructor
void ManagedStaticImage_destructor(ManagedStaticImage this)
{
	ASSERT(this, "ManagedStaticImage::destructor: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	// delete the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

static void ManagedStaticImage_registerSprites(ManagedStaticImage this, Entity child)
{
	ASSERT(this, "ManagedStaticImage::registerSprites: null this");
	ASSERT(child, "ManagedStaticImage::registerSprites: null child");

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
				ManagedStaticImage_registerSprites(this, __SAFE_CAST(Entity, childNode->data));
			}
		}
	}
}

// transform class
void ManagedStaticImage_initialTransform(ManagedStaticImage this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "ManagedStaticImage::initialTransform: null this");

	__CALL_BASE_METHOD(RecyclableImage, initialTransform, this, environmentTransform, recursive);

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

void ManagedStaticImage_ready(ManagedStaticImage this, bool recursive)
{
	ASSERT(this, "ManagedStaticImage::ready: null this");

	__CALL_BASE_METHOD(RecyclableImage, ready, this, recursive);

	if(!VirtualList_getSize(this->managedSprites))
	{
		ManagedStaticImage_registerSprites(this, __SAFE_CAST(Entity, this));
	}
}

// transform class
void ManagedStaticImage_transform(ManagedStaticImage this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedStaticImage::transform: null this");

	// allow normal transformation while not visible to avoid projection errors
	// at the initial transformation
//	if(!Entity_isVisible(__SAFE_CAST(Entity, this), 0, false) || Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	if(Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	{
		__CALL_BASE_METHOD(RecyclableImage, transform, this, environmentTransform);

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

	if((__INVALIDATE_POSITION & this->invalidateGlobalTransformation) |
		(u32)this->children)
	{
		// call base class's transform method
		Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
	}

	// apply environment transform
	Container_applyEnvironmentToTransformation(__SAFE_CAST(Container, this), environmentTransform);

	this->updateSprites |= __UPDATE_SPRITE_POSITION;

	this->invalidateGlobalTransformation = 0;
}

void ManagedStaticImage_updateVisualRepresentation(ManagedStaticImage this)
{
	ASSERT(this, "ManagedStaticImage::updateVisualRepresentation: null this");

	if(!this->updateSprites)
	{
		return;
	}

	if(this->updateSprites & __UPDATE_SPRITE_SCALE)
	{
		__CALL_BASE_METHOD(RecyclableImage, updateVisualRepresentation, this);
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

	this->updateSprites = 0;
}

void ManagedStaticImage_releaseGraphics(ManagedStaticImage this)
{
	ASSERT(this, "ManagedStaticImage::releaseGraphics: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	__CALL_BASE_METHOD(RecyclableImage, releaseGraphics, this);
}

// execute logic
void ManagedStaticImage_update(ManagedStaticImage this __attribute__ ((unused)), u32 elapsedTime __attribute__ ((unused)))
{
	ASSERT(this, "ManagedStaticImage::update: null this");
}

int ManagedStaticImage_passMessage(ManagedStaticImage this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "ManagedStaticImage::passMessage: null this");

	return false;
}

void ManagedStaticImage_resume(ManagedStaticImage this)
{
	ASSERT(this, "ManagedStaticImage::resume: null this");

	__CALL_BASE_METHOD(RecyclableImage, resume, this);

	ManagedStaticImage_registerSprites(this, __SAFE_CAST(Entity, this));
}

void ManagedStaticImage_suspend(ManagedStaticImage this)
{
	ASSERT(this, "ManagedStaticImage::suspend: null this");

	__CALL_BASE_METHOD(RecyclableImage, suspend, this);

	VirtualList_clear(this->managedSprites);
}
