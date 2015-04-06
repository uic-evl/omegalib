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
#include "omega/glheaders.h"

using namespace omega;

bool Texture::sUsePbo = false;

///////////////////////////////////////////////////////////////////////////////
Texture::Texture(GpuContext* context): 
    GpuResource(context),
    myInitialized(false),
    myTextureUnit(GpuContext::TextureUnitInvalid) 
{}

///////////////////////////////////////////////////////////////////////////////
void Texture::initialize(int width, int height, TextureType tt, ChannelType ct, ChannelFormat cf)
{
    myWidth = width;
    myHeight = height;

    myGlFormat = GL_RGBA;
    switch(ct)
    {
    case ChannelRGB:
        myGlFormat = GL_RGB;
        break;
    case ChannelRGBA:
        myGlFormat = GL_RGBA;
        break;
    case ChannelDepth:
        myGlFormat = GL_DEPTH_COMPONENT;
        break;
    }
    myChannelType = ct;

    uint textureType;
    switch(tt)
    {
    case Type2D:
        textureType = GL_TEXTURE_2D;
        break;
    case TypeRectangle:
        textureType = GL_TEXTURE_RECTANGLE;
        break;
    }
    myTextureType = tt;

    uint channelFormat;
    switch(cf)
    {
    case FormatFloat:
        channelFormat = GL_FLOAT;
        break;
    case FormatUInt:
        channelFormat = GL_UNSIGNED_INT;
        break;
    case FormatUByte:
        channelFormat = GL_UNSIGNED_BYTE;
        break;
    }
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

        glBindTexture(GL_TEXTURE_2D, myId);
        int xoffset = 0;
        int yoffset = 0;
        // If needed, resize the texture.
        if(h != myHeight || w != myWidth)
        {
            myHeight = h;
            myWidth = w;
            glTexImage2D(GL_TEXTURE_2D, 0, myGlFormat, myWidth, myHeight, 0, myGlFormat, GL_UNSIGNED_BYTE, NULL);
        }

        if(format == GL_RGB)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, w, h, format, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
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
    glBindTexture(GL_TEXTURE_2D, myId);
}

///////////////////////////////////////////////////////////////////////////////
void Texture::unbind()
{
    myTextureUnit = GpuContext::TextureUnitInvalid;
}
