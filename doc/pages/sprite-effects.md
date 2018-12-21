Sprite effects
==============

Different types of effects can be applied to Bgmap Sprites by setting their mode to either __WORLD_HBIAS or
__WORLD_AFFINE instead of __WORLD_BGMAP.


HBias
-----

...


Affine
------

...


Effect functions
----------------
 
... built-in or custom functions ...


Shared Param Tables
-------------------

If you want the same transformation applied to more than one Sprite at once, it would be wasteful to simply 
define the same effect function for each, because a separate Param Table would be set up for each Sprite 
and the same effect function executed separately for each Sprite.

Instead it makes sense for all these Sprites to share a single Param Table, which can be achieved by 
inheriting from BgmapSprite. Since you can manage the Sprites of any Entity, you can define all of them as 
HBias or Affine, allocate one of them manually by calling the ParamTableManager's allocate 
method, and then assign the returned param value to all the Sprites that you want to be affected. 
All this can be done in the Entity's ready method. 
Make sure that the paramTableRow attribute is -1 for all Sprites but one, otherwise all them will call the 
effect function.
