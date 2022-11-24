/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Entity.h>
#include <Camera.h>
#include <VUEngine.h>
#include <Entity.h>
#include <Optics.h>
#include <Shape.h>
#include <CollisionManager.h>
#include <SpriteManager.h>
#include <BgmapSprite.h>
#include <MBgmapSprite.h>
#include <Mesh.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

static int16 _visibilityPadding = 0;

friend class VirtualNode;
friend class VirtualList;

#define ENTITY_SPRITE_DEPTH						16
#define ENTITY_SPRITE_HALF_DEPTH				(ENTITY_SPRITE_DEPTH >> 1)


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static void Entity::setVisibilityPadding(int16 visibilityPadding)
{
	_visibilityPadding = visibilityPadding;
}

/**
 * Class constructor
 *
 * @param entitySpec
 * @param id
 * @param internalId
 * @param name
 */
void Entity::constructor(EntitySpec* entitySpec, int16 internalId, const char* const name)
{
	// construct base Container
	Base::constructor(name);

	this->transform = Entity::overrides(this, transform);
	this->synchronizeGraphics = Entity::overrides(this, synchronizeGraphics);

	// set the ids
	this->internalId = internalId;

	// save spec
	this->entitySpec = entitySpec;

	// the sprite must be initialized in the derived class
	this->sprites = NULL;
	this->shapes = NULL;
	this->centerDisplacement = NULL;
	this->entityFactory = NULL;
	this->wireframes = NULL;
	this->inCameraRange = true;

	// initialize to 0 for the engine to know that size must be set
	this->size = Size::getFromPixelSize(entitySpec->pixelSize);

	this->invalidateGraphics = 0;
	this->transformShapes = true;
	this->allowCollisions = true;

	Entity::createBehaviors(this);
}

/**
 * Class destructor
 */
