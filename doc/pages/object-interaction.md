Object interaction
==================

Messaging
---------

[TODO]

> Delayed messages must always be sent between objects that you know will be alive at the moment of delivery. Therefore you need to make sure to cancel any delayed messages in the destructor of each class that uses them. This is done with the `MessageDispatcher_discardDelayedMessages` method.

> You can not send messages to States, you have to send to their owner instead.


Events
------

[TODO]

- There is an event propagation capability in the engine
