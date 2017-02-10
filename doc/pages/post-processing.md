Post-processing
===============

The engine supports post-processing effects that allow the direct manipulation of the pixels drawn by the VIP before they current game frame is show in the Virtual Boy's displays.

To apply a post processing effect register it using the following call:

	Game_addPostProcessingEffect(Game_getInstance(), postProcessingEffectFunctionPointer, spatialObject);

To remove a previously registered post processing effect use the following code:
	
	Game_removePostProcessingEffect(Game_getInstance(), postProcessingEffect, spatialObject);

A PostProcessingEffect is a pointer to a function with the following signature:

	void PostProcessingEffects_dummyEffect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject __attribute__ ((unused)))

All registered post processing effects are removed when the `Game` changes state.
