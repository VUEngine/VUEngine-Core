/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Component.h>
#include <ListenerObject.h>
#include <Collider.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Body;
class Component;
class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Entity
///
/// Inherits from ListenerObject
///
/// Defines objects that occupy a place in 3D space.
abstract class Entity : ListenerObject
{
	/// @protectedsection
	
	/// Linked list of attached components
	VirtualList* components;
	
	/// 3D transformation
	Transformation transformation;

	/// Body for physics simulations
	Body body;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Clear the linked lists of components.
	/// @param componentType: Type of components whose lists must be cleard
	void clearComponentLists(uint32 componentType);

	/// Attach a new component to the actor and configure it with the provided spec.
	/// @param componentSpec: Specification to be used to configure the new component
	/// @return Added component
	Component addComponent(const ComponentSpec* componentSpec);

	/// Remove a component from the actor.
	/// @param component: Component to remove
	void removeComponent(Component component);

	/// Attach a new components to the actor and configure it with the provided specs.
	/// @param componentSpecs: Specifications to be used to configure the new components
	/// @param componentType: Type of components to add
	void addComponents(ComponentSpec** componentSpecs, uint32 componentType);

	/// Remove the components of a give type from the actor.
	/// @param componentType: Type of components to remove
	void removeComponents(uint32 componentType);

	/// Retrieve a component of the given type at the desired position.
	/// @param componentType: Type of components to add
	/// @param componentIndex: Component's index according to their order of creation
	/// @return Component at the provided index position
	Component getComponentAtIndex(uint32 componentType, int16 componentIndex);

	/// Retrieve a list with the components of the provided type.
	/// @param componentType: Type of components to add
	/// @return Linked list of components of the type provided
	VirtualList getComponents(uint32 componentType);

	/// Retrieve the linked list of components that are instances of the provided class.
	/// @param classPointer: Pointer to the class to use as search criteria. Usage: typeofclass(ClassName)
	/// @param components: Linked list to be filled with the behaviors that meed the search criteria
	/// (it is externally allocated and must be externally deleted)
	/// @param componentType: Type of components to retrieve
	/// @return True if one or more behaviors met the search criteria; false otherwise
	bool getComponentsOfClass(ClassPointer classPointer, VirtualList components, uint32 componentType);

	/// Retrieve the number of components belonging to the actor.
	/// @param componentType: Type of components to count
	/// @return Number of components belonging to the actor
	uint16 getComponentsCount(uint32 componentType);

	/// Reset components.
	void resetComponents();

	/// Retrieve the object's transformation.
	/// @return Pointer to the object's 3D transformation
	const Transformation* getTransformation();

	/// Retrieve the object's position.
	/// @return Pointer to the object's 3D vector defining its position
	const Vector3D* getPosition();

	/// Retrieve the object's rotation.
	/// @return Pointer to the object's 3D rotation
	const Rotation* getRotation();

	/// Retrieve the object's scale.
	/// @return Pointer to the object's 3D
	const Scale* getScale();

	/// Retrieve the statefulActor's physical body.
	/// @return StatefulActor's physical body
	Body getBody();

	/// Check if the statefulActor is moving.
	/// @return True if the statefulActor's body is moving; false otherwise
	bool isMoving();

	/// Stop all statefulActor's movement.
	void stopAllMovement();

	/// Stop the statefulActor's movement in the specified axis.
	/// @param axis: Axis on which to stop the movement of the statefulActor's body
	void stopMovement(uint16 axis);

	/// Set the statefulActor's velocity vector.
	/// @param velocity: Velocity vector to assign to the statefulActor's body
	/// @param checkIfCanMove: If true, the statefulActor checks that none of its colliders will
	/// enter a collision if it were to move in the direction of the provided velocity
	/// @return True if the statefulActor started to move in the direction specified by the
	/// provided velocity vector
	bool setVelocity(const Vector3D* velocity, bool checkIfCanMove);

	/// Retrieve the object's velocity vector.
	/// @return Pointer to the direction towards which the object is moving
	const Vector3D* getVelocity();

	/// Retrieve the object's current speed (velocity vector's magnitude).
	/// @return Object's current speed (velocity vector's magnitude)
	fixed_t getSpeed();

	/// Retrieve the statefulActor's maximum speed.
	/// @return Maximum speed at which the statefulActor's body is allowed to move
	fixed_t getMaximumSpeed();

	/// Retrieve the object's bounciness factor.
	/// @return Object's bounciness factor
	fixed_t getBounciness();

	/// Retrieve the object's friction coefficient.
	/// @return Object's friction coefficient
	fixed_t getFrictionCoefficient();

	/// Enable collision detection on the actor's colliders.
	void enableCollisions();