void Entity::destructor()
{
	Entity::destroyComponents(this);

	if(this->centerDisplacement)
	{
		delete this->centerDisplacement;
	}

	if(this->entityFactory)
	{
		delete this->entityFactory;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Clean up method
 */
void Entity::iAmDeletingMyself()
{
	Base::iAmDeletingMyself(this);

	Entity::destroyComponents(this);
}

void Entity::destroyComponents()
{
	Entity::destroyWireframes(this);
	Entity::destroySprites(this);
	Entity::destroyShapes(this);
}

/**
 * Retrieve instance's in game id
 *
 * @return		Internal ID
 */
int16 Entity::getInternalId()
{
	return this->internalId;
}

/**
 * Get child by id
 *
 * @param id
 * @return		Child Entity
 */
Entity Entity::getChildById(int16 id)
{
	if(this->children)
	{
		// look through all children
		for(VirtualNode node = this->children->head; NULL != node ; node = node->next)
		{
			Entity child = Entity::safeCast(node->data);

			if(child->internalId == id)
			{
				return !isDeleted(child) && !child->deleteMe ? child : NULL;
			}
		}
	}

	return NULL;
}

/**
 * Set spec
 *
 * @param entitySpec	EntitySpec
 */
void Entity::setSpec(void* entitySpec)
{
	// save spec
	this->entitySpec = entitySpec;
}

/**
 * Destroy shapes
 *
 * @private
 */
void Entity::destroyShapes()
{
	if(NULL != this->shapes)
	{
		ASSERT(!isDeleted(this->shapes), "Entity::setSpec: dead shapes");

		CollisionManager collisionManager = VUEngine::getCollisionManager(_vuEngine);

		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			CollisionManager::destroyShape(collisionManager, Shape::safeCast(node->data));
		}

		delete this->shapes;
		this->shapes = NULL;
	}
}

/**
 * Become subject to the governing streaming algorithm
 *
 * @private
 */
void Entity::streamOut()
{
	this->dontStreamOut = false;
}

/**
 * Create behaviors
 */
void Entity::createBehaviors()
{
	if(NULL == this->behaviors)
	{
		Entity::addBehaviors(this, this->entitySpec->behaviorSpecs, true);
	}
}

/**
 * Add behaviors
 *
 * @param spriteSpecs
 */
void Entity::addBehaviors(BehaviorSpec** behaviorSpecs, bool destroyOldBehaviors)
{
	if(NULL == behaviorSpecs)
	{
		return;
	}

	if(destroyOldBehaviors)
	{
		Entity::destroyBehaviors(this);
	}

	if(NULL == this->behaviors)
	{
		this->behaviors = new VirtualList();
	}

	// go through n behaviors in entity's spec
	for(int32 i = 0; behaviorSpecs[i]; i++)
	{
		Entity::addBehavior(this, Behavior::create(behaviorSpecs[i]));
		ASSERT(Behavior::safeCast(VirtualList::back(this->behaviors)), "Entity::addBehaviors: behavior not created");
	}
}

/**
 * Destroy behaviors
 *
 * @private
 */
void Entity::destroyBehaviors()
{
	if(!isDeleted(this->behaviors))
	{
		ASSERT(!isDeleted(this->behaviors), "Entity::destroyBehaviors: dead behaviors");

		VirtualList::deleteData(this->behaviors);
		delete this->behaviors;
		this->behaviors = NULL;
	}
}

/**
 * Create sprites
 */
void Entity::createSprites()
{
	// this method can be called multiple times so only add shapes
	// if not already done
	if(NULL == this->sprites)
	{
		Entity::addSprites(this, this->entitySpec->spriteSpecs, true);
	}
}

/**
 * Add sprites from a list of specs
 *
 * @param spriteSpecs
 */
void Entity::addSprites(SpriteSpec** spriteSpecs, bool destroyOldSprites)
{
	if(NULL == spriteSpecs || NULL == spriteSpecs[0])
	{
		return;
	}

	if(destroyOldSprites)
	{
		Entity::destroySprites(this);
	}

	if(NULL == this->sprites)
	{
		this->sprites = new VirtualList();
	}

	SpriteManager spriteManager = SpriteManager::getInstance();

	// go through n sprites in entity's spec
	for(int32 i = 0; NULL != spriteSpecs[i] && NULL != spriteSpecs[i]->allocator; i++)
	{
		VirtualList::pushBack(this->sprites, SpriteManager::createSprite(spriteManager, (SpriteSpec*)spriteSpecs[i], ListenerObject::safeCast(this)));
		ASSERT(Sprite::safeCast(VirtualList::back(this->sprites)), "Entity::addSprite: sprite not created");
	}

	this->synchronizeGraphics = this->synchronizeGraphics || !isDeleted(this->sprites);
	this->invalidateGraphics = __INVALIDATE_TRANSFORMATION;
}

/**
 * Delete all of the Entity's sprites
 *
 */
void Entity::destroySprites()
{
#ifndef __SHIPPING
#ifndef __RELEASE
	int32 lp = HardwareManager::getLinkPointer();
#endif
#endif

	if(!isDeleted(this->sprites))
	{
		SpriteManager spriteManager = SpriteManager::getInstance();

		// Must use a temporal list to prevent any race condition
		VirtualList sprites = this->sprites;
		this->sprites = NULL;

		for(VirtualNode node = sprites->head; NULL != node ; node = node->next)
		{

#ifndef __SHIPPING
			if(isDeleted(Sprite::safeCast(node->data)))
			{
				Printing::setDebugMode(Printing::getInstance());
				Printing::clear(Printing::getInstance());
				Printing::text(Printing::getInstance(), "Onwer type: ", 1, 26, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 12, 26, NULL);
#ifndef __RELEASE
				Printing::text(Printing::getInstance(), "Caller address: ", 1, 27, NULL);
				Printing::hex(Printing::getInstance(), lp, 1, 12, 8, NULL);
#endif
				Error::triggerException("Entity::destroySprites: trying to dispose dead sprite", NULL);		
			}
#endif
			NM_ASSERT(!isDeleted(Sprite::safeCast(node->data)), "Entity::destroySprites: trying to dispose dead sprite");
			SpriteManager::disposeSprite(spriteManager, Sprite::safeCast(node->data));
		}

		// delete the sprites
		delete sprites;
	}

	this->sprites = NULL;
}

/**
 * Add wireframes
 */
void Entity::createWireframes()
{
	// this method can be called multiple times so only add shapes
	// if not already done
	if(NULL == this->wireframes)
	{
		Entity::addWireframes(this, this->entitySpec->wireframeSpecs, true);
	}
}

/**
 * Add wireframes from specs list
 *
 * @private
 * @param wireframeSpecs		List of wireframes
 */ 
void Entity::addWireframes(WireframeSpec** wireframeSpecs, bool destroyOldWireframes)
{
	if(NULL == wireframeSpecs || NULL == wireframeSpecs[0])
	{
		return;
	}

	if(destroyOldWireframes)
	{
		Entity::destroyWireframes(this);
	}

	if(NULL == this->wireframes)
	{
		this->wireframes = new VirtualList();
	}

	for(int32 i = 0; NULL != wireframeSpecs[i] && NULL != wireframeSpecs[i]->allocator; i++)
	{
		Wireframe wireframe = ((Wireframe (*)(WireframeSpec*)) wireframeSpecs[i]->allocator)(wireframeSpecs[i]);
		Wireframe::setup(wireframe, Entity::getPosition(this), Entity::getRotation(this), Entity::getScale(this));
		VirtualList::pushBack(this->wireframes, wireframe);
	}

	this->synchronizeGraphics = this->synchronizeGraphics || !isDeleted(this->wireframes);
}

/**
 * Add a wireframe
 *
 * @private
 * @param wireframeSpecs		List of wireframes
 */
void Entity::addWireframe(Wireframe wireframe)
{
	if(isDeleted(wireframe))
	{
		return;
	}

	if(NULL == this->wireframes)
	{
		this->wireframes = new VirtualList();
	}

	Wireframe::setup(wireframe, Entity::getPosition(this), Entity::getRotation(this), Entity::getScale(this));
	VirtualList::pushBack(this->wireframes, wireframe);
}

/**
 * Destroy wireframes
 *
 * @private
 */
void Entity::destroyWireframes()
{
	if(!isDeleted(this->wireframes))
	{
		ASSERT(!isDeleted(this->wireframes), "Entity::destroyWireframes: dead wireframes");

		VirtualList::deleteData(this->wireframes);
		delete this->wireframes;
		this->wireframes = NULL;
	}
}

/**
 * Add shapes
 */
void Entity::createShapes()
{
	// this method can be called multiple times so only add shapes
	// if not already done
	if(NULL == this->shapes)
	{
		Entity::addShapes(this, this->entitySpec->shapeSpecs, true);
	}

	Entity::transformShapes(this);
}

/**
 * Add shapes from a list of specs
 *
 * @private
 * @param shapeSpecs		List of shapes
 */
void Entity::addShapes(ShapeSpec* shapeSpecs, bool destroyOldShapes)
{
	if(NULL == shapeSpecs)
	{
		return;
	}

	if(destroyOldShapes)
	{
		Entity::destroyShapes(this);
	}

	if(NULL == this->shapes)
	{
		this->shapes = new VirtualList();
	}

	CollisionManager collisionManager = VUEngine::getCollisionManager(_vuEngine);

	// go through n sprites in entity's spec
	for(int32 i = 0; shapeSpecs[i].allocator; i++)
	{
		Shape shape = CollisionManager::createShape(collisionManager, SpatialObject::safeCast(this), &shapeSpecs[i]);
		ASSERT(shape, "Entity::addShapes: sprite not created");
		VirtualList::pushBack(this->shapes, shape);
	}

	this->transformShapes = !isDeleted(this->shapes);
}

/**
 * Calculate size from children
 *
 * @private
 * @param pixelRightBox
 * @param environmentPosition
 */
void Entity::calculateSizeFromChildren(PixelRightBox* pixelRightBox, Vector3D environmentPosition)
{
	PixelVector pixelGlobalPosition = PixelVector::getFromVector3D(environmentPosition, 0);

	pixelGlobalPosition.x += __METERS_TO_PIXELS(this->transformation.localPosition.x);
	pixelGlobalPosition.y += __METERS_TO_PIXELS(this->transformation.localPosition.y);
	pixelGlobalPosition.z += __METERS_TO_PIXELS(this->transformation.localPosition.z);

	int16 left = 0;
	int16 right = 0;
	int16 top = 0;
	int16 bottom = 0;
	int16 front = 0;
	int16 back = 0;
	int16 halfWidth = 0;
	int16 halfHeight = 0;
	int16 halfDepth = ENTITY_SPRITE_HALF_DEPTH;

	if((!this->size.x || !this->size.y || !this->size.z) && (NULL != this->sprites || NULL != this->wireframes))
	{
		if(NULL != this->sprites)
		{
			for(VirtualNode spriteNode = this->sprites->head; spriteNode; spriteNode = spriteNode->next)
			{
				Sprite sprite = Sprite::safeCast(spriteNode->data);
				ASSERT(sprite, "Entity::calculateSizeFromChildren: null sprite");

				halfWidth = Sprite::getHalfWidth(sprite);
				halfHeight = Sprite::getHalfHeight(sprite);
				halfDepth = ENTITY_SPRITE_HALF_DEPTH;

				PixelVector spriteDisplacement = *Sprite::getDisplacement(sprite);

				if(left > -halfWidth + spriteDisplacement.x)
				{
					left = -halfWidth + spriteDisplacement.x;
				}

				if(right < halfWidth + spriteDisplacement.x)
				{
					right = halfWidth + spriteDisplacement.x;
				}

				if(top > -halfHeight + spriteDisplacement.y)
				{
					top = -halfHeight + spriteDisplacement.y;
				}

				if(bottom < halfHeight + spriteDisplacement.y)
				{
					bottom = halfHeight + spriteDisplacement.y;
				}

				if(front > -halfDepth + spriteDisplacement.z)
				{
					front = -halfDepth + spriteDisplacement.z;
				}

				if(back < halfDepth + spriteDisplacement.z)
				{
					back = halfDepth + spriteDisplacement.z;
				}
			}
		}

		if(NULL != this->wireframes)
		{
			for(VirtualNode wireframeNode = this->wireframes->head; wireframeNode; wireframeNode = wireframeNode->next)
			{
				Wireframe wireframe = Wireframe::safeCast(wireframeNode->data);
				ASSERT(wireframe, "Entity::calculateSizeFromChildren: null wireframe");

				PixelRightBox pixelRightBox = Wireframe::getPixelRightBox(wireframe);

				if(left > pixelRightBox.x0)
				{
					left = pixelRightBox.x0;
				}

				if(right < pixelRightBox.x1)
				{
					right = pixelRightBox.x1;
				}

				if(top > pixelRightBox.y0)
				{
					top = pixelRightBox.y0;
				}

				if(right < pixelRightBox.y1)
				{
					top = pixelRightBox.y1;
				}

				if(front > pixelRightBox.z0)
				{
					front = pixelRightBox.z0;
				}

				if(back < pixelRightBox.z1)
				{
					back = pixelRightBox.z1;
				}
			}
		}
	}
	else
	{
		right = __METERS_TO_PIXELS(this->size.x >> 1);
		left = -right;
		bottom = __METERS_TO_PIXELS(this->size.y >> 1);
		top = -bottom;
		back = __METERS_TO_PIXELS(this->size.z >> 1);
		front = -back;
	}

	if((0 == pixelRightBox->x0) | (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) | (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) | (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) | (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) | (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) | (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(this->children)
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Entity::calculateSizeFromChildren(childNode->data, pixelRightBox, Vector3D::getFromPixelVector(pixelGlobalPosition));
		}
	}
}

/**
 * Calculate my size based on me and my children
 *
 * @private
 */
void Entity::calculateSize()
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::calculateSizeFromChildren(this, &pixelRightBox, Vector3D::zero());

	Vector3D centerDisplacement =
	{
		__PIXELS_TO_METERS((pixelRightBox.x1 + pixelRightBox.x0) >> 1) - this->transformation.localPosition.x,
		__PIXELS_TO_METERS((pixelRightBox.y1 + pixelRightBox.y0) >> 1) - this->transformation.localPosition.y,
		__PIXELS_TO_METERS((pixelRightBox.z1 + pixelRightBox.z0) >> 1) - this->transformation.localPosition.z
	};

	if(centerDisplacement.x | centerDisplacement.y | centerDisplacement.z)
	{
		if(this->centerDisplacement)
		{
			delete this->centerDisplacement;
		}

		this->centerDisplacement = new Vector3D;
		*this->centerDisplacement = centerDisplacement;
	}

	this->size.x = __PIXELS_TO_METERS(pixelRightBox.x1 - pixelRightBox.x0);
	this->size.y = __PIXELS_TO_METERS(pixelRightBox.y1 - pixelRightBox.y0);
	this->size.z = __PIXELS_TO_METERS(pixelRightBox.z1 - pixelRightBox.z0);
}

