/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Clock.h>
#include <AnimatedSprite.h>
#include <MessageDispatcher.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Cuboid.h>

#include <Actor.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


__CLASS_DEFINITION(Actor);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// retrieve shape
Shape InGameEntity_getShape(InGameEntity this);

// resolve collision against other entities
static void Actor_resolveCollision(Actor this, VirtualList collidingEntities);

// update colliding entities
static void Actor_updateCollisionStatus(Actor this, int movementAxis);

// retrieve friction of colliding objects
static void Actor_updateSourroundingFriction(Actor this);

enum AxisOfCollision{
	
	kXAxis = 0,
	kYAxis,
	kZAxis
};

		
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(Actor, __PARAMETERS(ActorDefinition* actorDefinition, int ID))
__CLASS_NEW_END(Actor, __ARGUMENTS(actorDefinition, ID));

// Actor.c 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's conctructor
void Actor_constructor(Actor this, ActorDefinition* actorDefinition, int ID){

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, __ARGUMENTS(&actorDefinition->inGameEntityDefinition, ID));
	
	// save ROM definition
	this->actorDefinition = actorDefinition;
	
	// construct the game state machine
	this->stateMachine = __NEW(StateMachine, __ARGUMENTS(this));
	
	//set the direction
	this->direction.x = __RIGHT;
	this->previousDirection.x = __LEFT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;
	
	//state ALIVE for initial update
	this->inGameState = __LOADED;
	
	this->lastCollidingEntity[kXAxis] = NULL;
	this->lastCollidingEntity[kYAxis] = NULL;
	this->lastCollidingEntity[kZAxis] = NULL;
	
	this->sensibleToFriction.x = true;
	this->sensibleToFriction.y = true;
	this->sensibleToFriction.z = true;
	
	this->body = NULL;
	
	this->isAffectedBygravity = true;
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Actor_destructor(Actor this){
	
	// inform the screen I'm being removed
	Screen_focusEntityDeleted(Screen_getInstance(), (InGameEntity)this);
	
	if (this->body) {
		
		__DELETE(this->body);
	}
	
	// destroy state machine
	__DELETE(this->stateMachine);

	// destroy the super object
	__DESTROY_BASE(InGameEntity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set class's local position
void Actor_setLocalPosition(Actor this, VBVec3D position){
	
	Container_setLocalPosition((Container)this, position);

	if(this->body) {
		
		VBVec3D globalPosition = Container_getGlobalPosition((Container)this);
		VBVec3D localPosition = Container_getLocalPosition((Container)this);
		
		globalPosition.x += localPosition.x;
		globalPosition.y += localPosition.y;
		globalPosition.z += localPosition.z;

		Body_setPosition(this->body, &globalPosition, (Object)this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_transform(Actor this, Transformation* environmentTransform){

	// set sprite direction
	if(this->direction.x != this->previousDirection.x){
		
		// change sprite's direction
		Entity_setSpritesDirection((Entity)this, __XAXIS, this->direction.x);
		
		// save current direction
		this->previousDirection = this->direction; 
	}
	
	if(this->body && Body_isAwake(this->body)) {

		// an Actor with a physical body is agnostic to parenting
		Transformation environmentAgnosticTransform = {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// scale
				{environmentTransform->scale.x, environmentTransform->scale.y},
				// rotation
				{0, 0, 0}			
		};

		// save previous position
		this->previousGlobalPosition = this->transform.globalPosition;

		Container_setLocalPosition((Container) this, Body_getPosition(this->body));

		// call base
		InGameEntity_transform((InGameEntity)this, &environmentAgnosticTransform);
	}
	else {
		
		// call base
		InGameEntity_transform((InGameEntity)this, environmentTransform);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute character's logic
void Actor_update(Actor this){

	// call base
	Container_update((Container)this);

	// update state machine
	StateMachine_update(this->stateMachine);
	
	// if direction changed
	if(this->direction.x != this->previousDirection.x){
		
		// calculate gap again
		InGameEntity_setGap((InGameEntity)this);
	}	
	
	if(this->sprites){

		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for(; node ; node = VirtualNode_getNext(node)){

			Sprite sprite = (Sprite)VirtualNode_getData(node);

			// first animate the frame
			AnimatedSprite_update((AnimatedSprite)sprite);
		}
	}
	
	if(this->body) {
		
		Actor_updateCollisionStatus(this, Body_isMoving(this->body));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update colliding entities
static void Actor_updateCollisionStatus(Actor this, int movementAxis){

	ASSERT(this->body, "Actor::updateCollisionStatus: NULL body");

	if(__XAXIS & movementAxis) {

		this->lastCollidingEntity[kXAxis] = NULL;
	}
	if(__YAXIS & movementAxis) {

		this->lastCollidingEntity[kYAxis] = NULL;
	}
	if(__ZAXIS & movementAxis) {

		this->lastCollidingEntity[kZAxis] = NULL;
	}
	
	Actor_updateSourroundingFriction(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve friction of colliding objects
static void Actor_updateSourroundingFriction(Actor this){
	
	ASSERT(this->body, "Actor::updateSourroundingFriction: NULL body");
	
	Force friction = {0, 0, 0};

	if(this->sensibleToFriction.x) {
	
		friction.x = this->lastCollidingEntity[kYAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kYAxis]): 0;
		friction.x += this->lastCollidingEntity[kZAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kZAxis]): 0;
	}

	if(this->sensibleToFriction.y) {
		
		friction.y = this->lastCollidingEntity[kXAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kXAxis]): 0;
		friction.y += this->lastCollidingEntity[kZAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kZAxis]): 0;
	}

	if(this->sensibleToFriction.z) {
		
		friction.z = this->lastCollidingEntity[kXAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kXAxis]): 0;
		friction.z += this->lastCollidingEntity[kYAxis]? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kYAxis]): 0;
	}
		
	Body_setFriction(this->body, friction);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve previous position
VBVec3D Actor_getPreviousPosition(Actor this){
	
	return this->previousGlobalPosition;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor_setInGameState(Actor this, int inGameState){
	
	this->inGameState = inGameState;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set character's in game type
void Actor_setInGameType(Actor this, int inGameType){
	
	this->inGameType = inGameType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// change direction
void Actor_moveOpositeDirecion(Actor this, int axis){
	
	switch(axis){
	
		case __XAXIS:
			
			this->direction.x *= -1;			
			break;
			
		case __YAXIS:
			
			this->direction.y *= -1;
			break;
			
		case __ZAXIS:
			
			this->direction.z *= -1;
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether changed direction in the last cycle or not
int Actor_changedDirection(Actor this, int axis){
	
	switch(axis){
	
		case __XAXIS:
			
			return this->direction.x != this->previousDirection.x;			
			break;
			
		case __YAXIS:
			
			return this->direction.x != this->previousDirection.x;			
			break;
			
		case __ZAXIS:
			
			return this->direction.x != this->previousDirection.x;			
			break;
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate a write in graphic memory again
void Actor_resetMemoryState(Actor this, int worldLayer){		

	//Frame_resetMemoryState(this->sprite, worldLayer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// true if inside the screen range 
int Actor_isInsideGame(Actor this){
	
	//Texture map = Sprite_getTexture(this->sprite); 
	
	return 0;//!outsideScreenRange(this->transform.localPosition, Texture_getCols(map), Texture_getRows(map), __CHARACTERUNLOADPAD);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if gravity must apply to this actor
int Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration) {

	int axisFreeForMovement = __VIRTUAL_CALL(int, Actor, getAxisFreeForMovement, this);

	int axisOfCollision = 0;
	
	if(axisFreeForMovement) {

		ASSERT(this->body, "Actor::resolveCollision: NULL body");
	
		int i = 0; 
		// TODO: must still solve when there will be a collision with an object not yet in the list
		for(; i <= kZAxis; i++) {

			if (this->lastCollidingEntity[i]) {

				VBVec3D displacement = {
					kXAxis == i? acceleration->x: 0, 
					kYAxis == i? acceleration->y: 0, 
					kZAxis == i? acceleration->z: 0 
				}; 
				
				axisOfCollision |= __VIRTUAL_CALL(int, Shape, testIfCollision, this->shape, __ARGUMENTS(this->lastCollidingEntity[i], displacement));
			}	
		}
	}
	
	return axisFreeForMovement & ~axisOfCollision;
}

// retrieve axis free for movement
int Actor_getAxisFreeForMovement(Actor this){

	int movingState = Body_isMoving(this->body);
	
	return ((__XAXIS & ~(__XAXIS & movingState) )| (__YAXIS & ~(__YAXIS & movingState)) | (__ZAXIS & ~(__ZAXIS & movingState)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// resolve collision against other entities
static void Actor_resolveCollision(Actor this, VirtualList collidingEntities){
	
	ASSERT(this->body, "Actor::resolveCollision: NULL body");

	int axisOfCollision = 0;
	int alignThreshold = 1;

	// get last physical displacement
	VBVec3D displacement = Body_getLastDisplacement(this->body); 

	VirtualNode node = VirtualList_begin(collidingEntities);

	InGameEntity collidingEntity = NULL;
	
	// TODO: solve when more than one entity has been touched
	for(; node; node = VirtualNode_getNext(node)) {

		collidingEntity = VirtualNode_getData(node);
		axisOfCollision = __VIRTUAL_CALL(int, Shape, getAxisOfCollision, this->shape, __ARGUMENTS(collidingEntity, displacement));

		if(axisOfCollision) {
			
			break;
		}
	}

	if(axisOfCollision) {

		if(__XAXIS & axisOfCollision) {

			Actor_alignTo(this, collidingEntity, __XAXIS, alignThreshold);
			this->lastCollidingEntity[kXAxis] = collidingEntity;
		}
		
		if(__YAXIS & axisOfCollision) {

			Actor_alignTo(this, collidingEntity, __YAXIS, alignThreshold);
			this->lastCollidingEntity[kYAxis] = collidingEntity;
		}
		
		if(__ZAXIS & axisOfCollision) {

			Actor_alignTo(this, collidingEntity, __ZAXIS, alignThreshold);
			this->lastCollidingEntity[kZAxis] = collidingEntity;
		}

		// bounce over axis
		Body_bounce(this->body, axisOfCollision, __VIRTUAL_CALL(fix19_13, InGameEntity, getElasticity, collidingEntity));

		if(!(axisOfCollision & Body_isMoving(this->body))){

			MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this, kBodyStoped, &axisOfCollision);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Actor_handleMessage(Actor this, Telegram telegram){

	if (!StateMachine_handleMessage(this->stateMachine, telegram)) {
		
		// retrieve message
		int message = Telegram_getMessage(telegram);

		if (this->body) {
			
			Object sender = Telegram_getSender(telegram);
			Actor atherActor = __GET_CAST(Actor, sender);
			
			if (__GET_CAST(Cuboid, sender) || __GET_CAST(Body, sender)){
				
				switch(message){

					case kCollision:
						
						Actor_resolveCollision(this, (VirtualList)Telegram_getExtraInfo(telegram));
						Actor_updateCollisionStatus(this, *(int*)Telegram_getExtraInfo(telegram));
						return true;
						break;
												
					case kBodyStartedMoving:
						
						Actor_updateCollisionStatus(this, *(int*)Telegram_getExtraInfo(telegram));
						return true;
						break;
				}
			}
			else if (atherActor) {

				__VIRTUAL_CALL(void, Actor, takeHitFrom, atherActor);

				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state machine
StateMachine Actor_getStateMachine(Actor this){
	
	return this->stateMachine;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// does it moves?
int Actor_moves(Actor this){
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
int Actor_isMoving(Actor this){

	return this->body? Body_isMoving(this->body): 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve character's scale
Scale Actor_getScale(Actor this){

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	// get sprite's scale
	Scale scale = Sprite_getScale(sprite);
	
	// change direction
	scale.x = fabsf(scale.x) * this->direction.x;

	return scale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve global position
VBVec3D Actor_getPosition(Actor this){
	
	if(this->body) {
		
		return Body_getPosition(this->body);
	}
	
	return Entity_getPosition((Entity)this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// play an animation
void Actor_playAnimation(Actor this, char* animationName){
	
	if(this->sprites){

		VirtualNode node = VirtualList_begin(this->sprites);

		// play animation on each sprite
		for(; node ; node = VirtualNode_getNext(node)){
			
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			AnimatedSprite_play((AnimatedSprite)sprite, this->actorDefinition->animationDescription, animationName);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is play an animation
int Actor_isPlayingAnimation(Actor this, char* functionName){
	
	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return AnimatedSprite_isPlayingFunction((AnimatedSprite)sprite, this->actorDefinition->animationDescription, functionName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state when unloading the entity 
int Actor_getInGameState(Actor this){

	return this->inGameState;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's position
int Actor_updateSpritePosition(Actor this){

	return (this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z || Actor_isMoving(this) || *((int*)_screenMovementState));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this){

	if (Entity_updateSpriteScale((Entity)this)) {

		return true;
	}
	
	if (this->body && Body_isAwake(this->body) &&  Body_getVelocity(this->body).z) {
		
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop movement completelty
void Actor_stopMovement(Actor this){
	
	if(this->body) {
		
		Body_stopMovement(this->body, __XAXIS);
		Body_stopMovement(this->body, __YAXIS);
		Body_stopMovement(this->body, __ZAXIS);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop movement over axis
void Actor_stopMovementOnAxis(Actor this, int axis){

	if(this->body) {
		
		Body_stopMovement(this->body, axis);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// align character to other entity on collision
void Actor_alignTo(Actor this, InGameEntity entity, int axis, int pad){

	// retrieve the colliding entity's position and gap
	VBVec3D otherPosition = Entity_getLocalPosition((Entity) entity);	
	Gap otherGap = InGameEntity_getGap(entity);
	
	// pointers to the dimensions to affect
	fix19_13 *myPositionAxis = NULL;
	fix19_13 *otherPositionAxis = NULL;
	
	// used to the width, height or deep
	int myHalfSize = 0;
	int otherHalfSize = 0;
	
	// gap to use based on the axis
	int otherLowGap = 0;
	int otherHighGap = 0;
	int myLowGap = 0;
	int myHighGap = 0;	
	
	int screenSize = 0;

	// calculate gap again (scale, etc)
	InGameEntity_setGap((InGameEntity)this);

	// select the axis to affect
	switch(axis){
		
		case __XAXIS:
			
			myPositionAxis = &this->transform.localPosition.x;
			otherPositionAxis = &otherPosition.x;
			
			myHalfSize = (Entity_getWidth((Entity)this) >> 1);
			otherHalfSize = (Entity_getWidth((Entity)entity) >> 1);
			
			otherLowGap = otherGap.left;
			otherHighGap = otherGap.right;
			myLowGap = this->gap.left;
			myHighGap = this->gap.right;
			
			screenSize = __SCREENWIDTH;
			
			break;
			
		case __YAXIS:

			myPositionAxis = &this->transform.localPosition.y;
			otherPositionAxis = &otherPosition.y;
			
			myHalfSize = (Entity_getHeight((Entity)this) >> 1);
			otherHalfSize = (Entity_getHeight((Entity)entity) >> 1);
			
			otherLowGap = otherGap.up;
			otherHighGap = otherGap.down;
			myLowGap = this->gap.up;
			myHighGap = this->gap.down;
			
			screenSize = __SCREENHEIGHT * 100;
			break;
			
		case __ZAXIS:
			
			myPositionAxis = &this->transform.localPosition.z;
			otherPositionAxis = &otherPosition.z;
			
			myHalfSize = (InGameEntity_getDeep((InGameEntity)this) >> 1);
			otherHalfSize = (InGameEntity_getDeep(entity) >> 1);
			
			screenSize = __MAXVIEWDISTANCE;
			break;			
	}
	
	// decide to which side of the entity align myself
	if(*myPositionAxis > *otherPositionAxis){

//		pad -= (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1)? 1: 0);
		// align right / below / behind
		*myPositionAxis = *otherPositionAxis +  
							ITOFIX19_13(otherHalfSize - otherHighGap
							+ myHalfSize - myLowGap
							+ pad);
	}
	else{
		// align left / over / in front
		*myPositionAxis = *otherPositionAxis -  
							ITOFIX19_13(otherHalfSize - otherLowGap
							+ myHalfSize - myHighGap
							+ pad);
	}

	if (this->body) {

		// force position
		Body_setPosition(this->body, &this->transform.localPosition, (Object)this);

		// sync to body
		Container_setLocalPosition((Container) this, Body_getPosition(this->body));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
void Actor_syncBodyPosition(this){
	
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve body
const Body Actor_getBody(Actor this){
	
	return this->body;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// take hit
void Actor_takeHitFrom(Actor this, Actor other){
	
	const Body otherBody = Actor_getBody(other);
	
	if (otherBody){
		
		Body_takeHitFrom(this->body, otherBody);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get elasticiy
fix19_13 Actor_getElasticity(Actor this){
	
	return this->body? Body_getElasticity(this->body): InGameEntity_getElasticity((InGameEntity)this);
}

