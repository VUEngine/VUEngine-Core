/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <BodyManager.h>
#include <Camera.h>
#include <Collider.h>
#include <Printing.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "StatefulActor.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StatefulActor::constructor(const StatefulActorSpec* statefulActorSpec, int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor((ActorSpec*)&statefulActorSpec->actorSpec, internalId, name);

	// construct the game state machine
	this->stateMachine = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StatefulActor::destructor()
{
	// destroy state machine
	if(!isDeleted(this->stateMachine))
	{
		delete this->stateMachine;
		this->stateMachine = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool StatefulActor::handleMessage(Telegram telegram)
{
	if(!this->stateMachine || !StateMachine::handleMessage(this->stateMachine, telegram))
	{
		return Base::handleMessage(this, telegram);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StatefulActor::update()
{
	Base::update(this);

	if(!isDeleted(this->stateMachine))
	{
		StateMachine::update(this->stateMachine);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StatefulActor::createStateMachine(State state)
{
	if(isDeleted(this->stateMachine))
	{
		this->stateMachine = new StateMachine(this);
	}

	StateMachine::swapState(this->stateMachine, state);

	this->update = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