/**
 * Get size from spec
 *
 * @private
 * @param positionedEntity		Function scope
 * @param environmentPosition
 * @param pixelRightBox
 */
static void Entity::getSizeFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition, PixelRightBox* pixelRightBox)
{
	ASSERT(positionedEntity, "Entity::getSizeFromSpec: null positionedEntity");
	ASSERT(positionedEntity->entitySpec, "Entity::getSizeFromSpec: null entitySpec");

	PixelVector pixelGlobalPosition = *environmentPosition;

	pixelGlobalPosition.x += positionedEntity->onScreenPosition.x;
	pixelGlobalPosition.y += positionedEntity->onScreenPosition.y;
	pixelGlobalPosition.z += positionedEntity->onScreenPosition.z;

	int16 left = 0;
	int16 right = 0;
	int16 top = 0;
	int16 bottom = 0;
	int16 front = 0;
	int16 back = 0;
	int16 halfWidth = 0;
	int16 halfHeight = 0;
	int16 halfDepth = ENTITY_SPRITE_HALF_DEPTH;

	if(0 != positionedEntity->entitySpec->pixelSize.x || 0 != positionedEntity->entitySpec->pixelSize.y || 0 != positionedEntity->entitySpec->pixelSize.z)
	{
		// TODO: there should be a class which handles special cases
		halfWidth = positionedEntity->entitySpec->pixelSize.x >> 1;
		halfHeight = positionedEntity->entitySpec->pixelSize.y >> 1;
		halfDepth = positionedEntity->entitySpec->pixelSize.z >> 1;

		left = -halfWidth;
		right = halfWidth;
		top = -halfHeight;
		bottom = halfHeight;
		front = -halfDepth;
		back = halfDepth;
	}
	else 
	{
		if(NULL != positionedEntity->entitySpec->spriteSpecs && NULL != positionedEntity->entitySpec->spriteSpecs[0])
		{
			int32 i = 0;

			for(; NULL != positionedEntity->entitySpec->spriteSpecs[i]; i++)
			{
				if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(positionedEntity->entitySpec->spriteSpecs[i]->allocator && ((MBgmapSpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i])->textureSpecs[0]))
				{
					MBgmapSpriteSpec* mBgmapSpriteSpec = (MBgmapSpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i];

					int32 j = 0;

					halfWidth = 0;
					halfHeight = 0;
					halfDepth = ENTITY_SPRITE_HALF_DEPTH;

					for(; mBgmapSpriteSpec->textureSpecs[j]; j++)
					{
						if(halfWidth < (int16)(mBgmapSpriteSpec->textureSpecs[j]->cols << 2))
						{
							halfWidth = mBgmapSpriteSpec->textureSpecs[j]->cols << 2;
						}

						if(halfHeight < (int16)(mBgmapSpriteSpec->textureSpecs[j]->rows << 2))
						{
							halfHeight = (int16)(mBgmapSpriteSpec->textureSpecs[j]->rows << 2);
						}
					}

					if(left > -halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x)
					{
						left = -halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x;
					}

					if(right < halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x)
					{
						right = halfWidth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.x;
					}

					if(top > -halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y)
					{
						top = -halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y;
					}

					if(bottom < halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y)
					{
						bottom = halfHeight + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.y;
					}

					if(front > mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z)
					{
						front = mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z;
					}

					if(back < halfDepth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z)
					{
						back = halfDepth + mBgmapSpriteSpec->bgmapSpriteSpec.spriteSpec.displacement.z;
					}

				}
				else if(NULL != positionedEntity->entitySpec->spriteSpecs[i]->textureSpec)
				{
					SpriteSpec* spriteSpec = (SpriteSpec*)positionedEntity->entitySpec->spriteSpecs[i];
					halfWidth = spriteSpec->textureSpec->cols << 2;
					halfHeight = spriteSpec->textureSpec->rows << 2;
					halfDepth = ENTITY_SPRITE_HALF_DEPTH;

					if(left > -halfWidth + spriteSpec->displacement.x)
					{
						left = -halfWidth + spriteSpec->displacement.x;
					}

					if(right < halfWidth + spriteSpec->displacement.x)
					{
						right = halfWidth + spriteSpec->displacement.x;
					}

					if(top > -halfHeight + spriteSpec->displacement.y)
					{
						top = -halfHeight + spriteSpec->displacement.y;
					}

					if(bottom < halfHeight + spriteSpec->displacement.y)
					{
						bottom = halfHeight + spriteSpec->displacement.y;
					}

					if(front > -halfDepth + spriteSpec->displacement.z)
					{
						front = -halfDepth + spriteSpec->displacement.z;
					}

					if(back < halfDepth + spriteSpec->displacement.z)
					{
						back = halfDepth + spriteSpec->displacement.z;
					}
				}
			}
		}

		if(NULL != positionedEntity->entitySpec->wireframeSpecs && NULL != positionedEntity->entitySpec->wireframeSpecs[0])
		{
			int32 i = 0;

			for(; NULL != positionedEntity->entitySpec->wireframeSpecs[i]; i++)
			{
				if(__TYPE(Mesh) == __ALLOCATOR_TYPE(positionedEntity->entitySpec->wireframeSpecs[i]->allocator && ((MeshSpec*)positionedEntity->entitySpec->wireframeSpecs[i])->segments[0]))
				{
					PixelRightBox pixelRightBox = Mesh::getPixelRightBoxFromSpec((MeshSpec*)positionedEntity->entitySpec->wireframeSpecs[i]);

					if(left > pixelRightBox.x0)
					{
						left = pixelRightBox.x0;
					}

					if(right < pixelRightBox.x1)
					{
						right = pixelRightBox.x1;
					}

					if(top > pixelRightBox.y0)
					{
						top = pixelRightBox.y0;
					}

					if(right < pixelRightBox.y1)
					{
						top = pixelRightBox.y1;
					}

					if(front > pixelRightBox.z0)
					{
						front = pixelRightBox.z0;
					}

					if(back < pixelRightBox.z1)
					{
						back = pixelRightBox.z1;
					}
				}
			}
		}
	}	

	if((0 == pixelRightBox->x0) | (pixelGlobalPosition.x + left < pixelRightBox->x0))
	{
		pixelRightBox->x0 = pixelGlobalPosition.x + left;
	}

	if((0 == pixelRightBox->x1) | (right + pixelGlobalPosition.x > pixelRightBox->x1))
	{
		pixelRightBox->x1 = right + pixelGlobalPosition.x;
	}

	if((0 == pixelRightBox->y0) | (pixelGlobalPosition.y + top < pixelRightBox->y0))
	{
		pixelRightBox->y0 = pixelGlobalPosition.y + top;
	}

	if((0 == pixelRightBox->y1) | (bottom + pixelGlobalPosition.y > pixelRightBox->y1))
	{
		pixelRightBox->y1 = bottom + pixelGlobalPosition.y;
	}

	if((0 == pixelRightBox->z0) | (pixelGlobalPosition.z + front < pixelRightBox->z0))
	{
		pixelRightBox->z0 = pixelGlobalPosition.z + front;
	}

	if((0 == pixelRightBox->z1) | (back + pixelGlobalPosition.z > pixelRightBox->z1))
	{
		pixelRightBox->z1 = back + pixelGlobalPosition.z;
	}

	if(NULL != positionedEntity->childrenSpecs)
	{
		int32 i = 0;
		for(; positionedEntity->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->childrenSpecs[i], &pixelGlobalPosition, pixelRightBox);
		}
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		int32 i = 0;
		for(; positionedEntity->entitySpec->childrenSpecs[i].entitySpec; i++)
		{
			Entity::getSizeFromSpec(&positionedEntity->entitySpec->childrenSpecs[i], &pixelGlobalPosition, pixelRightBox);
		}
	}
}

