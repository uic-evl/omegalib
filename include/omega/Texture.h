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
*	A wrapper around an OpenGL Texture
******************************************************************************/
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/GpuResource.h"

namespace omega
{
    class PixelData;

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API Texture: public GpuResource
    {
        friend class GpuContext;
    public:
        static void enablePboTransfers(bool value) { sUsePbo = value; }

        //! Texture types
        enum TextureType {
            Type2D,
            TypeRectangle
        };

        //! Channel types
        enum ChannelType {
            ChannelRGBA,
            ChannelRGB,
            ChannelDepth
        };

        //! Channel format
        enum ChannelFormat {
            FormatUInt,
            FormatUByte,
            FormatFloat
        };

    public:
        //! Initializes this texture object using a specific texture type,
        //! channel type and channel format.
        void initialize(int width, int height, 
            TextureType tt = Type2D, 
            ChannelType ct = ChannelRGBA, 
            ChannelFormat cf = FormatUByte);
        
        //! Resets the size of this texture without changing its format.
        void resize(int width, int height);

        bool isInitialized() { return myInitialized; }

        //! Write pixels from a PixelData object
        void writePixels(PixelData* data);
        //! Write pixels to a PixelData object
        void readPixels(PixelData* data);

        //! Write pixels from a memory buffer.
        //! @param format - a pixel format such as GL_RGB, GL_RGBA, etc.
        void writeRawPixels(const byte* data, int width, int height, uint format);

        int getWidth();
        int getHeight();

        TextureType getTextureType();
        ChannelType getChannelType();
        ChannelFormat getChannelFormat();

        //! Texture operations
        //@{
        GLuint getGLTexture();
        void bind(GpuContext::TextureUnit unit);
        void unbind();
        void refresh();
        bool isBound();
        GpuContext::TextureUnit getTextureUnit();
        //@}

        virtual void dispose();
        
    protected:
        // Only renderer can allocate textures.
        Texture(GpuContext* context);

    private:
        static bool sUsePbo;

        bool myInitialized;
        int myWidth;
        int myHeight;

        TextureType myTextureType;
        ChannelType myChannelType;
        ChannelFormat myChannelFormat;

        // GL stuff
        GLuint myId;
        uint myGlFormat;
        GLuint myPboId;

        GpuContext::TextureUnit myTextureUnit;
    };

    struct DrawContext;

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API TextureSource : public ReferenceType
    {
    public:
        TextureSource() :
            myTextureUpdateFlags(0),
            myDirty(false),
            myRequireExplicitClean(false)
        {
            memset(myDirtyCtx, 0, sizeof(myDirtyCtx));
        }
        virtual ~TextureSource() {}

        virtual Texture* getTexture(const DrawContext& context);
        virtual void attachTexture(Texture* tex, const DrawContext& context);

        virtual bool isDirty() { return myDirty; }
        virtual void setDirty(bool value = true);

        //! When enabled, the TextureSource object stays dirty even after all 
        //! the associated Textures have been updated, and will be marked as 
        //! clean only Through an explicit setDirty(false) call.
        void requireExplicitClean(bool value) { myRequireExplicitClean = value; }

        virtual int getHeight() { return 0; }
        virtual int getWidth() { return 0; }

    protected:
        virtual void refreshTexture(Texture* texture, const DrawContext& context) = 0;

    private:
        Ref<Texture> myTextures[GpuContext::MaxContexts];
        uint64_t myTextureUpdateFlags;
        bool myRequireExplicitClean;
        bool myDirtyCtx[GpuContext::MaxContexts];
        bool myDirty;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline int Texture::getWidth() 
    { 
        return myWidth; 
    }

    ///////////////////////////////////////////////////////////////////////////
    inline int Texture::getHeight() 
    { 
        return myHeight; 
    }

    ///////////////////////////////////////////////////////////////////////////
    inline Texture::TextureType Texture::getTextureType()
    {
        return myTextureType; 
    }

    ///////////////////////////////////////////////////////////////////////////
    inline Texture::ChannelType Texture::getChannelType()
    {
        return myChannelType;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline Texture::ChannelFormat Texture::getChannelFormat()
    {
        return myChannelFormat;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline GLuint Texture::getGLTexture() 
    {
        oassert(myInitialized);
        return myId; 
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Texture::isBound()
    {
        return (myTextureUnit != GpuContext::TextureUnitInvalid ? true : false);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline GpuContext::TextureUnit Texture::getTextureUnit()
    { return myTextureUnit; }
}; // namespace omega

#endif