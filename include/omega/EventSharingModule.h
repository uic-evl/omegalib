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
#ifndef __EVENT_SHARING_MODULE_H__
#define __EVENT_SHARING_MODULE_H__

#include "omega/osystem.h"
#include "omega/ModuleServices.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API EventSharingModule: public EngineModule
	{
	public:
		//! Max number of events.
		static const int MaxSharedEventsQueue = 640;

		//! Flag for local events.
		static const uint LocalEventFlag = Event::User << 2;

	public:
		static void markLocal(const Event& evt);
		static bool isLocal(const Event& evt);
		static void share(const Event& evt);

		EventSharingModule();

		virtual void commitSharedData(SharedOStream& out);
		virtual void updateSharedData(SharedIStream& in);
		virtual void dispose();

	private:
		static Ref<EventSharingModule> mysInstance;

		Lock myQueueLock;
		Event myEventQueue[MaxSharedEventsQueue];
		int myQueuedEvents;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline void EventSharingModule::markLocal(const Event& evt)
	{
		evt.setFlags(LocalEventFlag);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline bool EventSharingModule::isLocal(const Event& evt)
	{
		return evt.isFlagSet(LocalEventFlag);
	}

}; // namespace omega

#endif