/**
 * Calculate total size from spec
 *
 * @param positionedEntity		Function scope
 * @param environmentPosition
 * @return						PixelRightBox
 */
static PixelRightBox Entity::getTotalSizeFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition)
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	Entity::getSizeFromSpec(positionedEntity, environmentPosition, &pixelRightBox);

	pixelRightBox.x0 = pixelRightBox.x0 - positionedEntity->onScreenPosition.x;
	pixelRightBox.x1 = pixelRightBox.x1 - positionedEntity->onScreenPosition.x;
	pixelRightBox.y0 = pixelRightBox.y0 - positionedEntity->onScreenPosition.y;
	pixelRightBox.y1 = pixelRightBox.y1 - positionedEntity->onScreenPosition.y;
	pixelRightBox.z0 = pixelRightBox.z0 - positionedEntity->onScreenPosition.z;
	pixelRightBox.z1 = pixelRightBox.z1 - positionedEntity->onScreenPosition.z;

	return pixelRightBox;
}

/**
 * Find child by name in given list
 *
 * @param childrenSpecs	Function scope
 * @param environmentPosition
 * @param childName
 * @return						Entity's global position
 */
static Vector3D* Entity::calculateGlobalPositionFromSpecByName(const struct PositionedEntity* childrenSpecs, Vector3D environmentPosition, const char* childName)
{
	ASSERT(childrenSpecs, "Entity::calculateGlobalPositionFromSpecByName: null positionedEntity");

	if(!childrenSpecs)
	{
		return NULL;
	}

	static Vector3D position;

	int32 i = 0;
	for(; childrenSpecs[i].entitySpec; i++)
	{
		if(!strncmp(childName, childrenSpecs[i].name, __MAX_CONTAINER_NAME_LENGTH))
		{
			position.x = environmentPosition.x + __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.x);
			position.y = environmentPosition.y + __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.y);
			position.z = environmentPosition.z + __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.z + childrenSpecs[i].onScreenPosition.zDisplacement);
			return &position;
		}

		if(childrenSpecs[i].childrenSpecs)
		{
			Vector3D concatenatedEnvironmentPosition = environmentPosition;
			concatenatedEnvironmentPosition.x += __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.x);
			concatenatedEnvironmentPosition.y += __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.y);
			concatenatedEnvironmentPosition.z += __PIXELS_TO_METERS(childrenSpecs[i].onScreenPosition.z + childrenSpecs[i].onScreenPosition.zDisplacement);

			Vector3D* position = Entity::calculateGlobalPositionFromSpecByName(childrenSpecs[i].childrenSpecs, concatenatedEnvironmentPosition, childName);

			if(position)
			{
				return position;
			}
		}
	}

	return NULL;
}

/**
 * Instantiate an Entity using the provided allocator
 *
 * @param entitySpec
 * @param id
 * @param internalId
 * @param name
 * @param extraInfo
 * @return					Entity instance
 */
static Entity Entity::instantiate(const EntitySpec* const entitySpec, int16 internalId, const char* const name, const PositionedEntity* const positionedEntity)
{
	NM_ASSERT(NULL != entitySpec, "Entity::instantiate: null spec");
	NM_ASSERT(NULL != entitySpec->allocator, "Entity::instantiate: no allocator defined");

	if(NULL == entitySpec || NULL == entitySpec->allocator)
	{
		return NULL;
	}

	// call the appropriate allocator to support inheritance
	Entity entity = ((Entity (*)(EntitySpec*, int16, const char* const)) entitySpec->allocator)((EntitySpec*)entitySpec, internalId, name);

	// process extra info
	if(positionedEntity->extraInfo)
	{
		Entity::setExtraInfo(entity, positionedEntity->extraInfo);
	}

	if(entitySpec->extraInfo)
	{
		Entity::setExtraInfo(entity, entitySpec->extraInfo);
	}

	return entity;
}

