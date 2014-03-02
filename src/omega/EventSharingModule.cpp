/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#include "omega/EventSharingModule.h"
#include "eqinternal/eqinternal.h"

using namespace omega;

Ref<EventSharingModule> EventSharingModule::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
EventSharingModule::EventSharingModule():
	EngineModule("EventSharingModule"),
	myQueuedEvents(0)
{
	mysInstance = this;
	enableSharedData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void EventSharingModule::share(const Event& evt)
{
	if(mysInstance != NULL)
	{
		if(mysInstance->getEngine() == NULL) 
		{
			owarn("EventSharingModule::share: server not initialized yet. Ignoring call.");
			return;
		}
	
		if(!SystemManager::instance()->isMaster())
		{
			owarn("EventSharingModule::share: can be called only from master server. Ignoring call.");
		}
		else
		{
			if(mysInstance->myQueuedEvents >= MaxSharedEventsQueue)
			{
				ofwarn("EventSharingModule::share: cannot queue more than %1% events. Dropping event.", %((int)MaxSharedEventsQueue));
			}
			else
			{
				mysInstance->myQueueLock.lock();
				mysInstance->myEventQueue[mysInstance->myQueuedEvents++].copyFrom(evt);
				mysInstance->myQueueLock.unlock();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void EventSharingModule::commitSharedData(SharedOStream& out)
{
	myQueueLock.lock();
	out << myQueuedEvents;
	int i = 0;
	while(myQueuedEvents)
	{
		EventUtils::serializeEvent(myEventQueue[i++], *out.getInternalStream());
		myQueuedEvents--;
	}
	myQueueLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void EventSharingModule::updateSharedData(SharedIStream& in)
{
	// Read the events from the network data stream, and send them to the engine for processing.
	myQueueLock.lock();
	in >> myQueuedEvents;
	if(myQueuedEvents != 0)
	{
		Engine* server = getEngine();
		//ServiceManager* sm = getEngine()->getServiceManager();
		//sm->lockEvents();
		while(myQueuedEvents)
		{
			Event evt;
			//Event* evtHead = sm->writeHead();
			EventUtils::deserializeEvent(evt, *in.getInternalStream());

			if(evt.isProcessed())
			{
				owarn("EventSharingModule::updateSharedData: received already-processed event.");
			}

			server->handleEvent(evt);

			myQueuedEvents--;
		}
		//sm->unlockEvents();
	}
	myQueueLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void EventSharingModule::dispose()
{
	omsg("EventSharingModule::dispose");
	mysInstance = NULL;
}
