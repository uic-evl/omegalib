/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2016		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2016, Electronic Visualization Laboratory,  
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
 *	The Camera output class is used to simplify and optimize reading back 
 *  rendered frames to main memory.
 ******************************************************************************/
#ifndef __CAMERA_OUTPUT_H__
#define __CAMERA_OUTPUT_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/SceneNode.h"
#include "omega/RenderTarget.h"

namespace omega {
    struct FrameInfo;

    ///////////////////////////////////////////////////////////////////////////
    //!	The Camera output class is used to simplify and optimize reading back 
    //! rendered frames to main memory. Each camera has multiple output available,
    //! one for each GPU device active in the current configuration.
    //! The general workflow is as follows:
    //!		1. Create a PixelData object to store the output frame
    //!		2. Attach the PixelData object to one of the outputs of a camera
    //!		3. enable the camera output - the pixel data will now be updated
    //!		at the end of each frame
    class OMEGA_API CameraOutput: public ReferenceType
    {
    public:
        CameraOutput();

        void reset(RenderTarget::Type type);

        void beginDraw(const DrawContext& context);
        void endDraw(const DrawContext& context);
        void startFrame(const FrameInfo& frame);
        void finishFrame(const FrameInfo& frame);

        bool isEnabled() { return myEnabled; }
        void setEnabled(bool value) { myEnabled = value; }

        void setReadbackTarget(PixelData* color, PixelData* depth = NULL);
        void setReadbackTargetAndViewport(PixelData* color, PixelData* depth, const Rect& readbackViewport);
        void setTextureTarget(Texture* color, Texture* depth = NULL);
        void setTextureTargetAndViewport(Texture* color, Texture* depth, const Rect& readbackViewport);

        const Rect& getReadbackViewport() { return myReadbackViewport; }

        RenderTarget* getRenderTarget() { return myRenderTarget; }
        RenderTarget::Type getType() { return myType; }

        //void lock() { myLock.lock();}
        //void unlock() { myLock.unlock();}

    private:
        bool myEnabled;

        RenderTarget::Type myType;
        Ref<RenderTarget> myRenderTarget;

        PixelData* myReadbackColorTarget;
        PixelData* myReadbackDepthTarget;
        Texture* myTextureColorTarget;
        Texture* myTextureDepthTarget;

        Rect myReadbackViewport;

        Lock myLock;
    };
}; // namespace omega

#endif