/**
 * Add children to the instance from the specs array
 *
 * @param childrenSpecs
 */
void Entity::addChildEntities(const PositionedEntity* childrenSpecs)
{
	if(!childrenSpecs)
	{
		return;
	}

	int32 i = 0;

	// go through n sprites in entity's spec
	for(; childrenSpecs[i].entitySpec; i++)
	{
		Entity child = Entity::loadEntity(&childrenSpecs[i], this->internalId + i + 1);
		ASSERT(child, "Entity::loadChildren: entity not loaded");

		// create the entity and add it to the world
		Entity::addChild(this, Container::safeCast(child));
	}
}

/**
 * Load an entity and instantiate all its children
 *
 * @param positionedEntity
 * @param internalId
 * @return					Entity
 */
static Entity Entity::loadEntity(const PositionedEntity* const positionedEntity, int16 internalId)
{
	NM_ASSERT(NULL != positionedEntity, "Entity::loadEntity: null positionedEntity");
	NM_ASSERT(NULL != positionedEntity->entitySpec, "Entity::loadEntity: null spec");
	NM_ASSERT(NULL != positionedEntity->entitySpec->allocator, "Entity::loadEntity: no allocator defined");

	if(NULL == positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity->entitySpec, internalId, positionedEntity->name, positionedEntity);
	ASSERT(entity, "Entity::loadFromSpec: entity not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	Entity::setLocalPosition(entity, &position);

	// add children if defined
	if(NULL != positionedEntity->childrenSpecs)
	{
		Entity::addChildEntities(entity, positionedEntity->childrenSpecs);
	}

	if(NULL != positionedEntity->entitySpec->childrenSpecs)
	{
		Entity::addChildEntities(entity, positionedEntity->entitySpec->childrenSpecs);
	}

	return entity;
}

/**
 * Add children to instance from the specs array, but deferred
 *
 * @param childrenSpecs
 */
void Entity::addChildEntitiesDeferred(const PositionedEntity* childrenSpecs)
{
	ASSERT(childrenSpecs, "Entity::addChildEntitiesDeferred: null childrenSpecs");

	if(!childrenSpecs)
	{
		return;
	}

	if(!this->entityFactory)
	{
		this->entityFactory = new EntityFactory();
	}

	int32 i = 0;

	// go through n sprites in entity's spec
	for(; childrenSpecs[i].entitySpec; i++)
	{
		EntityFactory::spawnEntity(this->entityFactory, &childrenSpecs[i], Container::safeCast(this), NULL, this->internalId + Entity::getChildCount(this));
	}
}

/**
 * Load an entity and instantiate all its children, deferred
 *
 * @param positionedEntity
 * @param internalId
 * @return					Entity
 */
static Entity Entity::loadEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId)
{
	NM_ASSERT(NULL != positionedEntity, "Entity::loadEntityDeferred: null positionedEntity");
	NM_ASSERT(NULL != positionedEntity->entitySpec, "Entity::loadEntityDeferred: null spec");
	NM_ASSERT(NULL != positionedEntity->entitySpec->allocator, "Entity::loadEntityDeferred: no allocator defined");

	if(!positionedEntity)
	{
		return NULL;
	}

	Entity entity = Entity::instantiate(positionedEntity->entitySpec, internalId, positionedEntity->name, positionedEntity);

	NM_ASSERT(!isDeleted(entity), "Entity::loadEntityDeferred: entity not loaded");

	Vector3D position = Vector3D::getFromScreenPixelVector(positionedEntity->onScreenPosition);

	// set spatial position
	Entity::setLocalPosition(entity, &position);

	// add children if defined
	if(positionedEntity->childrenSpecs)
	{
		Entity::addChildEntitiesDeferred(entity, positionedEntity->childrenSpecs);
	}

	if(positionedEntity->entitySpec->childrenSpecs)
	{
		Entity::addChildEntitiesDeferred(entity, positionedEntity->entitySpec->childrenSpecs);
	}

	return entity;
}

/**
 * Add child entity from spec
 *
 * @param entitySpec
 * @param internalId
 * @param name
 * @param position
 * @param extraInfo
 * @return					Entity
 */
Entity Entity::addChildEntity(const EntitySpec* entitySpec, int16 internalId, const char* name, const Vector3D* position, void* extraInfo)
{
	ASSERT(entitySpec, "Entity::addChildEntity: null entitySpec");

	if(!entitySpec)
	{
		return NULL;
	}

	PixelVector pixelPosition = NULL != position ? PixelVector::getFromVector3D(*position, 0) : PixelVector::zero();

	ScreenPixelVector screenPixelVector =
	{
		pixelPosition.x,
		pixelPosition.y,
		pixelPosition.z,
		0
	};

	PositionedEntity positionedEntity =
	{
		(EntitySpec*)entitySpec,
		screenPixelVector,
		this->internalId + Entity::getChildCount(this),
		(char*)name,
		NULL,
		extraInfo,
		false
	};

	// load child entity
	Entity childEntity = Entity::loadEntity(&positionedEntity, 0 > internalId ? internalId : positionedEntity.id);
	ASSERT(childEntity, "Entity::addChildEntity: childEntity no created");

	// create the entity and add it to the world
	Entity::addChild(this, Container::safeCast(childEntity));

	// apply transformations
	Transformation environmentTransform = Entity::getEnvironmentTransform(this);
	Entity::concatenateTransform(this, &environmentTransform, &this->transformation);
	Entity::initialTransform(childEntity, &environmentTransform, true);

	// make ready
	Entity::ready(childEntity, true);

	return childEntity;
}

/**
 * Are all children instantiated?
 *
 * @return		Boolean whether all children are instantiated
 */
