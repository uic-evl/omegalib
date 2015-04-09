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
*	The base class for classes that perform drawing on the render thread.
******************************************************************************/
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "DrawInterface.h"

namespace omega {
    class Renderer;

    ///////////////////////////////////////////////////////////////////////////
    //! The base class for classes that perform drawing on the render thread.
    class OMEGA_API RenderPass: public ReferenceType
    {
    public:
        RenderPass(Renderer* client, const String& name, int priority = 0): 
          myInitialized(false), myDisposeRequested(false), 
              myClient(client), myName(name), myCameraMask(0),
              myPriority(priority)
          {}
        virtual ~RenderPass()
        { /*ofmsg("~RenderPass %1%", %myName);*/ }

        virtual void initialize() { myInitialized = true; }

        //! Prepares the render pass for rendering.
        //! @remarks this method runs once per frame. Unlike RenderPass::render
        //! this method runs even if the current tile is disabled. This lets the
        //! render pass allocate/deallocate gpu resources depending on the tile
        //! state.
        virtual void prepare(Renderer* client, const DrawContext& context) {}

        //! Performs rendering.
        //! @remarks this method is called multiple times for each frames,
        //! depending on enabled cameras, stereo modes, etc. If rendering on
        //! a tile is disabled, this RenderPass::render won't run on that tile. 
        virtual void render(Renderer* client, const DrawContext& context) = 0;
        virtual void dispose() {}
        void requestDispose() { myDisposeRequested = true; }
        bool needsDispose() { return myDisposeRequested; }

        const String& getName() { return myName; }

        void setUserData(void* value) { myUserData = value; }
        void* getUserData() { return myUserData; }

        bool isInitialized() { return myInitialized; }

        Renderer* getClient() { return myClient; }

        void setCameraMask(uint mask) { myCameraMask = mask; }
        uint getCameraMask() { return myCameraMask; }

        //! Returns the render pass priority. Render pass priorities are used to
        //! order render passes. Render passes with high priority get rendered
        //! first.
        int getPriority() { return myPriority; }

    private: 
        bool myInitialized;
        bool myDisposeRequested;
        void* myUserData;
        String myName;
        Renderer* myClient;
        unsigned int myCameraMask;
        int myPriority;
    };

}; // namespace omega

#endif