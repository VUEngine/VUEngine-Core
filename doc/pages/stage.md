Stages
======

Each `GameState` has a `Stage` that works as the `Container` for all the `Entities` that compose a game level. In the general case, the stage's child entities are defined in a `StageDefinition` struct that is managed by the `GameState`.

Adding entities to the stage
----------------------------

There are two ways to add entities to the `Stage` during gameplay. The first is to add the entity directly into the `Stage`and must be done using the following interface:
 
	Entity newEntity = Stage_addChildEntity(stagePointer, &positionedEntity, true);

The second way to add an entity after the stage's initialization is to add the new entity as a child of another already added to the stage:

	Entity newChildEntity = Entity_addChildEntity(__SAFE_CAST(Entity, this), entityDefinitionPointer, -1, "name", &position, NULL);

In any case, it is never legal to call the Entity's allocator (or that of any class that inherits from `Container`) with __NEW(Entity, ...) since the process of memory allocation, instantiation and initialization is handled by the `EntityFactory`, and calling the allocator directly will never achieve the intended results. 

Deleting entities
-----------------

Since the whole stage is composed of containers, the safe way to delete them is to use the Container_deleteMyself method, otherwise the container could be being deleted from within a list's loop, causing undefined behavior.

Don't delete directly entities added to the parenting hierarchy by using the __DELETE macro.


Streaming
---------

[TODO]

- Constructing a Stage: Here we have to address the PositionedEntity struct in detail, specially the extra info attribute which must only be used in "final" classes. 

