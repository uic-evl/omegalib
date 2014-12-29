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
 *	The Equalizer channel implementation: this class is the entry point for
 *	every rendering operation. It sets up the draw context and calls the
 *  omegalib Renderer.draw method to perform the actual rendering.
 ******************************************************************************/
#include "eqinternal.h"
#include "omega/DisplaySystem.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

using namespace omega;
using namespace co::base;
using namespace std;

using namespace eq;

///////////////////////////////////////////////////////////////////////////////
ChannelImpl::ChannelImpl( eq::Window* parent ) 
    :eq::Channel( parent ), myWindow((WindowImpl*)parent), myDrawBuffer(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////
ChannelImpl::~ChannelImpl() 
{}

///////////////////////////////////////////////////////////////////////////////
bool ChannelImpl::configInit(const eq::uint128_t& initID)
{
    eq::Channel::configInit(initID);

    EqualizerDisplaySystem* ds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
    String name = getName();

	if(ds->getDisplayConfig().tiles.find(name) == ds->getDisplayConfig().tiles.end())
	{
		oferror("ChannelImpl::configInit: could not find tile %1%", %name);
	}
	else
	{
		myDC.tile = ds->getDisplayConfig().tiles[name];
	}

    Renderer* client = myWindow->getRenderer();
    myDC.gpuContext = client->getGpuContext();
	myDC.renderer = client;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::frameDraw( const co::base::uint128_t& frameID )
{
	// Pass the current tile to the draw context. The tile contains all the 
	// properties of the current draw surface.
	myDC.tile = myWindow->getTileConfig();

    if(myDC.tile->enabled)
    {
        // (spin is 128 bits, gets truncated to 64... 
        // do we really need 128 bits anyways!?)
        myDC.drawFrame(frameID.low());
    }
	
	// NOTE: This call NEEDS to stay after drawFrames, or frames will not 
	// update / display correctly.
    eq::Channel::frameDraw( frameID );
}

///////////////////////////////////////////////////////////////////////////////
omega::Renderer* ChannelImpl::getRenderer()
{
    WindowImpl* window = static_cast<WindowImpl*>(getWindow());
    return window->getRenderer();
}
