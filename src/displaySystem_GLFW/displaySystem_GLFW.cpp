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
*	A basic desktop display system using GLFW
******************************************************************************/
#include <omega.h>
#include <omegaGl.h>
#include <GLFW/glfw3.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class GLFWDisplaySystem : public DisplaySystem
{
public:
    GLFWDisplaySystem();
    virtual void run();
    virtual void cleanup();

private:
    bool myInitialized;
    Ref<GpuContext> myGpuContext;
    Ref<Renderer> myRenderer;
    Ref<Engine> myEngine;
};

#define HANDLE_KEY_FLAG(keycode, flag) \
    if(key == keycode && action == GLFW_PRESS) sKeyFlags |= Event::flag; \
    if(key == keycode && action == GLFW_RELEASE) keyFlagsToRemove |= Event::flag;

///////////////////////////////////////////////////////////////////////////////
static uint sKeyFlags;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Ignore repeat events
    if (action == GLFW_REPEAT) return;

    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->lockEvents();
    Event* evt = sm->writeHead();

    Event::Type et = Event::Down;
    if (action == GLFW_RELEASE) et = Event::Up;

    int evtkey = key + 1;

    // Map keys to characters.
    if (key >= 64 && key < 90) evtkey = key + 32;
    if (key == GLFW_KEY_TAB) evtkey = KC_TAB;
    if (key == GLFW_KEY_ESCAPE) evtkey = KC_ESCAPE;

    evt->reset(et, Service::Keyboard, evtkey);

    uint keyFlagsToRemove = 0;

    HANDLE_KEY_FLAG(GLFW_KEY_LEFT_ALT, Alt);
    HANDLE_KEY_FLAG(GLFW_KEY_LEFT_SHIFT, Shift);
    HANDLE_KEY_FLAG(GLFW_KEY_LEFT_CONTROL, Ctrl);
    HANDLE_KEY_FLAG(GLFW_KEY_RIGHT_ALT, Alt);
    HANDLE_KEY_FLAG(GLFW_KEY_RIGHT_SHIFT, Shift);
    HANDLE_KEY_FLAG(GLFW_KEY_RIGHT_CONTROL, Ctrl);

    HANDLE_KEY_FLAG(GLFW_KEY_LEFT, ButtonLeft);
    HANDLE_KEY_FLAG(GLFW_KEY_RIGHT, ButtonRight);
    HANDLE_KEY_FLAG(GLFW_KEY_UP, ButtonUp);
    HANDLE_KEY_FLAG(GLFW_KEY_DOWN, ButtonDown);

    HANDLE_KEY_FLAG(GLFW_KEY_ENTER, Button4);
    HANDLE_KEY_FLAG(GLFW_KEY_BACKSPACE, Button5);
    HANDLE_KEY_FLAG(GLFW_KEY_TAB, Button6);
    HANDLE_KEY_FLAG(GLFW_KEY_HOME, Button7);

    evt->setFlags(sKeyFlags);

    // Remove the bit of all buttons that have been unpressed.
    sKeyFlags &= ~keyFlagsToRemove;

    // If ESC is pressed, request exit.
    if (evt->isKeyDown(KC_ESCAPE)) SystemManager::instance()->postExitRequest();

    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->lockEvents();
    Event* evt = sm->writeHead();
    evt->reset(Event::Move, Service::Pointer);
    evt->setPosition((float)xpos, (float)ypos);
    evt->setFlags(sKeyFlags); 
    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void mouse_button_callback(GLFWwindow* window, int key, int action, int mods)
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->lockEvents();
    Event* evt = sm->writeHead();

    Event::Type et = Event::Down;
    if (action == GLFW_RELEASE) et = Event::Up;

    evt->reset(et, Service::Pointer);

    uint keyFlagsToRemove = 0;

    HANDLE_KEY_FLAG(GLFW_MOUSE_BUTTON_1, Button1);
    HANDLE_KEY_FLAG(GLFW_MOUSE_BUTTON_2, Button2);
    HANDLE_KEY_FLAG(GLFW_MOUSE_BUTTON_3, Button3);

    evt->setFlags(sKeyFlags);
    sKeyFlags &= ~keyFlagsToRemove;
    sm->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
static void errorCallback(int error, const char* description)
{
    oferror("[GLFW] %1% ", %description);
}

///////////////////////////////////////////////////////////////////////////////
GLFWDisplaySystem::GLFWDisplaySystem()
{
}

