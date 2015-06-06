/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2015, Electronic Visualization Laboratory,
* University of Illinois at Chicago
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials provided
* with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file
*	A module that shares input events with slave nodes on a cluster system
******************************************************************************/
#include "omega/EventSharingModule.h"
//#include "eqinternal/eqinternal.h"

using namespace omega;

Ref<EventSharingModule> EventSharingModule::mysInstance = NULL;

namespace omicron {
#define OS(stream, data) stream.write(&data, sizeof(data));
#define IS(stream, data) stream.read(&data, sizeof(data));
    class EventUtils
    {
    public:
        static void serializeEvent(Event& evt, SharedOStream& out)
        {
            OS(out, evt.myTimestamp);
            OS(out, evt.mySourceId);
            OS(out, evt.myDeviceTag);
            OS(out, evt.myServiceType);
            OS(out, evt.myType);
            OS(out, evt.myFlags);
            OS(out, evt.myPosition[0]); OS(out, evt.myPosition[1]); OS(out, evt.myPosition[2]);
            OS(out, evt.myOrientation.x()); OS(out, evt.myOrientation.y()); OS(out, evt.myOrientation.z()); OS(out, evt.myOrientation.w());

            // Serialize extra data
            OS(out, evt.myExtraDataType);
            OS(out, evt.myExtraDataItems);
            if (evt.myExtraDataType != Event::ExtraDataNull)
            {
                OS(out, evt.myExtraDataValidMask);
                out.write(evt.myExtraData, evt.getExtraDataSize());
            }
        }
        static void deserializeEvent(Event& evt, SharedIStream& in)
        {
            IS(in, evt.myTimestamp);
            IS(in, evt.mySourceId);
            IS(in, evt.myDeviceTag);
            IS(in, evt.myServiceType);
            IS(in, evt.myType);
            IS(in, evt.myFlags);
            IS(in, evt.myPosition[0]); IS(in, evt.myPosition[1]); IS(in, evt.myPosition[2]);
            IS(in, evt.myOrientation.x()); IS(in, evt.myOrientation.y()); IS(in, evt.myOrientation.z()); IS(in, evt.myOrientation.w());

            // Deserialize extra data
            IS(in, evt.myExtraDataType);
            IS(in, evt.myExtraDataItems);
            if (evt.myExtraDataType != Event::ExtraDataNull)
            {
                IS(in, evt.myExtraDataValidMask);
                in.read(evt.myExtraData, evt.getExtraDataSize());
            }
            if (evt.myExtraDataType == Event::ExtraDataString)
            {
                evt.myExtraData[evt.getExtraDataSize()] = '\0';
            }
        }
    private:
        EventUtils() {}
    };
};

///////////////////////////////////////////////////////////////////////////////
EventSharingModule::EventSharingModule():
	EngineModule("EventSharingModule"),
	myQueuedEvents(0)
{
	mysInstance = this;
	enableSharedData();
}

///////////////////////////////////////////////////////////////////////////////
void EventSharingModule::clearQueue()
{
	mysInstance->myQueueLock.lock();
    mysInstance->myQueuedEvents = 0;
    mysInstance->myQueueLock.unlock();
}

bool sEventDropNotified = false;

///////////////////////////////////////////////////////////////////////////////
void EventSharingModule::share(const Event& evt)
{
	if(mysInstance != NULL)
	{
		if(mysInstance->getEngine() == NULL) 
		{
			//owarn("EventSharingModule::share: server not initialized yet. Ignoring call.");
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
                if(!sEventDropNotified)
                {
                    //ofwarn("EventSharingModule::share: cannot queue more than %1% events. Dropping event.", %((int)MaxSharedEventsQueue));
                    sEventDropNotified = true;
                }
			}
			else
			{
                sEventDropNotified = false;
				mysInstance->myQueueLock.lock();
				mysInstance->myEventQueue[mysInstance->myQueuedEvents++].copyFrom(evt);
				mysInstance->myQueueLock.unlock();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void EventSharingModule::commitSharedData(SharedOStream& out)
{
	myQueueLock.lock();
	out << myQueuedEvents;
	int i = 0;
	while(myQueuedEvents)
	{
		EventUtils::serializeEvent(myEventQueue[i++], out);
		myQueuedEvents--;
	}
	myQueueLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
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
			EventUtils::deserializeEvent(evt, in);

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

///////////////////////////////////////////////////////////////////////////////
void EventSharingModule::dispose()
{
	//omsg("EventSharingModule::dispose");
	mysInstance = NULL;
}
