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
*	Support for offscreen rendering to various targets
******************************************************************************/
#include "omega/RenderTarget.h"
#include "omega/Texture.h"
#include "omega/PixelData.h"
#include "omega/DisplaySystem.h"
#include "omega/glheaders.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(GpuContext* context, Type type, GLuint id):
    GpuResource(context),
    myType(type),
    myId(id),
    myRbColorId(0),
    myRbDepthId(0),
    myRbWidth(0),
    myRbHeight(0),
    myTextureColorTarget(NULL),
    myTextureDepthTarget(NULL),
    myBound(false),
    myClearDepth(true),
    myClearColor(true)
{
    if(myType != RenderOnscreen && myId == 0)
    {
    	oassert(!oglError);
        glGenFramebuffers(1, &myId);
        oassert(!oglError);
    }
}

///////////////////////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget()
{
}

///////////////////////////////////////////////////////////////////////////////
void RenderTarget::dispose() 
{
    if(myId != 0)
    {
        oflog(Verbose, "[RenderTarget::dispose] id=<%1%>", %myId);
        
        glDeleteFramebuffers(1, &myId);
        myId = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
void RenderTarget::setTextureTarget(Texture* color, Texture* depth)
{
    if(myType != RenderToTexture)
    {
        owarn("RenderTarget::setTextureTarget: this rendertarget is not of RenderToTexture type.");
    }
    else
    {
        myTextureColorTarget = color;
        myTextureDepthTarget = depth;
    }
}

///////////////////////////////////////////////////////////////////////////////
void RenderTarget::setReadbackTarget(PixelData* color, PixelData* depth)
{
    myReadbackColorTarget = color;
    myReadbackDepthTarget = depth;
    if(myReadbackColorTarget != NULL)
    {
        myReadbackViewport = Rect(
            0, 0, 
            myReadbackColorTarget->getWidth(), myReadbackColorTarget->getHeight());
    }
}

///////////////////////////////////////////////////////////////////////////////
void RenderTarget::setReadbackTarget(PixelData* color, PixelData* depth, const Rect& readbackViewport)
{
    setReadbackTarget(color, depth);
    myReadbackViewport = readbackViewport;
}

///////////////////////////////////////////////////////////////////////////////
int RenderTarget::getWidth() 
{ 
    if(myType == RenderToTexture && myTextureColorTarget != NULL)
    {
        return myTextureColorTarget->getWidth();
    } 
    return myRbWidth;
}

///////////////////////////////////////////////////////////////////////////////
int RenderTarget::getHeight() 
{ 
    if(myType == RenderToTexture && myTextureColorTarget != NULL)
    {
        return myTextureColorTarget->getHeight();
    }
    return myRbHeight;
}

///////////////////////////////////////////////////////////////////////////////////
void RenderTarget::unbind()
{
    //ofmsg("      RenderTarget %1% unbind", %myId);
	bool coreProfile = SystemManager::instance()->getDisplaySystem()->getDisplayConfig().openGLCoreProfile;

    if(myBound)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        myBound = false;
        if(!coreProfile) glPopAttrib();

        // v10.1, 18Nov15: If we are rendering to a texture, set the viewport 
        // to the full texture by default.
        if((myType == RenderToTexture && myTextureColorTarget != NULL)
            || myType == RenderOffscreen)
        {
            if(!coreProfile) glPopAttrib();
        }
        
        oassert(!oglError);
    }
}

///////////////////////////////////////////////////////////////////////////////////
void RenderTarget::readback()
{
    //ofmsg("      RenderTarget %1% readback", %myId);
    bool needBinding = false;

    if(myType != RenderOnscreen && !myBound) needBinding = true;
    if(needBinding) bind();

    if(myReadbackColorTarget != NULL)
    {
        if(myReadbackColorTarget->getFormat() == PixelData::FormatRgb)
        {
            GLvoid* target = myReadbackColorTarget->bind(getContext());
            // NOTE: DO NOT CHANGE REDBACK BYTE ORDERING HERE. IT IS CORRECT
            // INFERTING BYTE ORDERING MAKES IT LOOK WRONG IN PORTHOLE
            glReadPixels(
                myReadbackViewport.x(), myReadbackViewport.y(), 
                myReadbackViewport.width(), myReadbackViewport.height(), GL_RGB, GL_UNSIGNED_BYTE, 
                target);
            myReadbackColorTarget->unbind();
        }
        else if(myReadbackColorTarget->getFormat() == PixelData::FormatRgba)
        {
            GLvoid* target = myReadbackColorTarget->bind(getContext());
            // NOTE: DO NOT CHANGE REDBACK BYTE ORDERING HERE. IT IS CORRECT
            // INFERTING BYTE ORDERING MAKES IT LOOK WRONG IN PORTHOLE
            glReadPixels(
                myReadbackViewport.x(), myReadbackViewport.y(), 
                myReadbackViewport.width(), myReadbackViewport.height(), GL_RGBA, GL_UNSIGNED_BYTE, 
                target);
            myReadbackColorTarget->unbind();
        }
        myReadbackColorTarget->setDirty();
    }
    if(myReadbackDepthTarget != NULL)
    {
        GLvoid* target = myReadbackDepthTarget->bind(getContext());
        glReadPixels(
            myReadbackViewport.x(), myReadbackViewport.y(), 
            myReadbackViewport.width(), myReadbackViewport.height(), GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 
            target);
        myReadbackDepthTarget->unbind();
        myReadbackColorTarget->setDirty();
    }
    if(needBinding) unbind();
}

///////////////////////////////////////////////////////////////////////////////////
void RenderTarget::bind()
{
    //ofmsg("      RenderTarget %1% bind", %myId);
	bool coreProfile = SystemManager::instance()->getDisplaySystem()->getDisplayConfig().openGLCoreProfile;
	
    glBindFramebuffer(GL_FRAMEBUFFER, myId);

    myBound = true;

    // Disable scissor test for render targets
    if(!coreProfile) glPushAttrib(GL_SCISSOR_BIT);
    glDisable(GL_SCISSOR_TEST);
    //glDisable(GL_STENCIL_TEST);

    if(myType == RenderToTexture)
    {
        if(myTextureColorTarget != NULL)
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, myTextureColorTarget->getGLTexture(), 0);
            oassert(!oglError);

            // v10.1, 18Nov15: If we are rendering to a texture, set the viewport 
            // to the full texture by default.
            if(!coreProfile) glPushAttrib(GL_VIEWPORT_BIT);
            glViewport(0, 0, getWidth(), getHeight());
        }
        if(myTextureDepthTarget != NULL)
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, myTextureDepthTarget->getGLTexture(), 0);
            oassert(!oglError);
        }
    }
    else if(myType == RenderOffscreen)
    {
        // See if we need to create a renderbuffer or update its storage.
        if(myRbColorId == 0)
        {
            glGenRenderbuffers(1, &myRbColorId);
            glGenRenderbuffers(1, &myRbDepthId);
            oassert(!oglError);
        }
        if(myRbWidth != myReadbackColorTarget->getWidth() || myRbHeight != myReadbackColorTarget->getHeight())
        {
            myRbWidth = myReadbackColorTarget->getWidth();
            myRbHeight = myReadbackColorTarget->getHeight();
            glBindRenderbuffer(GL_RENDERBUFFER, myRbColorId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, myRbWidth, myRbHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, myRbDepthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, myRbWidth, myRbHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            myReadbackViewport = Rect(
                0, 0,
                myReadbackColorTarget->getWidth(), myReadbackColorTarget->getHeight());
        }

        // v10.2, 2Dec15: If we are rendering to a buffer, set the viewport 
        // to the full texture by default.
        if(!coreProfile) glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0, 0, myRbWidth, myRbHeight);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, myRbColorId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, myRbDepthId);
    }
}

///////////////////////////////////////////////////////////////////////////////////
void RenderTarget::clear()
{
    //ofmsg("      RenderTarget %1% clear", %myId);

    bool needBinding = false;

    if(!myBound) needBinding = true;
    if(needBinding) bind();
    
    glClear(
        (myClearColor ? GL_COLOR_BUFFER_BIT : 0) |
        (myClearDepth ? GL_DEPTH_BUFFER_BIT : 0));

    if(needBinding) unbind();
}