	/// Disable collision detection on the actor's colliders.
	void disableCollisions();

	/// Enable or disable collision detection against other actors' colliders.
	/// @param activate: If true, this actor's colliders check collision against other actors'
	void checkCollisions(bool activate);

	/// Enable or disable the register of detected collisions.
	/// @param activate: If false, this actor's colliders won't keep track of collisions, hence they
	/// won't notify of it of persisting (::collisionPersists) collisions or when end (::collisionEnds)
	void registerCollisions(bool activate);

	/// Set the layers in which this actor's colliders must live.
	/// @param layers: Flags that determine the layers for the actor's colliders
	void setCollidersLayers(uint32 layers);

	/// Retrieve the layers in which this actor's colliders live.
	/// @return Flags that determine the layers where the actor's colliders live
	uint32 getCollidersLayers();

	/// Set the layers that the actor's colliders must ignore when detecting collision.
	/// @param layersToIgnore: Flags that determine the layers with colliders to ignore when detecting
	/// collisions
	void setCollidersLayersToIgnore(uint32 layersToIgnore);

	/// Retrieve the layers that the actor's colliders ignore when detecting collision.
	/// @return The layers that the actor's colliders ignore when detecting collision
	uint32 getCollidersLayersToIgnore();

	/// Check if the actor has attached colliders.
	/// @return True if the actor hast at least on collider arrached; false otherwise
	bool hasColliders();

	/// Make the actor's colliders visible.
	void showColliders();

	/// Make the actor's colliders invisible.
	void hideColliders();

	/// Create the components that must attach to this container. 	
	/// @param componentSpecs: Specifications to be used to configure the new components
	virtual void createComponents(ComponentSpec** componentSpecs);

	/// Destroy the components that attach to this container. 	
	virtual void destroyComponents();

	/// A new component has been added to this statefulActor. 
	/// @param component: Added component
	virtual void addedComponent(Component component);

	/// A component has been removed from this statefulActor. 
	/// @param component: Removed component
	virtual void removedComponent(Component component);

	/// Make this instance visible.
	virtual void show();

	/// Make this instance invisible.
	virtual void hide();

	/// Set this instance's transparency effects.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __EVEN or __ODD)
	virtual void setTransparency(uint8 transparency);

	/// Configure the actor's size.
	virtual void calculateSize();

	/// Retrieve the object's radius.
	/// @return Radius
	virtual fixed_t getRadius();

	/// Set the object's position.
	/// @param position: 3D vector defining the object's new position
	virtual void setPosition(const Vector3D* position);

	/// Set the object's rotation.
	/// @param rotation: Rotation
	virtual void setRotation(const Rotation* rotation);

	/// Set the object's scale.
	/// @param scale: Scale
	virtual void setScale(const Scale* scale);

	/// Set the direction towards which the object must move.
	/// @param direction: Pointer to a direction vector
	virtual void setDirection(const Vector3D* direction);

	/// Retrieve the direction towards which the object is moving.
	/// @return Pointer to the direction towards which the object is moving
	virtual const Vector3D* getDirection();

	/// Apply a force to the statefulActor's body.
	/// @param force: Force to be applied
	/// @param checkIfCanMove: If true, the statefulActor checks that none of its colliders will
	/// @return True if the force was succesfully applied to the statefulActor's body
	virtual bool applyForce(const Vector3D* force, bool checkIfCanMove);

	/// Check if the statefulActor will enter a collision if it were to move in the provided direction
	/// @param direction: Direction vector to check
	virtual bool canMoveTowards(Vector3D direction);

	/// Check if when the statefulActor bounces it has to take into account the colliding object's bounciness.
	/// @return True if the statefulActor has to take into account the colliding object's bounciness when bouncing
	virtual bool isSensibleToCollidingObjectBouncinessOnCollision(Entity collidingEntity);

	/// Check if when the statefulActor bounces it has to take into account the colliding object's friction
	/// coefficient.
	/// @return True if the statefulActor has to take into account the colliding object's friction coefficient when
	/// bouncing
	virtual bool isSensibleToCollidingObjectFrictionOnCollision(Entity collidingEntity);

	/// Check if the object is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the object; false otherwise
	virtual bool isSubjectToGravity(Vector3D gravity);

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	/// @return True if the collider must keep track of the collision to detect if it persists and when it ends; false otherwise
	virtual bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Process a going on collision detected by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve 
	virtual void collisionPersists(const CollisionInformation* collisionInformation);

	/// Process when a previously detected collision by one of the component colliders stops.
	/// @param collisionInformation: Information struct about the collision to resolve
	virtual void collisionEnds(const CollisionInformation* collisionInformation);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	virtual uint32 getInGameType();
}

#endif