uint32 Entity::areAllChildrenInstantiated()
{
	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory::instantiateEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children transformed?
 *
 * @return		Boolean whether all children are transformed
 */
uint32 Entity::areAllChildrenTransformed()
{
	if(this->entityFactory)
	{
		return __LIST_EMPTY == EntityFactory::transformEntities(this->entityFactory);
	}

	return true;
}

/**
 * Are all children ready?
 *
 * @return		Boolean whether all children are ready
 */
uint32 Entity::areAllChildrenReady()
{
	if(this->entityFactory)
	{
		uint32 returnValue = __LIST_EMPTY == EntityFactory::makeReadyEntities(this->entityFactory);

		if(!EntityFactory::hasEntitiesPending(this->entityFactory))
		{
			delete this->entityFactory;
			this->entityFactory = NULL;

			// must force size calculation now that all children are loaded
			Entity::calculateSize(this);
		}

		return returnValue;
	}

	Entity::calculateSize(this);

	return true;
}

/**
 * Set shape's position
 *
 * @private
 */
void Entity::transformShape(Shape shape, const Vector3D* myPosition, const Rotation* myRotation, const Scale* myScale, Direction currentDirection __attribute__((unused)), int32 shapeSpecIndex)
{
	if(shape)
	{
		if(this->entitySpec->shapeSpecs && 0 <= shapeSpecIndex && this->entitySpec->shapeSpecs[shapeSpecIndex].allocator)
    	{
			const ShapeSpec* shapeSpecs = this->entitySpec->shapeSpecs;

			//uint16 axisForShapeSyncWithDirection = Entity::getAxisForShapeSyncWithDirection(this);

			Vector3D shapeDisplacement = Vector3D::rotate(Vector3D::getFromPixelVector(shapeSpecs[shapeSpecIndex].displacement), this->transformation.localRotation);

			Vector3D shapePosition = Vector3D::sum(*myPosition, shapeDisplacement);
			/*
			{
				myPosition->x + ((__X_AXIS & axisForShapeSyncWithDirection) && __LEFT == currentDirection.x ? -shapeDisplacement.x : shapeDisplacement.x),
				myPosition->y + ((__Y_AXIS & axisForShapeSyncWithDirection) && __UP == currentDirection.y ? -shapeDisplacement.y : shapeDisplacement.y),
				myPosition->z + ((__Z_AXIS & axisForShapeSyncWithDirection) && __NEAR == currentDirection.z ? -shapeDisplacement.z : shapeDisplacement.z),
			};
			*/

			Rotation shapeRotation = Rotation::sum(*myRotation, Rotation::getFromPixelRotation(shapeSpecs[shapeSpecIndex].pixelRotation));

			Scale shapeScale = Scale::product(*myScale, shapeSpecs[shapeSpecIndex].scale);

			Size size = Size::getFromPixelSize(shapeSpecs[shapeSpecIndex].pixelSize);

			Shape::position(shape, &shapePosition, &shapeRotation, &shapeScale, &size);
		}
		else
		{
			Shape::position(shape, myPosition, myRotation, myScale, &this->size);
		}
	}
}

/**
 * Set shape's position
 *
 * @private
 */
void Entity::transformShapes()
{
	if(this->shapes && this->transformShapes)
	{
		// setup shape
		const Vector3D* myPosition = Entity::getPosition(this);
		const Rotation* myRotation = Entity::getRotation(this);
		const Scale* myScale = Entity::getScale(this);

		Direction currentDirection = Entity::getDirection(this);

		if(this->entitySpec->shapeSpecs)
    	{
			const ShapeSpec* shapeSpecs = this->entitySpec->shapeSpecs;
			int32 i = 0;

			for(VirtualNode node = this->shapes->head; node && shapeSpecs[i].allocator; node = node->next, i++)
			{
				Shape shape = Shape::safeCast(node->data);

				Entity::transformShape(this, shape, myPosition, myRotation, myScale, currentDirection, i);
			}
		}
		else
		{
			for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
			{
				Shape shape = Shape::safeCast(node->data);

				Entity::transformShape(this, shape, myPosition, myRotation, myScale, currentDirection, -1);
			}
		}
	}
}

bool Entity::transformShapeAtSpecIndex(int32 shapeSpecIndex)
{
	if(!this->entitySpec->shapeSpecs)
	{
		return false;
	}

	if(!this->entitySpec->shapeSpecs[shapeSpecIndex].allocator)
	{
		return false;
	}

	if(this->shapes && 0 <= shapeSpecIndex && NULL != VirtualList::begin(this->shapes))
	{
		Shape shape = Shape::safeCast(VirtualList::getObjectAtPosition(this->shapes, shapeSpecIndex));

		if(!isDeleted(shape))
		{
			Entity::transformShape(this, shape, Entity::getPosition(this), Entity::getRotation(this), Entity::getScale(this), Entity::getDirection(this), shapeSpecIndex);
		}

		return true;
	}

	return false;
}

/**
 * Process extra info in initialization
 *
 * @param extraInfo
 */
void Entity::setExtraInfo(void* extraInfo __attribute__ ((unused)))
{
}

void Entity::updateSprites(uint32 updatePosition, uint32 updateScale, uint32 updateRotation, uint32 updateProjection)
{
	updatePosition |= updateRotation;
	updatePosition |= updateProjection;
	updateScale |= updateRotation;	

	if(this->entitySpec->useZDisplacementInProjection)
	{
		Entity::perSpriteUpdateSprites(this, updatePosition, updateScale, updateRotation);
	}
	else
	{
		Entity::condensedUpdateSprites(this, updatePosition, updateScale, updateRotation);
	}
}

void Entity::perSpriteUpdateSprites(uint32 updatePosition, uint32 updateScale, uint32 updateRotation)
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	Vector3D relativeGlobalPosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation.globalPosition), *_cameraInvertedRotation);

	for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(updatePosition)
		{
			Vector3D position = relativeGlobalPosition;
			position.z += __PIXELS_TO_METERS(Sprite::getDisplacement(sprite)->z);

			PixelVector projectedPosition = Vector3D::projectToPixelVector(position, Optics::calculateParallax(position.z));
			projectedPosition.z = __METERS_TO_PIXELS(relativeGlobalPosition.z);

			// update sprite's 2D position
			Sprite::setPosition(sprite, &projectedPosition);
		}

		if(updateRotation)
		{
			// update sprite's 2D rotation
			Sprite::rotate(sprite, &this->transformation.localRotation);
		}
		
		if(updateScale)
		{
			// calculate the scale
			Sprite::resize(sprite, this->transformation.globalScale, relativeGlobalPosition.z);
		}
	}
}

void Entity::condensedUpdateSprites(uint32 updatePosition, uint32 updateScale, uint32 updateRotation)
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	Vector3D relativeGlobalPosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation.globalPosition), *_cameraInvertedRotation);
	PixelVector position = Vector3D::projectToPixelVector(relativeGlobalPosition, Optics::calculateParallax(relativeGlobalPosition.z));

	for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(updatePosition)
		{
			// update sprite's 2D position
			Sprite::setPosition(sprite, &position);
		}

		if(updateRotation)
		{
			// update sprite's 2D rotation
			Sprite::rotate(sprite, &this->transformation.localRotation);
		}

		if(updateScale)
		{
			// calculate the scale
			Sprite::resize(sprite, this->transformation.globalScale, relativeGlobalPosition.z);
		}
	}
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param recursive
 */
void Entity::initialTransform(const Transformation* environmentTransform, uint32 recursive)
{
	// call base class's transformation method
	Base::initialTransform(this, environmentTransform, recursive);

	Entity::createSprites(this);
	Entity::createWireframes(this);
	Entity::createShapes(this);
	Entity::synchronizeGraphics(this);

	if(this->hidden)
	{
		Entity::hide(this);
	}

	// now can calculate the size
	if(recursive && (!this->size.x || !this->size.y || !this->size.z))
	{
		// must force size calculation now
		Entity::calculateSize(this);
	}
}

/**
 * Transform class
 *
 * @param environmentTransform
 */
