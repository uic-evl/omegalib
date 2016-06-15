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
#include "omega/Texture.h"
#include "omega/PixelData.h"
#include "omega/DrawContext.h"
#include "omega/Renderer.h"
#include "omega/glheaders.h"

using namespace omega;

bool Texture::sUsePbo = false;

///////////////////////////////////////////////////////////////////////////////
uint glTextureType(Texture::TextureType tt)
{
    switch(tt)
    {
        case Texture::Type2D: return GL_TEXTURE_2D;
        case Texture::TypeRectangle: return GL_TEXTURE_RECTANGLE;
    }
    return GL_TEXTURE_2D;
}

///////////////////////////////////////////////////////////////////////////////
uint glChannelType(Texture::ChannelType ct)
{
    switch(ct)
    {
        case Texture::ChannelRGB: return GL_RGB;
        case Texture::ChannelRGBA: return GL_RGBA;
        case Texture::ChannelDepth: return GL_DEPTH_COMPONENT;
    }
    return GL_RGBA;
}

///////////////////////////////////////////////////////////////////////////////
uint glChannelFormat(Texture::ChannelFormat cf)
{
    switch(cf)
    {
        case Texture::FormatFloat: return GL_FLOAT;
        case Texture::FormatUInt: return GL_UNSIGNED_INT;
        case Texture::FormatUByte: return GL_UNSIGNED_BYTE;
    }
    return GL_UNSIGNED_BYTE;
}

///////////////////////////////////////////////////////////////////////////////
Texture::Texture(GpuContext* context): 
    GpuResource(context),
    myInitialized(false),
    myTextureUnit(GpuContext::TextureUnitInvalid),
    myId(0)
{}

///////////////////////////////////////////////////////////////////////////////
void Texture::initialize(int width, int height, TextureType tt, ChannelType ct, ChannelFormat cf)
{
    myWidth = width;
    myHeight = height;

    myGlFormat = glChannelType(ct);
    myChannelType = ct;

    uint textureType = glTextureType(tt);
    myTextureType = tt;

    uint channelFormat = glChannelFormat(cf);
    myChannelFormat = cf;

    //Now generate the OpenGL texture object 
    glGenTextures(1, &myId);
    glBindTexture(textureType, myId);
    glTexImage2D(textureType, 0, myGlFormat, myWidth, myHeight, 0, myGlFormat, channelFormat, NULL);

    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(sUsePbo)
    {
        glGenBuffers(1, &myPboId);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, myPboId);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 4, 0, GL_STREAM_DRAW);
    }

    GLenum glErr = glGetError();

    if(glErr)
    {
        const unsigned char* str = gluErrorString(glErr);
        oferror("Texture::initialize: %1%", %str);
        return;
    }

    myInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////
void Texture::dispose()
{
    if(myId != 0)
    {
        oflog(Verbose, "[Texture::dispose] id=<%1%>", %myId);
        
        glDeleteTextures(1, &myId);
        myId = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
void Texture::resize(int w, int h)
{
    oassert(myId != 0);
    oflog(Verbose, "[Texture::resize] id=<%1%> from=<%2%x%3%> to=<%4%x%5%>", 
        %myId %myWidth %myHeight %w %h);

    myHeight = h;
    myWidth = w;
    glBindTexture(glTextureType(myTextureType), myId);
    glTexImage2D(glTextureType(myTextureType), 
        0, myGlFormat, 
        myWidth, myHeight, 0, 
        myGlFormat, 
        glChannelFormat(myChannelFormat), NULL);
    if(oglError) return;
}

///////////////////////////////////////////////////////////////////////////////
void Texture::writePixels(PixelData* data)
{
    if(data != NULL)
    {
        int h = data->getHeight();
        int w = data->getWidth();

        GLenum format = GL_RGBA;
        if(data->getFormat() == PixelData::FormatRgb) format = GL_RGB;

        byte* pixels = data->bind(getContext());
        writeRawPixels(pixels, w, h, format);
        data->unbind();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Texture::writeRawPixels(const byte* pixels, int w, int h, uint format)
{
    if(myInitialized && pixels != NULL)
    {
        if(sUsePbo)
        {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, myPboId);
        }

        glBindTexture(glTextureType(myTextureType), myId);
        int xoffset = 0;
        int yoffset = 0;
        // If needed, resize the texture.
        if(h != myHeight || w != myWidth)
        {
            myHeight = h;
            myWidth = w;
            glTexImage2D(glTextureType(myTextureType), 
                0, myGlFormat, 
                myWidth, myHeight, 0, 
                myGlFormat, glChannelFormat(myChannelFormat), NULL);
        }

        if(format == GL_RGB)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }

        glTexSubImage2D(glTextureType(myTextureType), 
            0, xoffset, yoffset, w, h, 
            format, 
            glChannelFormat(myChannelFormat), (GLvoid*)pixels);
        GLenum glErr = glGetError();

        if(glErr)
        {
            const unsigned char* str = gluErrorString(glErr);
            oferror("Texture::writePixels: %1%", %str);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Texture::bind(GpuContext::TextureUnit unit)
{
    myTextureUnit = unit;
    glActiveTexture(myTextureUnit);
    glBindTexture(glTextureType(myTextureType), myId);
}

///////////////////////////////////////////////////////////////////////////////
void Texture::unbind()
{
    myTextureUnit = GpuContext::TextureUnitInvalid;
}

///////////////////////////////////////////////////////////////////////////////
Texture* TextureSource::getTexture(const DrawContext& context)
{
    uint id = context.gpuContext->getId();
    if(myTextures[id].isNull())
    {
        myTextures[id] = context.renderer->createTexture();
        myTextureUpdateFlags |= 1ull << id;
    }

    // See if the texture needs refreshing
    if(myDirtyCtx[id] && (myTextureUpdateFlags & (1ull << id)))	{
        refreshTexture(myTextures[id], context);
        myTextureUpdateFlags &= ~(1ull << id);

        // If no other texture needs refreshing, reset the dirty flag
        if(!myRequireExplicitClean) myDirtyCtx[id] = false;
    }

    return myTextures[id];
}

///////////////////////////////////////////////////////////////////////////////
void TextureSource::attachTexture(Texture* tex, const DrawContext& context)
{
    uint id = context.gpuContext->getId();
    // If a texture already exists for this context it will be deattached and will not be refreshed
    // by this object anymore. Texture ref counting should take care of deletion when needed.
    myTextures[id] = tex;
    // always refresh the texture
    refreshTexture(myTextures[id], context);
    // Make sure the refresh flag for this texture is reset.
    myTextureUpdateFlags &= ~(1ull << id);
}

///////////////////////////////////////////////////////////////////////////////
void TextureSource::setDirty(bool value)
{
    myDirty = value;
    for(int i = 0; i < GpuContext::MaxContexts; i++)
        myDirtyCtx[i] = value;

    if(value)
    {
        // mark textures as needing update
        for(int i = 0; i < GpuContext::MaxContexts; i++)
        {
            // if the ith texture exists, set the ith bit in the update mask.
            if(!myTextures[i].isNull()) myTextureUpdateFlags |= 1ull << i;
        }
    }
}
