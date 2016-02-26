/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2016		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*  Dennis                  koracas@gmail.com
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
* What's in this file:
*	Equalizer display system master code
******************************************************************************/
#include "omega/Engine.h"

#ifdef OMEGA_OS_LINUX
#include <X11/Xlib.h>
#endif


#include "EqualizerDisplaySystem.h"
#include "omega/EventSharingModule.h"

#include "eqinternal.h"

using namespace omega;
using namespace co::base;
using namespace std;

uint sKeyFlags = 0;
unsigned int sButtonFlags = 0;
Ray sPointerRay;

// 8/12/14 LOGIC CHANGE
// Now 'button' processing for Keyboard events works same as gamepads:
// Flag is set for button down events and stays set until AFTER the corresponding
// button up, so isButtonUp(ButtonName) works consistently and as expected.
#define HANDLE_KEY_FLAG(keycode, flag) \
    if(key == keycode && type == Event::Down) sKeyFlags |= Event::flag; \
    if(key == keycode && type == Event::Up) keyFlagsToRemove |= Event::flag;

///////////////////////////////////////////////////////////////////////////////
void keyboardButtonCallback(uint key, Event::Type type)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->lockEvents();

    Event* evt = sm->writeHead();
    evt->reset(type, Service::Keyboard, key);

    uint keyFlagsToRemove = 0;

    HANDLE_KEY_FLAG(296, Alt)
        HANDLE_KEY_FLAG(292, Shift)
        HANDLE_KEY_FLAG(294, Ctrl)

        // Convert arrow keys to buttons. This allows user code to do es. 
        // evt.isButtonDown(Event::ButtonLeft) without having to make a 
        // separate call to isKeyDown when using keyboards instead of gamepads.
        HANDLE_KEY_FLAG(KC_LEFT, ButtonLeft);
    HANDLE_KEY_FLAG(KC_RIGHT, ButtonRight);
    HANDLE_KEY_FLAG(KC_DOWN, ButtonDown);
    HANDLE_KEY_FLAG(KC_UP, ButtonUp);

    // Add some special keys as buttons
    HANDLE_KEY_FLAG(KC_RETURN, Button4);
    HANDLE_KEY_FLAG(KC_BACKSPACE, Button5);
    HANDLE_KEY_FLAG(KC_TAB, Button6);
    HANDLE_KEY_FLAG(KC_HOME, Button7);

    evt->setFlags(sKeyFlags);

    // Remove the bit of all buttons that have been unpressed.
    sKeyFlags &= ~keyFlagsToRemove;

    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void mouseWheelCallback(int btn, int wheel, int x, int y)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->lockEvents();
    Event* evt = sm->writeHead();
    evt->reset(Event::Zoom, Service::Pointer);
    evt->setPosition(x, y);

    evt->setExtraDataType(Event::ExtraDataIntArray);
    evt->setExtraDataInt(0, wheel);

    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void mouseMotionCallback(int x, int y)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();

    sm->lockEvents();

    Event* evt = sm->writeHead();
    evt->reset(Event::Move, Service::Pointer);
    evt->setPosition(x, y);
    evt->setFlags(sButtonFlags);

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    sPointerRay = ds->getViewRay(Vector2i(x, y));

    evt->setExtraDataType(Event::ExtraDataVector3Array);
    evt->setExtraDataVector3(0, sPointerRay.getOrigin());
    evt->setExtraDataVector3(1, sPointerRay.getDirection());

    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void mouseButtonCallback(int button, int state, int x, int y)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();

    sm->lockEvents();

    Event* evt = sm->writeHead();
    evt->reset(state ? Event::Down : Event::Up, Service::Pointer);
    evt->setPosition(x, y);
    // Note: buttons only contain active button flags, so we invoke
    // setFlags first, to generate ButtonDown events that
    // still contain the flag of the currently presset button.
    // This is needed to make vent.isButtonUp(button) calls working.
    if(state)
    {
        sButtonFlags = button;
        evt->setFlags(sButtonFlags);
    }
    else
    {
        evt->setFlags(sButtonFlags);
        sButtonFlags = button;
    }

    evt->setExtraDataType(Event::ExtraDataVector3Array);
    evt->setExtraDataVector3(0, sPointerRay.getOrigin());
    evt->setExtraDataVector3(1, sPointerRay.getDirection());

    sm->unlockEvents();
}


////////////////////////////////////////////////////////////////////////////////
ConfigImpl::ConfigImpl( co::base::RefPtr< eq::Server > parent): 
    eq::Config(parent) 
{
    //omsg("[EQ] ConfigImpl::ConfigImpl");
    SharedDataServices::setSharedData(&mySharedData);

#ifdef OMEGA_OS_LINUX
    XInitThreads();
#endif
}

