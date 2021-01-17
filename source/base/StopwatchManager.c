/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StopwatchManager.h>
#include <FrameRate.h>
#include <Game.h>
#include <HardwareManager.h>
#include <MessageDispatcher.h>
#include <TimerManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StopwatchManager::getInstance()
 * @memberof	StopwatchManager
 *
 * @return		StopwatchManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void StopwatchManager::constructor()
{
	Base::constructor();

	// create the clock list
	this->stopwatchs = new VirtualList();
}

/**
 * Class destructor
 *
 * @private
 */
void StopwatchManager::destructor()
{
	VirtualNode node = this->stopwatchs->head;

	// destroy all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::destructor(node->data);
	}

	// clear my list
	delete this->stopwatchs;

	// allow a new construct
	Base::destructor();
}

/**
 * Register a clock
 *
 * @private
 * @param clock Stopwatch to register
 */
void StopwatchManager::register(Stopwatch clock)
{
	if(!VirtualList::find(this->stopwatchs, clock))
	{
		VirtualList::pushFront(this->stopwatchs, clock);
	}
}

/**
 * Un-register a clock
 *
 * @param clock Stopwatch to un-register
 */
void StopwatchManager::unregister(Stopwatch clock)
{
	VirtualList::removeElement(this->stopwatchs, clock);
}

/**
 * Update stopwatchs
 *
 */
void StopwatchManager::update()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::update(node->data);
	}
}

/**
 * Reset registered stopwatchs
 */
void StopwatchManager::reset()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::reset(node->data);
	}
}
