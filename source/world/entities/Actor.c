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
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>

#include <Actor.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

enum CollidingObjectIndexes{
	eXAxis = 0,
	eYAxis,
	eZAxis,
	eLastCollidingObject,
};

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

		
//};
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
	
	switch(actorDefinition->inGameEntityDefinition.entityDefinition.spriteDefinition.textureDefinition->charGroupDefinition.allocationType){
	
		case __ANIMATED:
		case __ANIMATED_SHARED:	

			// create the animated sprite
			this->sprite = (Sprite)__NEW(AnimatedSprite, 
							__ARGUMENTS((void*)this, 
							actorDefinition->animationDescription->numberOfFrames,
							&actorDefinition->inGameEntityDefinition.entityDefinition.spriteDefinition
							));	
			
			break;
			
		default:
			// create the sprite
			this->sprite = __NEW(Sprite, __ARGUMENTS(&actorDefinition->inGameEntityDefinition.entityDefinition.spriteDefinition));
			
			break;
	}

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
	this->inGameState = kLoaded;
	
	// reset action timers
	this->timeX = 0;
	this->timeY = 0;
	this->timeZ = 0;
	
	//pointing to right
	this->direction.alpha.cos = 0;
	this->direction.betha.cos = 0;
	this->direction.tetha.cos = 0;

	// clear movement type
	this->moveType = 0;
	
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	
	this->acceleration.x = 0;
	this->acceleration.y = 0;
	this->acceleration.z = 0;
	
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	
	Actor_calculateAngles(this, __XAXIS);
	Actor_calculateAngles(this, __YAXIS);
	Actor_calculateAngles(this, __ZAXIS);
	
	this->objectBelow = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Actor_destructor(Actor this){
	
	// inform the screen I'm being removed
	Screen_focusEntityDeleted(Screen_getInstance(), (InGameEntity)this);
	
	// destroy state machine
	__DELETE(this->stateMachine);

	// destroy the super object
	__DESTROY_BASE(InGameEntity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set character's angle
void Actor_setAngle(Actor this, Angle angle){
	//this->direction.alpha.degree=angle.degree;
	//this->direction.betha.degree=angle.degree;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate character's direction angle
void Actor_calculateAngles(Actor this, int axis){
	
	// calculate total velocity
	int velocityX = FIX19_13TOI(this->velocity.x);
	int velocityY = FIX19_13TOI(this->velocity.y);
	int velocityZ = FIX19_13TOI(this->velocity.z);

	// do a quick int multiplication
	int totalVelocity = squareRootFloat(velocityX * velocityX + velocityY * velocityY + velocityZ * velocityZ);
	
	// pointer to the axis's elements to affect
	fix19_13* axisVelocity = NULL;	
	fix7_9* angleCos = NULL;
	
	// make sure that the total velocity is not greater than the maximun fix19_13 value
	ASSERT(totalVelocity < (1 << 19), Actor: total velocity overflow);

	// transform velocity to fixed point 
	this->velocity.V = ITOFIX19_13(totalVelocity);
	
	// select the axis
	switch(axis){
	
		case __XAXIS:
			
			axisVelocity = &this->velocity.x;
			angleCos = &this->direction.alpha.cos;
			break;

		case __YAXIS:
			
			axisVelocity = &this->velocity.y;
			angleCos = &this->direction.betha.cos;
			break;
			
		case __ZAXIS:
			
			axisVelocity = &this->velocity.z;
			angleCos = &this->direction.betha.cos;
			break;
			
		default:
			ASSERT(false, Actor: no valid axis);
	}
	
	// if the calculated velocity is not 0
	if(this->velocity.V){		
		
		// calculate it
		*angleCos = FIX19_13TOFIX7_9(FIX19_13_DIV(*axisVelocity, this->velocity.V));
	}
	else{
		
		// otherwise just asing a cos of 1 
		*angleCos = ITOFIX7_9(1);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup velocity
void Actor_setVelocity(Actor this, int xVel, int yVel, int zVel){
	
	//set physical movement attributes
	this->velocity.x = abs(xVel);
	this->velocity.y = abs(yVel);
	this->velocity.z = abs(zVel);
	
	/*
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	*/
	
	Actor_calculateAngles(this, __XAXIS);
	Actor_calculateAngles(this, __YAXIS);
	Actor_calculateAngles(this, __ZAXIS);
	//this->velocity.resultantXY=sqrtf(xVel*xVel+yVel*yVel);
	//this->velocity.resultantXZ=sqrt(xVel*xVel+zVel*zVel);
	//this->speedMultiplier=1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set aceleration
void Actor_setAceleration(Actor this, int xAcel, int yAcel, int zAcel){
	
	//set physical movement attributes
	this->acceleration.x = xAcel;	
	this->acceleration.y = yAcel;
	this->acceleration.z = zAcel;
	
	if(!xAcel){
		this->acceleration.x = -1;		
	}
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_render(Actor this, Transformation environmentTransform){

	// set sprite direction
	if(this->direction.x != this->previousDirection.x){
		
		// change sprite's direction
		Sprite_setDirection(this->sprite, __XAXIS, this->direction.x);
		
		// save current direction
		this->previousDirection = this->direction; 
	}

	// call base
	InGameEntity_render((InGameEntity)this, environmentTransform);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor_setInGameState(Actor this, int inGameState){
	
	this->inGameState = inGameState;
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
	
	// first animate the frame
	AnimatedSprite_update((AnimatedSprite)this->sprite);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve character's velocity
Velocity Actor_getVelocity(Actor this){
	
	return this->velocity;
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
// update movement
void Actor_move(Actor this){
	
	// check if axis's velocity
	if(this->velocity.x){
		
		Actor_keepXMovement(this);	
	}
	
	if(this->velocity.y){
		
		Actor_keepYMovement(this);
	}
	
	if(this->velocity.z){
		
		Actor_keepZMovement(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// make a character to jump
void Actor_jump(Actor this, fix19_13 velocity, fix19_13 acceleration){
	
	//initialice the dynamic changing velocity
	//set the direction over the y axis
	this->direction.y = __UP;
	
	// set values
	this->velocity.y  = velocity;	
	this->acceleration.y = acceleration;
	
	//record initial jumping time	
	this->timeY = Clock_getTime(_inGameClock);
	
	// calculate angles
	Actor_calculateAngles(this, __YAXIS);
	
	// y velocity = total velocity 
	this->velocity.y = this->velocity.V;

	// it's retar movement
	this->moveType |= __RETARMOVEY;
	
	if(this->moveType & __ACCELMOVEY){
		
		this->acceleration.y = (this->acceleration.y - ITOFIX19_13(__GRAVITY));
	}
	else{
		
		this->acceleration.y = FIX19_13_MULT(this->acceleration.y + ITOFIX19_13(__GRAVITY), ITOFIX19_13(-1));
	}
	
	this->maxFlyTime = this->timeY + 1000 * FIX19_13TOF(
				FIX19_13_DIV(
					FIX19_13_MULT(FIX7_9TOFIX19_13(this->direction.betha.cos), this->velocity.V), 
					ITOFIX19_13(__GRAVITY)
				)
			);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// make a character fall
void Actor_fall(Actor this){
	return;
	//set the direction over the y axis
	this->direction.y = __DOWN;
	
	//record initial falling time
	this->timeY = Clock_getTime(_inGameClock);

	this->velocity.y = ITOFIX19_13(100);
	
	Actor_calculateAngles(this, __YAXIS);

	this->moveType |= __ACCELMOVEY;
	this->acceleration.y = ITOFIX19_13(__GRAVITY);
	this->timeY = Clock_getTime(_inGameClock);
	
	// if I fall, there is nothing below me
	this->objectBelow = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// start the movement over the x axis
void Actor_startMovement(Actor this, int axis, int moveType, fix19_13 velocity, fix19_13 acceleration){
	
	// select the axis
	switch(axis){
		
		case __XAXIS:
			
			// initialize the velocity	
			this->velocity.x = velocity;
			
			// initialize acceleration
			this->acceleration.x = acceleration;
			
			//record initial x time
			this->timeX = Clock_getTime(_inGameClock);

			break;

		case __YAXIS:
			
			// initialize the velocity	
			this->velocity.y = velocity;
			
			// initialize acceleration
			this->acceleration.y = acceleration;
			
			//record initial z time
			this->timeY = Clock_getTime(_inGameClock);

			break;			
			
		case __ZAXIS:
			
			// initialize the velocity	
			this->velocity.z = velocity;
			
			// initialize acceleration
			this->acceleration.z = acceleration;
			
			//record initial z time
			this->timeZ = Clock_getTime(_inGameClock);

			break;			
	}
	
	// calculate the angles
	Actor_calculateAngles(this, axis);	
	
	// add movement type to current one
	this->moveType |= moveType;
	
	// calculate acceleration
	Actor_calculateAcceleration(this, axis);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate acceleration over an axis
void Actor_calculateAcceleration(Actor this, int axis){
	
	// get friction factor from the game world
	fix19_13 friction = GameWorld_getFriction(GameWorld_getInstance());
	
	// pointer to the axis's elements to affect
	fix19_13* acceleration = NULL;

	// the movement to compare
	int moveType = 0;
	
	// if I'm over something
	if(this->objectBelow){
		
		// grab it's friction factor
		friction = FIX7_9TOFIX19_13(InGameEntity_getFrictionFactor(this->objectBelow));
	}
	else{
	
		friction = ITOFIX19_13(50);
	}
	
	// select the axis
	switch(axis){
	
		case __XAXIS:
			
			acceleration = &this->acceleration.x;
			moveType = __RETARMOVEX;
			break;

		case __YAXIS:
			
			acceleration = &this->acceleration.y;
			moveType = __RETARMOVEY;
			break;
			
		case __ZAXIS:
			
			acceleration = &this->acceleration.z;
			moveType = __RETARMOVEZ;
			break;
			
		default:
			ASSERT(false, Actor: no valid axis);
	}
	
	// if movement is accelerated
	if(this->moveType & moveType){
		
		*acceleration += ITOFIX19_13(abs(FIX19_13TOI(*acceleration - FIX19_13_MULT(*acceleration, friction))));
		*acceleration = FIX19_13_MULT(*acceleration, ITOFIX19_13(-1));
	}
	else{
		
		*acceleration -= ITOFIX19_13(abs(FIX19_13TOI(*acceleration - FIX19_13_MULT(*acceleration, friction))));
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// udpdate movement over axis
void Actor_keepYMovement(Actor this){

	// get the elapsed time
	fix19_13 timeElapsed = FIX19_13_DIV(ITOFIX19_13(Clock_getTime(_inGameClock) - this->timeY ) , ITOFIX19_13(1000));
	
	// the movement displacement
	fix19_13 displacement = 0;
	
	// get the angle's sin
	fix19_13 sinXP = FIX7_9TOFIX19_13(this->direction.betha.cos);
	
	// if not time has elapsed
	if(!timeElapsed){
		
		// do not continue processing
		return;
	}
	
	// check the movement type
	if(this->moveType & (__ACCELMOVEY | __RETARMOVEY )){
	
		// calculate the displacement in a varied movement
		displacement = FIX19_13_MULT(FIX19_13_MULT(this->velocity.y, timeElapsed), sinXP)
				+ FIX19_13_MULT(this->acceleration.y, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
		
		// otherwise, update the velocity
		this->velocity.y += FIX19_13_MULT(this->acceleration.y, timeElapsed);
		// check if the character max flying time has been reached
		//if((this->timeY > this->maxFlyTime) && this->direction.y == __UP){
		vbjPrintInt(abs(FIX19_13TOI(this->velocity.y)), 10, 10);
		if(abs(FIX19_13TOI(this->velocity.y)) < 0.01f) {

			// stop the movement			
			this->velocity.y = 0;
			
			// begin to fall
			Actor_fall(this);
			
			// stop processing
			return;
		}
		else{
			
			// otherwise, update the velocity
			//this->velocity.y += FIX19_13_MULT(this->acceleration.y, timeElapsed);			
		}
	}
	else{
		
		// movement is uniform
		displacement = FIX19_13_MULT(FIX19_13_MULT(this->velocity.V, timeElapsed), sinXP);
	}
	
	
	// update position
	this->transform.localPosition.y += (FIX19_13_MULT(displacement, ITOFIX19_13(this->direction.y)));
	
	// check if floor has been reached
	if (this->transform.localPosition.y >= ITOFIX19_13(__FLOOR)){
		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this, kFloorReached, NULL);
	}
	
	// record this update time
	this->timeY = Clock_getTime(_inGameClock);

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// udpdate movement over axis
void Actor_keepXMovement(Actor this){
	
	// get the elapsed time
	fix19_13 timeElapsed = FIX19_13_DIV(ITOFIX19_13(Clock_getTime(_inGameClock) - this->timeX ) , ITOFIX19_13(1000));
	
	// the movement displacement
	fix19_13 displacement = 0;
	
	// if no time has elapsed
	if(!timeElapsed){

		//stop processing 
		return;
	}

	// determine the movement type	
	if(this->moveType & (__ACCELMOVEX | __RETARMOVEX )){
	
		// calculate displacement based in velocity, time and acceleration
		displacement =
			FIX19_13_MULT(FIX19_13_MULT(this->velocity.x, timeElapsed), FIX7_9TOFIX19_13(this->direction.alpha.cos)) 
			+ FIX19_13_MULT(this->acceleration.x, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
		
		// update the velocity
		this->velocity.x += FIX19_13_MULT(this->acceleration.x, timeElapsed);

		// if the velocity reached a very little magnitud
		if(FIX19_13TOI(this->velocity.x) <= 0) {

			// stop movement
			this->velocity.x = 0;
			
			// stop processing
			return;
		}
	}
	else{
		
		// otherwise just calculate displacement in uniform movement
		displacement = FIX19_13_MULT(FIX19_13_MULT(this->velocity.x, timeElapsed), FIX7_9TOFIX19_13(this->direction.alpha.cos));
	}
	
	// update position
	this->transform.localPosition.x += (FIX19_13_MULT(displacement, ITOFIX19_13(this->direction.x)));

	// record this update's time
	this->timeX = Clock_getTime(_inGameClock);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// udpdate movement over axis
void Actor_keepZMovement(Actor this){

	// get the elapsed time
	fix19_13 timeElapsed = FIX19_13_DIV(ITOFIX19_13(Clock_getTime(_inGameClock) - this->timeZ ) , ITOFIX19_13(1000));
	
	// the movement displacement
	fix19_13 displacement = 0;
	
	// if no time has elapsed
	if(!timeElapsed){

		// stop processing 
		return;
	}
	
	// determine the movement type	
	if(this->moveType & (__ACCELMOVEZ | __RETARMOVEZ)){
		
		// calculate displacement based in velocity, time and acceleration
		displacement =
			FIX19_13_MULT(FIX19_13_MULT(this->velocity.z, timeElapsed), FIX7_9TOFIX19_13(this->direction.tetha.cos))
			+ FIX19_13_MULT(this->acceleration.z, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
			
		
		// update the velocity
		this->velocity.z += FIX19_13_MULT(this->acceleration.z, timeElapsed);
		
		// if the velocity reached a very little magnitud
		if(FIX19_13TOI(this->velocity.z) < 0) {
			
			// stop movement
			this->velocity.z = 0;			
			
			// stop processing
			return;
		}
	}
	else{
		
		// otherwise just calculate displacement in uniform movement
		displacement = FIX19_13_MULT(FIX19_13_MULT(this->velocity.z, timeElapsed), FIX7_9TOFIX19_13(this->direction.tetha.cos));
		displacement = FIX19_13_MULT(this->velocity.z, timeElapsed);
	}

	// update position
	this->transform.localPosition.z += (FIX19_13_MULT(displacement, ITOFIX19_13(this->direction.z)));

	// record this update's time
	this->timeZ = Clock_getTime(_inGameClock);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor_printPhysics(Actor this, int x, int y){
	
	vbjPrintText("X             Y             Z",x,y++);
	vbjPrintText("Position",x,y++);
	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.x ),x,y);
	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.y),x+14,y);
	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.z),x+14*2,y++);

	vbjPrintText("Velocity",x,y++);
	vbjPrintFloat(this->velocity.x,x,y);
	vbjPrintFloat(this->velocity.y,x+14,y);
	vbjPrintFloat(this->velocity.z,x+14*2,y++);
	vbjPrintText("Acceleration",x,y++);
	vbjPrintFloat(this->acceleration.x,x,y);
	vbjPrintFloat(this->acceleration.y,x+14,y);
	vbjPrintFloat(this->acceleration.z,x+14*2,y++);
	vbjPrintText("Direction",x,y++);
	vbjPrintInt(this->direction.x,x,y);
	vbjPrintInt(this->direction.y,x+14,y);
	vbjPrintInt(this->direction.z,x+14*2,y++);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop movement over an axis
void Actor_stopMovement(Actor this, int axis){
	
	if(__XAXIS & axis){
	
		// not moving anymore
		this->velocity.x = 0;
	}
	
	if(__YAXIS & axis){
	
		// not moving anymore
		this->velocity.y = 0;
	}	
	
	if(__ZAXIS & axis){
	
		// not moving anymore
		this->velocity.z = 0;
		
		// scale the sprite
		Sprite_scale(this->sprite);
	}	
	
	//update class's 2D position 
	Sprite_setPosition(this->sprite, &this->transform.localPosition);
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
// process a telegram
int Actor_handleMessage(Actor this, Telegram telegram){

	return StateMachine_handleMessage(this->stateMachine, telegram);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state machine
StateMachine Actor_getStateMachine(Actor this){
	
	return this->stateMachine;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// align character to other entity on collision
void Actor_alignTo(Actor this, InGameEntity entity, int axis, int pad){

	// retrieve the colliding entity's position and gap
	VBVec3D otherPosition = Entity_getPosition((Entity) entity);	
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
			
			//myHalfSize = (Entity_getWidth((Entity)this) >> 1) + (FIX19_13TOI(*myPositionAxis) > __SCREENWIDTH / 2? 1: 0);
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

		//pad -= (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1)? 1: 0);
		// align right / below
		*myPositionAxis = *otherPositionAxis +  
							ITOFIX19_13(otherHalfSize - otherHighGap
							+ myHalfSize - myLowGap
							+ pad);
	}
	else{
		//pad += (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1)? 1: 0);
		// align left
		*myPositionAxis = *otherPositionAxis -  
							ITOFIX19_13(otherHalfSize - otherLowGap
							+ myHalfSize - myHighGap
							+ pad);

	}
}	


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// does it moves?
int Actor_moves(Actor this){
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
int Actor_isMoving(Actor this){

	int result = 0;
	result |= (this->velocity.x)? __XAXIS: 0;
	result |= (this->velocity.y)? __YAXIS: 0;
	result |= (this->velocity.z)? __ZAXIS: 0;

	return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve character's scale
Scale Actor_getScale(Actor this){

	// get sprite's scale
	Scale scale = Sprite_getScale(this->sprite);
	
	// change direction
	scale.x = fabsf(scale.x) * this->direction.x;

	return scale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// play an animation
void Actor_playAnimation(Actor this, char* animationName){
	
	AnimatedSprite_play((AnimatedSprite)this->sprite, this->actorDefinition->animationDescription, animationName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is play an animation
int Actor_isPlayingAnimation(Actor this, char* functionName){
	
	return AnimatedSprite_isPlayingFunction((AnimatedSprite)this->sprite, this->actorDefinition->animationDescription, functionName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// be thrown over an axis and a direction
void Actor_beThrown(Actor this, int axis, int direction){

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset timers
void Actor_resetTimers(Actor this){

	//record initial x time	
	this->timeX = this->timeY = this->timeZ = Clock_getTime(_inGameClock);
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state when unloading the entity 
int Actor_getInGameState(Actor this){

	return this->inGameState;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the the object below me
void Actor_setObjectBelow(Actor this, InGameEntity objectBelow){
	
	this->objectBelow = objectBelow;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's position
int Actor_updateSpritePosition(Actor this){

	return (this->invalidateGlobalPosition || Actor_isMoving(this) || *((int*)_screenMovementState));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this){

	return this->velocity.z | Entity_updateSpriteScale((Entity)this);
}