void Entity::transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag)
{
	uint8 invalidateGraphics = 0;

	if(!isDeleted(this->sprites))
	{
		invalidateGraphics = invalidateTransformationFlag | this->invalidateGlobalTransformation;
	}
	
	if(this->invalidateGlobalTransformation)
	{
		Base::transform(this, environmentTransform, invalidateTransformationFlag);

		Entity::transformShapes(this);
	}
	else if(NULL != this->children)
	{
		Entity::transformChildren(this, invalidateTransformationFlag);
	}
	
	this->invalidateGraphics |= invalidateGraphics;
}

/**
 * Set local position
 */
void Entity::setLocalPosition(const Vector3D* position)
{
	Base::setLocalPosition(this, position);

	Entity::transformShapes(this);
}

/**
 * Set local rotation
 */
void Entity::setLocalRotation(const Rotation* rotation)
{
	Base::setLocalRotation(this, rotation);

	Entity::transformShapes(this);
}

/**
 * Update visual representation
 */
void Entity::synchronizeGraphics()
{
	if(!this->transformed)
	{
		return;
	}

	if(NULL != this->children)
	{
		Base::synchronizeGraphics(this);
	}

	if(!isDeleted(this->sprites) && !this->hidden)
	{
		Entity::updateSprites(this, this->invalidateGraphics & __INVALIDATE_POSITION, this->invalidateGraphics & __INVALIDATE_SCALE, this->invalidateGraphics & __INVALIDATE_ROTATION, this->invalidateGraphics & __INVALIDATE_PROJECTION);
	}

	this->invalidateGraphics = false;

	this->inCameraRange = this->dontStreamOut;

	if(!this->inCameraRange)
	{
		Entity::computeIfInCameraRange(this, _visibilityPadding, true);
	}
}

/**
 * Retrieve EntitySpec
 *
 * @return		EntitySpec
 */
EntitySpec* Entity::getSpec()
{
	return this->entitySpec;
}

/**
 * Retrieve rotation
 *
 * @return		Global rotation
 */
const Rotation* Entity::getRotation()
{
	return &this->transformation.globalRotation;
}

/**
 * Retrieve scale
 *
 * @return		Global position
 */
const Scale* Entity::getScale()
{
	return &this->transformation.globalScale;
}

/**
 * Retrieve sprites
 *
 * @return		VirtualList of Entity's sprites
 */
VirtualList Entity::getSprites()
{
	return this->sprites;
}

/**
 * Retrieve wireframes
 *
 * @return		VirtualList of Entity's wireframes
 */
VirtualList Entity::getWireframes()
{
	return this->wireframes;
}

/**
 * Handles incoming messages
 *
 * @param telegram
 * @return			True if successfully processed, false otherwise
 */
bool Entity::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

/**
 * Get width
 *
 * @return		Entity's width
 */
fixed_t Entity::getWidth()
{
	if(0 == this->size.x)
	{
		Entity::calculateSize(this);
	}

	// must calculate based on the scale because not affine container must be enlarged
	return this->size.x;
}

/**
 * Get height
 *
 * @return		Entity's height
 */
fixed_t Entity::getHeight()
{
	if(0 == this->size.y)
	{
		Entity::calculateSize(this);
	}

	return this->size.y;
}

/**
 * Get depth
 *
 * @return		Entity's depth
 */
fixed_t Entity::getDepth()
{
	if(0 == this->size.z)
	{
		Entity::calculateSize(this);
	}

	// must calculate based on the scale because not affine object must be enlarged
	return this->size.z;
}

void Entity::setSize(Size size)
{
	this->size = size;
}

bool Entity::isSpriteVisible(Sprite sprite, int32 pad)
{
	PixelVector spritePosition = Sprite::getDisplacedPosition(sprite);

	PixelSize pixelSize = PixelSize::getFromSize(this->size);

	int16 halfWidth	= pixelSize.x >> 1;
	int16 halfHeight = pixelSize.y >> 1;
	int16 halfDepth	= pixelSize.z >> 1;

	int32 x = spritePosition.x;
	int32 y = spritePosition.y;
	int32 z = spritePosition.z;

	pad += __ABS(z);

	// check x visibility
	if((x + halfWidth < _cameraFrustum->x0 - pad) || (x - halfWidth > _cameraFrustum->x1 + pad))
	{
		return false;
	}

	// check y visibility
	if((y + halfHeight < _cameraFrustum->y0 - pad) || (y - halfHeight > _cameraFrustum->y1 + pad))
	{
		return false;
	}

	// check z visibility
	if((z + halfDepth < _cameraFrustum->z0 - pad) || (z - halfDepth > _cameraFrustum->z1 + pad))
	{
		return false;
	}

	return true;
}

/**
 * Whether it is visible
 *
 * @param pad
 * @param recursive
 * @return			Boolean if visible
 */
bool Entity::isInCameraRange()
{
	return this->inCameraRange;
}

void Entity::computeIfInCameraRange(int32 pad, bool recursive)
{
	this->inCameraRange = false;

	if(this->sprites && this->sprites->head)
	{
		bool areSpritesVisible = false;

		for(VirtualNode spriteNode = this->sprites->head; !this->inCameraRange && spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = Sprite::safeCast(spriteNode->data);

			if(Sprite::isVisible(sprite))
			{
				this->inCameraRange = areSpritesVisible = true;
				break;
			}

			if(Entity::isSpriteVisible(this, sprite, pad))
			{
				this->inCameraRange = areSpritesVisible = true;
				break;
			}
		}
	}
	else
	{
		Vector3D position3D = Vector3D::getRelativeToCamera(this->transformation.globalPosition);

		if(this->centerDisplacement)
		{
			position3D = Vector3D::sum(position3D, *this->centerDisplacement);
		}

		position3D = Vector3D::rotate(position3D, *_cameraInvertedRotation);
		PixelVector position2D = PixelVector::getFromVector3D(position3D, 0);

		PixelVector size = PixelVector::getFromVector3D(Vector3D::rotate((Vector3D){this->size.x >> 1, this->size.y >> 1, this->size.z >> 1}, *_cameraInvertedRotation), 0);

		size.x = __ABS(size.x);
		size.y = __ABS(size.y);
		size.z = __ABS(size.z);

		this->inCameraRange = true;

		int32 helperPad = pad + __ABS(position2D.z);

		// check x visibility
		if(position2D.x + size.x < _cameraFrustum->x0 - helperPad || position2D.x - size.x > _cameraFrustum->x1 + helperPad)
		{
			this->inCameraRange = false;
		}
		// check y visibility
		else if(position2D.y + size.y < _cameraFrustum->y0 - helperPad || position2D.y - size.y > _cameraFrustum->y1 + helperPad)
		{
			this->inCameraRange = false;
		}
		// check z visibility
		else if(position2D.z + size.z < _cameraFrustum->z0 - pad || position2D.z - size.z > _cameraFrustum->z1 + pad)
		{
			this->inCameraRange = false;
		}
	}

	if(!this->inCameraRange && recursive && NULL != this->children)
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Entity child = Entity::safeCast(VirtualNode::getData(childNode));

			if(child->hidden)
			{
				continue;
			}

			Entity::computeIfInCameraRange(child, pad, true);

			if(Entity::isInCameraRange(child))
			{
				this->inCameraRange = true;
				break;
			}
		}
	}

	if(!isDeleted(this->shapes))
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape::setVisible(Shape::safeCast(node->data), this->inCameraRange);
		}			
	}
}

