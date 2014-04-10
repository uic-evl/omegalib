/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
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
 *	The interface between omegalib and an Equalizer window. A window is exactly 
 *	what you would expect: a (possibly fullscreen) window on a screen, 
 *	associated with an OpenGL context. Each Equalizer node may control multiple
 *  windows.
 ******************************************************************************/
#include "omega/RenderTarget.h"
#include "omega/EqualizerDisplaySystem.h"

#include "eqinternal.h"

using namespace omega;
using namespace co::base;
using namespace std;
    
///////////////////////////////////////////////////////////////////////////////
WindowImpl::WindowImpl(eq::Pipe* parent): 
    eq::Window(parent), myPipe((PipeImpl*)parent),
	myVisible(true)
	//myIndex(Vector2i::Zero())
{}

///////////////////////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl() 
{}

omicron::Lock sInitLock;
///////////////////////////////////////////////////////////////////////////////
bool WindowImpl::configInit(const uint128_t& initID)
{
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
		myRenderer->setGpuContext(myPipe->getGpuContext());
		myRenderer->initialize();
	}
	else return false;

	// Serialize window init execution since we are tinkering with x cursors on linux inside there.
	sInitLock.lock();
	bool res = Window::configInit(initID);
	sInitLock.unlock();
	return res;
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
        newEvt.pointer.x = event.pointer.x + myTile->offset[0];
        newEvt.pointer.y = event.pointer.y + myTile->offset[1];
        return eq::Window::processEvent(newEvt);
    }
	else if(event.type == eq::Event::WINDOW_RESIZE)
	{
		myTile->pixelSize[0] = event.resize.w;
		myTile->pixelSize[1] = event.resize.h;

        // Update the canvas size.
        Vector2i tileEndPoint = myTile->offset + myTile->pixelSize;
        DisplayConfig& dc = getDisplaySystem()->getDisplayConfig();
        dc.canvasPixelSize =
            dc.canvasPixelSize.cwiseMax(tileEndPoint);
    }

    // Other events: just send to application node.
    return eq::Window::processEvent(event);
}

///////////////////////////////////////////////////////////////////////////////
void WindowImpl::frameStart( const uint128_t& frameID, const uint32_t frameNumber )
{
	eq::Window::frameStart(frameID, frameNumber);

    // Invert interleaver based on window position, WIP
    //int windowY = getPixelViewport().y;
    //myTile->invertStereo = windowY % 2;

	// Did the local tile visibility state change?
	if(myVisible != myTile->enabled)
	{
		myVisible = myTile->enabled;
		if(myVisible) getSystemWindow()->show();
		else getSystemWindow()->hide();
	}
	// Activate the glew context for this pipe, so initialize and update client
	// methods can handle openGL buffers associated with this Pipe.
	// NOTE: getting the glew context from the first window is correct since all
	// windows attached to the same pape share the same Glew (and OpenGL) contexts.
	// NOTE2: do NOT remove these two lines. rendering explodes if you do.
	const GLEWContext* glewc = glewGetContext();
	glewSetContext(glewc);
}

///////////////////////////////////////////////////////////////////////////////
Renderer* WindowImpl::getRenderer() 
{ 
	return myRenderer.get(); 
}

