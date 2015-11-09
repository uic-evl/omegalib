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
 *	The interface between omegalib and an Equalizer window. A window is exactly 
 *	what you would expect: a (possibly fullscreen) window on a screen, 
 *	associated with an OpenGL context. Each Equalizer node may control multiple
 *  windows.
 ******************************************************************************/
#include "omega/RenderTarget.h"
#include "omega/glheaders.h"
#include "EqualizerDisplaySystem.h"

#include "eqinternal.h"

using namespace omega;
using namespace co::base;
using namespace std;
    
///////////////////////////////////////////////////////////////////////////////
WindowImpl::WindowImpl(eq::Pipe* parent): 
    eq::Window(parent),
    myVisible(false), mySkipResize(false)
    //myIndex(Vector2i::Zero())
{
}

///////////////////////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl() 
{}

omicron::Lock sInitLock;
///////////////////////////////////////////////////////////////////////////////
bool WindowImpl::configInit(const uint128_t& initID)
{
    // Serialize window init execution since we are tinkering with x cursors on linux inside there.
    sInitLock.lock();
    bool res = Window::configInit(initID);
    sInitLock.unlock();

    // Get the tile index from the window name.
    String name = getName();
    
    EqualizerDisplaySystem* ds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
    if(ds->getDisplayConfig().tiles.find(name) == ds->getDisplayConfig().tiles.end())
    {
        oferror("WindowImpl::configInit: could not find tile %1%", %name);
    }
    else
    {
        myTile = ds->getDisplayConfig().tiles[name];
    }

    ApplicationBase* app = SystemManager::instance()->getApplication();
    if(app)
    {
        myRenderer = new Renderer(Engine::instance());
        myGpuContext = new GpuContext(const_cast<GLEWContext*>(this->glewGetContext()));
        myRenderer->setGpuContext(myGpuContext);
        myRenderer->initialize();
    }
    else return false;

    oflog(Debug, "[WindowImpl::configInit] <%1%> done", %initID);
    return res;
}

///////////////////////////////////////////////////////////////////////////////
bool WindowImpl::configExit()
{
    myRenderer->dispose();
    myRenderer = NULL;

    return Window::configExit();
}

///////////////////////////////////////////////////////////////////////////////
bool WindowImpl::processEvent(const eq::Event& event) 
{
    // Pointer events: convert the mouse position from local (tile-based) to global (canvas-based)
    if(
        event.type == eq::Event::WINDOW_POINTER_BUTTON_PRESS ||
        event.type == eq::Event::WINDOW_POINTER_BUTTON_RELEASE ||
        event.type == eq::Event::WINDOW_POINTER_MOTION ||
        event.type == eq::Event::WINDOW_POINTER_WHEEL)
    {
        //const Vector2i& ts = getDisplaySystem()->getDisplayConfig().tileResolution;
        eq::Event newEvt = event;
        newEvt.pointer.x = event.pointer.x + myTile->activeCanvasRect.min[0];
        newEvt.pointer.y = event.pointer.y + myTile->activeCanvasRect.min[1];
        return eq::Window::processEvent(newEvt);
    }
    else if(event.type == eq::Event::WINDOW_RESIZE)
    {
        // NOTE: we skip the first frame since a resize event on the first
        // frame tries to set the window to the full tile size, and we don't want
        // that.
        if(myVisible && event.statistic.frameNumber > 0)
        {
            if(myTile->activeRect.width() != event.resize.w ||
                myTile->activeRect.height() != event.resize.h)
            {
                myTile->activeRect.max =
                    myTile->activeRect.min +
                    Vector2i(event.resize.w, event.resize.h);

                myTile->pixelSize = Vector2i(event.resize.w, event.resize.h);
                myTile->activeCanvasRect.max = Vector2i(event.resize.w, event.resize.h);
                myTile->displayConfig.setCanvasRect(myTile->activeCanvasRect);
                // Skip next size/move in WindowImpl::frameStart
                myCurrentRect = myTile->activeRect;
            }
        }
    }

    // Other events: just send to application node.
    return eq::Window::processEvent(event);
}

///////////////////////////////////////////////////////////////////////////////
void WindowImpl::frameStart(const uint128_t& frameID, const uint32_t frameNumber)
{
    eq::Window::frameStart(frameID, frameNumber);

    // Invert interleaver based on window position, WIP
    //int windowY = getPixelViewport().y;
    //myTile->invertStereo = windowY % 2;
    // Did the local tile visibility state change?
    if (myVisible != myTile->enabled)
    {
        myVisible = myTile->enabled;
        if (myTile->enabled)
        {
            oflog(Debug, "WindowImpl: showing window %1%", %getName());

            // The window switched back to visible.
            // show it and bring it to front.
            getSystemWindow()->show();
            getSystemWindow()->bringToFront();
        }
        else
        {
            oflog(Debug, "WindowImpl: hiding window %1%", %getName());

            getSystemWindow()->hide();
            myCurrentRect.min = Vector2i::Zero();
            myCurrentRect.max = Vector2i::Zero();
            return;
        }
    }

    // Bring this window to front if needed.
    if (myVisible && myTile->displayConfig.isBringToFrontRequested())
    {
        getSystemWindow()->bringToFront();
    }

    // Did the window position / size change?
    if (myCurrentRect.min != myTile->activeRect.min ||
        myCurrentRect.max != myTile->activeRect.max)
    {
        myCurrentRect = myTile->activeRect;

        // If window is smaller that 10x10 just hide it. Done to avoid X errros.
        if (myCurrentRect.width() < 10 || myCurrentRect.height() < 10)
        {
            myTile->enabled = false;
        }
        else
        {
            //myTile->enabled = true;
            getSystemWindow()->move(
                myCurrentRect.x(),
                myCurrentRect.y(),
                myCurrentRect.width(),
                myCurrentRect.height());
        }
    }

    // Activate the glew context for this pipe, so initialize and update client
    // methods can handle openGL buffers associated with this Pipe.
    // NOTE: getting the glew context from the first window is correct since all
    // windows attached to the same pape share the same Glew (and OpenGL) contexts.
    // NOTE2: do NOT remove these two lines. rendering explodes if you do.
    const GLEWContext* glewc = myGpuContext->getGlewContext();
    //myRenderer->getGpuContext()->setGlewContext(glewc);
    myGpuContext->makeCurrent();
    oassert(glewc != NULL);
    glewSetContext(glewc);
}

///////////////////////////////////////////////////////////////////////////////
Renderer* WindowImpl::getRenderer() 
{ 
    return myRenderer.get(); 
}

