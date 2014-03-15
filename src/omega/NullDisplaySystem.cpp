/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
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
 *	Implements a basic 'empty' display system that can be used to launch some 
 *  applications in headless mode.
 ******************************************************************************/
#include "omega/ApplicationBase.h"
#include "omega/SystemManager.h"
#include "omega/Engine.h"
#include "omega/NullDisplaySystem.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
void NullDisplaySystem::run()
{
    SystemManager* sys = SystemManager::instance();
    ServiceManager* sm = sys->getServiceManager();

    // Setup and initialize the application server and client.
    Ref<Engine> engine;
    ApplicationBase* app = sys->getApplication();
    if(app)
    {
        engine = new Engine(app);
        engine->initialize();
    }
    

    // Initialize update context.
    UpdateContext uc;
    uc.dt = 0;
    uc.time = 0;
    uc.frameNum = 0;

    Timer timer;
    float lt = 0.0f;
    timer.start();

    while(!sys->isExitRequested())
    {
        float t = timer.getElapsedTimeInSec();
        uc.dt = t - lt;
        uc.time += uc.dt;

        // Process events.
        sm->poll();
        int av = sm->getAvailableEvents();
        if(av != 0)
        {
            sm->lockEvents();
            for(int evtNum = 0; evtNum < av; evtNum++)
            {
                Event* evt = sm->getEvent(evtNum);
                engine->handleEvent(*evt);
            }
            sm->unlockEvents();
        }

        // Update the engine state
        engine->update(uc);

        lt = t;
        // Process at ~60fps.
        osleep(16);
    }

    timer.stop();
}
