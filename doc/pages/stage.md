Stages
======

Adding entities to the stage
----------------------------

To add entities to the stage, either use the Stage_addEntity method, or the Entity_addChildFromDefinition, depending on the desired parent for the new entity.


Deleting entities
-----------------

Since the whole stage is composed of containers, the safe way to delete them is to use the Container_deleteMyself method, otherwise the container could be being deleted from within a list's loop, causing undefined behavior.

Don't delete directly entities added to the parenting hierarchy by using the __DELETE macro.


Streaming
---------

[TODO]

- Constructing a Stage: Here we have to address the PositionedEntity struct in detail, specially the extra info attribute which must only be used in "final" classes. 
