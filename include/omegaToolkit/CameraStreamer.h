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
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials
* provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file
*	A camera listener that uses encoder plug-ins to convert rendered frames into 
*   a video stream.
*****************************************************************************/
#ifndef __CAMERA_STREAMER_H__
#define __CAMERA_STREAMER_H__

#include "omegaToolkitConfig.h"
#include "omega/Camera.h"
#include "omega/ImageUtils.h"

namespace omegaToolkit
{
    using namespace omega;

    ///////////////////////////////////////////////////////////////////////////
    //! Interface for encoder implementations
    class IEncoder
    {
    public:
        virtual ~IEncoder() {}
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual bool configure(int width, int height, int fps = 30, int quality = 100) = 0;

        virtual bool encodeFrame(RenderTarget* rt) = 0;
        virtual bool dataAvailable() = 0;
        virtual bool lockBitstream(const void** stptr, uint32_t* bytes) = 0;
        virtual void unlockBitstream() = 0;

        virtual RenderTarget::Type getRenderTargetType() { return RenderTarget::RenderToTexture; }
    };

    ///////////////////////////////////////////////////////////////////////////
    //!	A camera listener that uses plug-ins to encode rendered frames into video
    //! streams.
    class OTK_API CameraStreamer : public ICameraListener, public ReferenceType
    {
    public:
        CameraStreamer(const String& encoderName);
        ~CameraStreamer();

        void initialize(Camera* c, const DrawContext& context);
        void reset(Camera* c, const DrawContext& context);

        virtual void endDraw(Camera* cam, DrawContext& context);
        virtual void beginDraw(Camera* cam, DrawContext& context);
        virtual void startFrame(Camera* cam, const FrameInfo& frame);
        virtual void finishFrame(Camera* cam, const FrameInfo& frame);

        IEncoder* lockEncoder();
        void unlockEncoder();

        void setTargetFps(int fps) { myTargetFps = fps; }
        int getTargetFps() { return myTargetFps; }

        void setResolution(const Vector2i& r) { myResolution = r; }
        Vector2i getResolution() { return myResolution; }

        const String& getEncoderName() { return myEncoderName; }

    private:
        String myEncoderName;
        IEncoder* myEncoder;
        Vector2i myResolution;

        Ref<RenderTarget> myRenderTarget;
        Ref<Texture> myRenderTexture;
        Ref<Texture> myDepthTexture;
        Ref<PixelData> myPixels;

        // FPS stuff
        int myTargetFps;
        Timer myTimer;
        double myLastFrameTime;
        Lock myEncoderLock;
    };
};

#endif