/**
 * Retrieve shapes list
 *
 * @return		Entity's Shape list
 */
VirtualList Entity::getShapes()
{
	return this->shapes;
}

/**
 * Make it visible
 */
void Entity::show()
{
	Base::show(this);

	// show all sprites
	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::show(node->data);
		}
	}

	// show all wireframes
	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::show(node->data);
		}
	}
}

/**
 * Make it invisible
 */
void Entity::hide()
{
	// This is called from the initialTransformation method
	// causing a call to transform that messes up the localPosition
	Base::hide(this);

	// hide all sprites
	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::hide(node->data);
		}
	}

	// hide all wireframes
	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::hide(node->data);
		}
	}
}

/**
 * Suspend for pause
 */
void Entity::suspend()
{
	Base::suspend(this);

	Entity::destroySprites(this);
	Entity::destroyWireframes(this);
}

/**
 * Resume after pause
 */
void Entity::resume()
{
	Base::resume(this);

	// initialize sprites
	if(NULL != this->entitySpec)
	{
		Entity::createSprites(this);
		Entity::createWireframes(this);
	}
	else
	{
		// force update sprites on next game's cycle
		this->invalidateGraphics = this->invalidateGlobalTransformation;
	}

	if(this->hidden)
	{
		// Force syncronization even if hidden
		this->hidden = false;
		Entity::synchronizeGraphics(this);
		Entity::hide(this);
	}
	else
	{
		Entity::synchronizeGraphics(this);
	}
}

/**
 * Defaults to true
 *
 * @param acceleration
 * @return				Defaults to true
 */
bool Entity::isSubjectToGravity(Acceleration gravity __attribute__ ((unused)))
{
	return true;
}

/**
 * Get in game type
 *
 * @return		Type of entity within the game's logic
 */
uint32 Entity::getInGameType()
{
	return this->entitySpec->inGameType;
}

/**
 * Get bounciness for physics
 *
 * @return		Bounciness
 */
fixed_t Entity::getBounciness()
{
	return this->entitySpec->physicalSpecification ? this->entitySpec->physicalSpecification->bounciness : 0;
}

/**
 * Get friction
 *
 * @return		Friction
 */
fixed_t Entity::getFrictionCoefficient()
{
	return this->entitySpec->physicalSpecification ? this->entitySpec->physicalSpecification->frictionCoefficient : 0;
}

/**
 * Propagate that movement started to the shapes
 *
 *@para Active status
 */
void Entity::activeCollisionChecks(bool active)
{
	this->allowCollisions |= active;

	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::activeCollisionChecks(shape, active);
		}
	}
}

/**
 * Set whether shapes must register shapes against which they have collided
 * in order to receive update and exit collision notifications
 */
void Entity::registerCollisions(bool value)
{
	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::registerCollisions(shape, value);
		}
	}
}

/**
 * Propagate active status to the shapes
 */
void Entity::allowCollisions(bool value)
{
	this->allowCollisions = value;

	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape::enable(node->data, value);
		}
	}
}

/**
 * Retrieve allowing collision status
 */
bool Entity::doesAllowCollisions()
{
	return this->allowCollisions;
}


/**
 * Returns whether I have collision shapes or not
 */
bool Entity::hasShapes()
{
	return NULL != this->shapes && 0 < VirtualList::getSize(this->shapes);
}

void Entity::showShapes()
{
	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape::show(node->data);
		}
	}
}

void Entity::hideShapes()
{
	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape::hide(node->data);
		}
	}
}


/**
 * Set direction
 *
 * @param direction		Direction
 */
void Entity::setDirection(Direction direction)
{
	Direction currentDirection = Entity::getDirection(this);

	// if directions XOR is 0, they are equal
	if(
		!(
			(currentDirection.x ^ direction.x) |
			(currentDirection.y ^ direction.y) |
			(currentDirection.z ^ direction.z)
		)
	)
	{
		return;
	}

	Rotation rotation =
	{
		__UP == direction.y ? __HALF_ROTATION_DEGREES : __DOWN == direction.y ? 0 : this->transformation.localRotation.x,
		__LEFT == direction.x ? __HALF_ROTATION_DEGREES : __RIGHT == direction.x ? 0 : this->transformation.localRotation.y,
		//__NEAR == direction.z ? __HALF_ROTATION_DEGREES : __FAR == direction.z ? 0 : this->transformation.localRotation.z,
		this->transformation.localRotation.z,
	};

	Entity::setLocalRotation(this, &rotation);
}

/**
 * Get direction
 *
 * @return		Direction
 */
Direction Entity::getDirection()
{
	Direction direction =
	{
		__RIGHT, __DOWN, __FAR
	};

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.globalRotation.y))
	{
		direction.x = __LEFT;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.globalRotation.x))
	{
		direction.y = __UP;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.globalRotation.z))
	{
		direction.z = __NEAR;
	}

	return direction;
}

/**
 * Get Shape layers
 *
 * @return		Shape layers
 */
uint32 Entity::getShapesLayers()
{
	uint32 shapesLayers = 0;

	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			shapesLayers |= Shape::getLayers(shape);
		}
	}

	return shapesLayers;
}

/**
 * Set Shape layers
 *
 * @param uint32	Shape layers
 */
void Entity::setShapesLayers(uint32 layers)
{
	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::setLayers(shape, layers);
		}
	}
}

/**
 * Get Shape layers to ignore
 *
 * @return		Shape layers to ignore
 */
uint32 Entity::getShapesLayersToIgnore()
{
	uint32 shapesLayersToIgnore = 0;

	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			shapesLayersToIgnore |= Shape::getLayersToIgnore(shape);
		}
	}

	return shapesLayersToIgnore;
}

/**
 * Set Shape layers to ignore
 *
 * @param uint32	Shape layers to ignore
 */
void Entity::setShapesLayersToIgnore(uint32 layersToIgnore)
{
	if(NULL != this->shapes)
	{
		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			Shape::setLayersToIgnore(shape, layersToIgnore);
		}
	}
}

/**
 * Set transparency
 *
 * @param uint8	Transparency value
 */
void Entity::setTransparent(uint8 transparent)
{
	Base::setTransparent(this, transparent);

	if(!isDeleted(this->sprites))
	{
		for(VirtualNode node = this->sprites->head; NULL != node ; node = node->next)
		{
			Sprite::setTransparent(node->data, transparent);
		}
	}

	if(!isDeleted(this->wireframes))
	{
		for(VirtualNode node = this->wireframes->head; NULL != node ; node = node->next)
		{
			Wireframe::setTransparent(node->data, transparent);
		}
	}
}

/**
 * Get axis for Shape sync with direction
 *
 * @return		Axis
 */
uint16 Entity::getAxisForShapeSyncWithDirection()
{
	return __ALL_AXIS;
}

/**
 * Whether to respawn this Entity after it has been streamed out
 *
 * @return		Boolean whether to respawn this Entity
 */
bool Entity::respawn()
{
	return true;
}