///////////////////////////////////////////////////////////////////////////////
ConfigImpl::~ConfigImpl()
{
    if(mySharedData.isAttached())
    {
        if(mySharedData.isMaster())
        {
            deregisterObject(&mySharedData);
        }
        else
        {
            unmapObject(&mySharedData);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool ConfigImpl::init()
{
    olog(Verbose, "[EQ] ConfigImpl::init");

    registerObject(&mySharedData);
    //mySharedData.setAutoObsolete(getLatency());

    SystemManager* sys = SystemManager::instance();
    
    ApplicationBase* app = sys->getApplication();
    myServer = new Engine(app);
    
    EqualizerDisplaySystem* eqds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
    eqds->finishInitialize(this, myServer);

    myServer->initialize();

    StatsManager* sm = SystemManager::instance()->getStatsManager();
    myFpsStat = sm->createStat("fps", StatsManager::Fps);

    myGlobalTimer.start();

    return eq::Config::init(mySharedData.getID());
}

///////////////////////////////////////////////////////////////////////////////
void ConfigImpl::mapSharedData(const uint128_t& initID)
{
    //omsg("[EQ] ConfigImpl::mapSharedData");
    if(!mySharedData.isAttached( ))
    {
        if(!mapObject( &mySharedData, initID))
        {
            oferror("ConfigImpl::mapSharedData: maoPobject failed (object id = %1%)", %initID);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
bool ConfigImpl::exit()
{
    //deregisterObject( &myFrameData );
    myServer->dispose();
    const bool ret = eq::Config::exit();
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
uint ConfigImpl::processMouseButtons(uint btns)
{
    uint buttons = 0;
    if((btns & eq::PTR_BUTTON1) == eq::PTR_BUTTON1) buttons |= Event::Left;
    if((btns & eq::PTR_BUTTON2) == eq::PTR_BUTTON2) buttons |= Event::Middle;
    if((btns & eq::PTR_BUTTON3) == eq::PTR_BUTTON3) buttons |= Event::Right;
    return buttons;
}

///////////////////////////////////////////////////////////////////////////////
bool ConfigImpl::handleEvent(const eq::ConfigEvent* event)
{ 
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
        {
            // BEHOLD THE MIGHTY KILL BUTTON:
            // Esc key press always posts an exit request.
            if(event->data.key.key == 256) 	
            {
                    SystemManager::instance()->postExitRequest();
            }
            else
            {
                keyboardButtonCallback( event->data.key.key , Event::Down);
            }
            return true;   
        }
        case eq::Event::KEY_RELEASE:
        {
            keyboardButtonCallback( event->data.key.key , Event::Up);
            return true;   
        }
    case eq::Event::WINDOW_POINTER_MOTION:
        {
            mouseMotionCallback(event->data.pointer.x, event->data.pointer.y);
            return true;
        }
    case eq::Event::WINDOW_POINTER_BUTTON_PRESS:
        {
            uint buttons = processMouseButtons(event->data.pointerButtonPress.buttons);
            mouseButtonCallback(buttons, 1, event->data.pointer.x, event->data.pointer.y);
            return true;
        }
    case eq::Event::WINDOW_POINTER_BUTTON_RELEASE:
        {
            uint buttons = processMouseButtons(event->data.pointerButtonPress.buttons);
            mouseButtonCallback(buttons, 0, event->data.pointer.x, event->data.pointer.y);
            return true;
        }
    case eq::Event::WINDOW_POINTER_WHEEL:
        {
            int wheel = event->data.pointerWheel.xAxis;
            uint buttons = processMouseButtons(event->data.pointerButtonPress.buttons);
            mouseWheelCallback(buttons, wheel, event->data.pointer.x, event->data.pointer.y);
            return true;
        }
    }
    return Config::handleEvent(event);
}

///////////////////////////////////////////////////////////////////////////////
uint32_t ConfigImpl::startFrame( const uint128_t& version )
{
    myServer->getDisplaySystem()->frameStarted();

    static float lt = 0.0f;
    static float tt = 0.0f;
    // Compute dt.
    float t = (float)myGlobalTimer.getElapsedTimeInSec();
    if(lt == 0) lt = t;
    
    UpdateContext uc;
    uc.dt = t - lt;
    tt += uc.dt;
    uc.time = tt;
    uc.frameNum = version.low();
    lt = t;

    mySharedData.setUpdateContext(uc);

    // Update fps stats every 10 frames.
    if(uc.frameNum % 10 == 0 && uc.dt > 0.0f)
    {
        myFpsStat->addSample(1.0f / uc.dt);
    }

    // If enabled, broadcast events to other server nodes.
    if(SystemManager::instance()->isMaster())
    {
        // Clear the event sharing queue. On cluster configs, the queue gets
        // emptied automatically when events are serialized for sending to slave
        // nodes. On single-node configs, we clear the previous frame queue here.
        EventSharingModule::clearQueue();

        ServiceManager* im = SystemManager::instance()->getServiceManager();
        im->poll();
        int av = im->getAvailableEvents();
        //ofmsg("Events: %1%", %av);
        if(av != 0)
        {
            im->lockEvents();
            // Dispatch events to application server.
            for( int evtNum = 0; evtNum < av; evtNum++)
            {
                Event* evt = im->getEvent(evtNum);

                myServer->handleEvent(*evt);
                if(!EventSharingModule::isLocal(*evt))
                {
                    uint flags = evt->getFlags();
                    evt->clearFlags();
                    evt->setFlags(flags & ~Event::Processed);
                    EventSharingModule::share(*evt);
                }
            }
            im->unlockEvents();
        }
        im->clearEvents();
    }

    // Send shared data.
    mySharedData.commit();

    myServer->update(uc);

    // NOTE: This call NEEDS to stay after Engine::update, or frames will not update / display correctly.
    uint32_t res = eq::Config::startFrame(version);;

    myServer->getDisplaySystem()->frameFinished();

    return res;
}

///////////////////////////////////////////////////////////////////////////////
void ConfigImpl::updateSharedData( )
{
    if(!mySharedData.isMaster())
    {
        // This call will update the shared data on all slave nodes.
        // All registered modules will receive updated data from the master.
        // In particular, the event sharing module will receive input events
        // from the master and will dispatch them to the local Engine instance.
        // For the event dispatch the call stack will be something like this:
        //   Engine.handleEvent
        //   EventSharingModule.updateSharedData
        //   SharedData.applyInstanceData
        //   SharedData.sync
        mySharedData.sync(co::VERSION_NEXT);
    }
}

///////////////////////////////////////////////////////////////////////////////
const UpdateContext& ConfigImpl::getUpdateContext()
{
    return mySharedData.getUpdateContext();
}
