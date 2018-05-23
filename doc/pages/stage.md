Stages
======

Each `GameState` has a `Stage` that works as the `Container` for all the entities that compose a game level. The `Stage`'s child entities are defined in a StageDefinition struct.

Adding entity containers to the stage
-------------------------------------

There are two ways to add entities to the `Stage` during gameplay. The first is to add the entity directly into the `Stage`and must be done using the following interface:
 
	Entity newEntity = Stage::addChildEntity(stagePointer, &positionedEntity, true);

The second way to add an entity after the stage's initialization is to add the new entity as a child of another already added to the stage:

	Entity newChildEntity = Entity::addChildEntity(__SAFE_CAST(Entity, this), entityDefinitionPointer, -1, "name", &position, NULL);

In any case, it is never legal to call the Entity's allocator (or that of any class that inherits from `Container`) with __NEW(Entity, ...) since the process of memory allocation, instantiation and initialization is handled by the `EntityFactory`, and calling the allocator directly will never achieve the intended results. 

Deleting entities
-----------------

Similarly, any `Container` added to the `Stage` must be removed by calling its deleteMyself method. 

Never use of the __DELETE macro to remove any `Container` that has been added as child of another, because the deletion could happen inside the iteration through a list that has that `Container`, causing undefined behavior.


Streaming
---------

The engine support level streaming at the `Stage` level. The stages are loaded and populated based on the StageDefinition passed to their constructor.

The StageDefinition has a pointer to an array of PositionedEntity structs that define the `Stage`'s children.

A PositionEntity is composed of the following attributes:

	typedef const struct PositionedEntity
	{
	    EntityDefinition* entityDefinition; 			// pointer to the entity definition in ROM
	    Vector3D position; 								// position in the world
	    s16 id;											// entity's id
	    char* name;										// name
	    struct PositionedEntity* childrenDefinitions;	// the children
	    void* extraInfo;								// extra info
		bool loadRegardlessOfPosition;					// force load
	} PositionedEntity;

The position attribute is used by the streaming to determine if a given entity must be streamed in or not. If the loadRegardlessOfPosition is set to true, the entity is loaded at the start of the game level and is never streamed out.

The StageDefinition struct has an special attribute that controls the behavior of the streaming:

	struct Streaming
	{
		u16 minimumSpareMilliSecondsToAllowStreaming;
		u16 loadPadding;
		u16 unloadPadding;
		u16 streamingAmplitude;
		u16 particleRemovalDelayCycles;
	} streaming;

The minimumSpareMilliSecondsToAllowStreaming attribute is used to skip the streaming processes during long-to-process game frames to help reduce frame rate drops or frame pacing issues.

The loadPadding attribute is added to the game's virtual screen's size (the Virtual Boy's screen's size plus depth: 384x224xZ) to calculate the threshold for entities to be loaded according to the position given in the PositionEntity entry. 

The unloadPadding attribute is added to previously calculated threshold (with the value of the loadPadding) to calculate the threshold for entities to be unloaded according to their positions during gameplay. 

The streamingAmplitude attribute determines the number of PositionEntity entries to poll to stream in entities in each streaming cycle.

The particleRemovalDelayCycles attribute determines the number of streaming cycles to wait before deleting any discarded particle.

Another attribute of the StageDefinition that impacts the behavior of the streaming is the rendering attribute. It holds a couple of values that impact the visual representation of streamed in entities:
 
 	// rendering
 	struct Rendering
 	{
	    int cyclesToWaitForTextureWriting;		// number of cycles that the texture writing is idle
	    int texturesMaximumRowsToWrite;			// maximum number of texture's rows to write each time the
	    										// texture writing is active
		int maximumAffineRowsToComputePerCall;	// maximum number of rows to compute
												// on each call to the affine functions
 		.
 		.
 		.
	}


To avoid any graphical glitch during gameplay, caused by the writing to DRAM while the VIP is still performing its drawing operations, or by the amount of work done during the VIP's drawing interrupt, the engine defers texture writing though a number of cycles. The engine performs texture writing every other cycle according to the value of the cyclesToWaitForTextureWriting attribute.
When the game reaches a cycle were texture writing is allowed, it starts writing the first non written texture found according to the order of the sprites in the `SpriteManager`'s list. The texturesMaximumRowsToWrite attribute determines the number of rows to write on each cycle. Depending on this number and on the number of rows of a given `Texture`, its writing could expand across multiple cycles, causing temporal graphical corruption if not balanced properly.

The maximumAffineRowsToComputePerCall attribute limits how many rows can be calculated per `Sprite` per rendering cycle for affine effects.

Limitations
-----------

The PositionEntity entries must be arranged in the array that defines the `Stage`'s children in such a way that their position in it reflects their spatial position within the level (except for those that has the loadRegardlessOfPosition flag to true) and the level's general shape must be roughly linear. This is because the engine does not implement octrees or any other similar approach to level construction.
