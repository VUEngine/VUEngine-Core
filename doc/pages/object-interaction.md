Object interaction
==================

The engine supports various decoupled schemes for interaction between objects at the foundation of the whole class hierarchy.
The `Object` class defines methods for message passing and for event propagation.


Messaging
---------

Message between objects are carried out by telegrams whose dispatching is managed by a singleton of the `MessageDispatcher` class.

	MessageDispatcher::dispatchMessage(delayInMilliseconds, sender, receiver, messageEnum, extraInfoPointer);
	
If the delay argument is zero, the message is delivered at the moment of the dispatch request. If a value greater than zero is passed as the first argument, then the message will be delivered once the delay expires.

Delayed messages must always be sent between objects that you know will be alive at the moment of delivery. Therefore you need to make sure to cancel any delayed messages in the destructor of each class that uses them. This is done with the `MessageDispatcher::discardDelayedMessages` method.

The `Object`'s handleMessage method can be overrode to specify each class' behavior.

States don't override the handleMessage method, instead, they define a processMessage method that is called by the `StateMachine` class and can be overrode by derived class for specialization. The `StateMachine`'s owner must receive the `Telegram` by overriding the `Object`'s handleMessage method and then forward the telegram by calling the `StateMachine` handleMessage method.  


Events
------

The engine supports event propagation at the `Object` level too.

To listen for an event, add a listener with the following code:

	Object::addEventListener(triggerObject, listener, eventListenerMethod, eventCode);

To fire an event, call the following code:

	Object::fireEvent(triggerObject, eventCode);
	
The removal of listeners is the responsibility of the client code. To remove a listener use one of the following:

	Object::removeEventListener(triggerObject, listener, method, eventCode);
	Object::removeEventListeners(listener, eventCode);
	Object::removeEventListenerScopes(triggerObject, eventCode);
	Object::removeAllEventListeners(triggerObject, eventCode);

Beware: During the processing of an event it is illegal to modify the firer's event list by adding or removing event listeners to it, or by deleting the firer. If any of that happens, the engine will trigger an exception.


Message propagation
-------------------

The Container implements a passMessage method that allows the propagation of a message to all its descendants. For example, to propagate a message through all the descendants of a Stage use the following call:

	Container::propagateMessage(stage, Container::onPropagatedMessage, message);
 
Then, to react to the message, override the following method in your class:

	bool Container::handlePropagatedMessage(int message);

Message propagation stops when handlePropagatedMessage returns true.