///////////////////////////////////////////////////////////////////////////////
void GLFWDisplaySystem::run()
{
    GLFWwindow* window;

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) oexit(-1);

    DisplayConfig& dcfg = getDisplayConfig();
    ApplicationBase* app = SystemManager::instance()->getApplication();
    myEngine = new Engine(app);
    myRenderer = new Renderer(myEngine);
    myEngine->initialize();

    // TODO: instead of guessing what API level to require, query the system and choose the
    // best one.
#ifdef OMEGA_OS_OSX
    if(dcfg.openGLCoreProfile)
    {
        // As of OSX 10.11, OpenGL 4.1 is the max spec supported.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        olog(Verbose, "[GLFWDisplaySystem::run]: OpenGL 4.1 core initializing");
    }
    else
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        olog(Verbose, "[GLFWDisplaySystem::run]: OpenGL 2.1 core initializing");
    }
#else
    if(dcfg.openGLCoreProfile)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        olog(Verbose, "[GLFWDisplaySystem::run]: OpenGL 4.2 core initializing");
    }
    else
    {
        // For compatible profiles we reuqest 3.2, the first version with
        // geometry shader support.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        olog(Verbose, "[GLFWDisplaySystem::run]: OpenGL 4.2 compatibility initializing");
    }
#endif
    
    DisplayTileConfig* tile = dcfg.tileGrid[0][0];
    Vector2i& ws = tile->pixelSize;

    if(tile->offscreen) glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(ws[0], ws[1], app->getName(), NULL, NULL);
    oassert(window != NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    myGpuContext = new GpuContext();
    myRenderer->setGpuContext(myGpuContext);
    myRenderer->initialize();

    float lt = 0.0f;
    uint64 frame = 0;
    UpdateContext uc;
    DrawContext dc;
    dc.tile = tile;
    dc.gpuContext = myGpuContext;
    dc.renderer = myRenderer;
    Timer t;
    t.start();
    bool tileEnabled = true;
    while(!SystemManager::instance()->isExitRequested())
    {
        uc.frameNum = frame;
        uc.time = t.getElapsedTimeInSec();
        uc.dt = uc.time - lt;
        lt = uc.time;

        glfwPollEvents();

        // Process events.
        ServiceManager* im = SystemManager::instance()->getServiceManager();
        int av = im->getAvailableEvents();
        if (av != 0)
        {
            im->lockEvents();
            //ofmsg("evts = %1%", %av);
            // Dispatch events to application server.
            for (int evtNum = 0; evtNum < av; evtNum++)
            {
                Event* evt = im->getEvent(evtNum);
                myEngine->handleEvent(*evt);
            }
            im->clearEvents();
            im->unlockEvents();
        }

        // NULL here is needed so glfwMakeContextCurrent after update actually
        // resets the context instead of just being a NOP.
        // Note that we need to make sure the window / gl context are current 
        // because some modules (ie osgEarth) tinker with the context during 
        // initialization and lave it in an inconsistent state.
        glfwMakeContextCurrent(NULL);

        myEngine->update(uc);

        if(tile->enabled != tileEnabled)
        {
            tileEnabled = tile->enabled;
            if(tileEnabled) glfwShowWindow(window);
            else glfwHideWindow(window);
        }

        if(tileEnabled)
        {
            glfwMakeContextCurrent(window);

            // Handle window resize
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            if(tile->activeRect.width() != width ||
                tile->activeRect.height() != height)
            {
                Vector2i ws(width, height);
                tile->activeRect.max = tile->activeRect.min + ws;
                tile->pixelSize = ws;
                tile->activeCanvasRect.max = ws;
                tile->displayConfig.setCanvasRect(tile->activeCanvasRect);
            }
            myRenderer->prepare(dc);
            oassert(!oglError);

            // Enable lighting by default (expected by native osg applications)
            // We might want to move this into the omegaOsg render pass if this
            // causes problems with other code.
#ifndef OMEGA_OS_OSX
            if(!dcfg.openGLCoreProfile) glEnable(GL_LIGHTING);
#endif
            dc.drawFrame(frame++);
            glfwSwapBuffers(window);
        }

        // Poll the service manager for new events.
        im->poll();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

///////////////////////////////////////////////////////////////////////////////
void GLFWDisplaySystem::cleanup()
{
    myEngine->dispose();
    myRenderer->dispose();
}


///////////////////////////////////////////////////////////////////////////////
// Entry point
extern "C"
#ifdef OMEGA_OS_WIN
    __declspec(dllexport)
#endif
DisplaySystem* createDisplaySystem()
{ return new GLFWDisplaySystem(); }
