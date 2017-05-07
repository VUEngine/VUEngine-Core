Post-processing
===============

The engine supports post-processing effects that allow the direct manipulation of the pixels drawn by the VIP before they current game frame is show in the Virtual Boy's displays.


Usage
-----

To apply a post-processing effect register it using the following call:

	Game_pushBackProcessingEffect(Game_getInstance(), postProcessingEffectFunctionPointer, spatialObject);

To remove a previously registered post-processing effect use the following code:
	
	Game_removePostProcessingEffect(Game_getInstance(), postProcessingEffectFunctionPointer, spatialObject);

A PostProcessingEffect is a pointer to a function with the following signature:

	void PostProcessingEffects_dummyEffect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject)

All registered post-processing effects are removed when the Game changes state.

It is possible to apply several post processes at once and thus combine their effects.


Applications
------------

Since the image frame has been fully processed and written to the frame buffer at the moment, post-processing kicks in, you are free to manipulate the image in any way you like. You can thus use post-processing to achieve visual effects which would not be possible by other means on the Virtual Boy.

For example, shift down each row of the screen by a different amount of bytes to achieve a wave like wobble effect as seen in the screen below. The `VUEngine Platformer Demo` comes with this and a few more example post-processing effects, which can be found in the file `source/components/PostProcessingEffects.c`.

@image html post-processing-effect-wobble.png "Post-processing effect example: \"wobble\"" 


Advices
-------

Effects like the aforementioned wobble effect might look impressive and even perform well on emulators, but they're not feasible for use on real hardware, since accessing the framebuffer is very slow. The problem is that, for each framebuffer access, the hardware has to wait a certain amount of CPU cycles. 1 to write and 3(!) for read access. Since the wobble effect reads and then writes almost the entire screen, it's very slow on a real Virtual Boy.

We can therefore only advice you to use post-processing effects carefully and try to manipulate only small areas of the screen! If possible, only write to, but not read from, the framebuffer!
