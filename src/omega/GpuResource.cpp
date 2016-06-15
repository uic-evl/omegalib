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
*	Classes to manage resources that are attached to a single GPU context.
******************************************************************************/
#include "omega/GpuResource.h"
#include "omega/glheaders.h"

#include "omega/Texture.h"
#include "omega/GpuProgram.h"
#include "omega/GpuBuffer.h"
#include "omega/RenderTarget.h"
#include "omega/SystemManager.h"

using namespace omega;

uint GpuContext::mysNumContexts = 0;
Lock GpuContext::mysContextLock = Lock();

///////////////////////////////////////////////////////////////////////////////
GpuContext::GpuContext(GLEWContext* ctx)
{
    oassert(!oglError);
    myOwnGlewContext = false;
    mysContextLock.lock();
    myId = mysNumContexts++;

    // Initialize Glew
    myGlewContext = ctx;
    if(myGlewContext == NULL)
    {
        myGlewContext = new GLEWContext();
        myOwnGlewContext = true;
        glewSetContext(myGlewContext);
        oflog(Debug, "[GpuContext::GpuContext] <%1%> Glew init", %myId);
        glewExperimental = GL_TRUE;
        glewInit();
    }
    
    // Get and ignore error right after glewInit().
    // See http://stackoverflow.com/questions/14046111/glewinit-apparently-successful-sets-error-flag-anyway
    glGetError();
    
    mysContextLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void GpuContext::makeCurrent()
{
    oassert(myGlewContext != NULL);
    glewSetContext(myGlewContext);
}

///////////////////////////////////////////////////////////////////////////////
GpuContext::~GpuContext()
{
    foreach(GpuResource* res, myResources) res->dispose();
    myResources.clear();

    if(myOwnGlewContext) delete myGlewContext;
    myGlewContext = NULL;
}

///////////////////////////////////////////////////////////////////////////////
Texture* GpuContext::createTexture()
{
    Texture* tex = new Texture(this);
    myResources.push_back(tex);
    return tex;
}

///////////////////////////////////////////////////////////////////////////////
RenderTarget* GpuContext::createRenderTarget(uint type)
{
    RenderTarget* rt = new RenderTarget(this, (RenderTarget::Type)type);
    myResources.push_back(rt);
    return rt;
}

///////////////////////////////////////////////////////////////////////////////
GpuProgram* GpuContext::createProgram()
{
    GpuProgram* p = new GpuProgram(this);
    myResources.push_back(p);
    return p;
}

///////////////////////////////////////////////////////////////////////////////
GpuBuffer* GpuContext::createVertexBuffer()
{
    GpuBuffer* p = new GpuBuffer(this);
    myResources.push_back(p);
    return p;
}

///////////////////////////////////////////////////////////////////////////////
GpuArray* GpuContext::createVertexArray()
{
    GpuArray* p = new GpuArray(this);
    myResources.push_back(p);
    return p;
}

///////////////////////////////////////////////////////////////////////////////
void GpuContext::garbageCollect()
{
    bool shuttingDown = SystemManager::instance()->isExitRequested();
    // Dispose of unused resources. When shutting down, clean everything.
    List<GpuResource*> txlist;
    foreach(GpuResource* tex, myResources)
    {
        if(tex->refCount() == 1 || shuttingDown)
        {
            tex->dispose();
            txlist.push_back(tex);
        }
    }
    foreach(GpuResource* gr, txlist) myResources.remove(gr);
}