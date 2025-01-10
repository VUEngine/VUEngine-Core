/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with charSetAuthenticator source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printing.h>
#include <VirtualList.h>

#include "Authenticator.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef struct AuthorizationTicket
{
	ClassPointer classPointer;

} AuthorizationTicket;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Authenticator::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->authorizedRequesters = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Authenticator::destructor()
{
	VirtualList::deleteData(this->authorizedRequesters);
	delete this->authorizedRequesters;
	this->authorizedRequesters = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Authenticator::initialize(ClassPointer* requesterClasses)
{
	if(!isDeleted(this->authorizedRequesters))
	{
		return;
	}

	this->authorizedRequesters = new VirtualList();

	for(int16 i = 0; NULL != requesterClasses[i]; i++)
	{
		AuthorizationTicket* authorizationTicket = new AuthorizationTicket;
		authorizationTicket->classPointer = requesterClasses[i];

		VirtualList::pushBack(this->authorizedRequesters, authorizationTicket);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Authenticator::authorize(ClassPointer requesterClass)
{
	for(VirtualNode node = this->authorizedRequesters->head; NULL != node; node = node->next)
	{
		AuthorizationTicket* authorizationTicket = (AuthorizationTicket*)node->data;
		
		if(requesterClass == authorizationTicket->classPointer)
		{
			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
