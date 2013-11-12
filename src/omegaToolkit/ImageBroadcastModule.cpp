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
 *	A module to share pixel data between master and slave nodes
 ******************************************************************************/
#include "omegaToolkit/ImageBroadcastModule.h"

using namespace omega;

ImageBroadcastModule* ImageBroadcastModule::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageBroadcastModule* ImageBroadcastModule::instance()
{
    if(mysInstance == NULL)
    {
        mysInstance = new ImageBroadcastModule();
        ModuleServices::addModule(mysInstance);
        mysInstance->doInitialize(Engine::instance());
    }
    return mysInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageBroadcastModule::ImageBroadcastModule():
    EngineModule("ImageBroadcastModule")
{
    enableSharedData();
    mysInstance = this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageBroadcastModule::~ImageBroadcastModule()
{
    mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageBroadcastModule::addChannel(PixelData* channel, const String& channelName, ImageUtils::ImageFormat format, int quality)
{
    Channel* ch = new Channel();
    ch->name = channelName;
    ch->data = channel;
    ch->encoding = format;
    ch->quality = quality;
    myChannels[channelName] = ch;

    // We will be in charge of marking the pixel data as clean.
    channel->requireExplicitClean(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageBroadcastModule::removeChannel(const String& channel)
{
    myChannels.erase(channel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageBroadcastModule::commitSharedData(SharedOStream& out)
{
    int numChannels = 0;
    foreach(ChannelDictionary::Item ch, myChannels)
    {
        // Count the number of channels that need sending.
        // NOTE: The PixelData dirty flag is also used to refresh textures
        // attached to the pixel data: we need to use and reset it here.
        // It is possible that textures attached to a PixelData object used as
        // an image broadcast object will not work correctly. 
        if(ch->data->isDirty()) numChannels++;
    }

    out << numChannels;

    foreach(ChannelDictionary::Item ch, myChannels)
    {
        // If the channel pixel data is marked as dirty, encode and send it.
        if(ch->data->isDirty())
        {
            Ref<ByteArray> data = ImageUtils::encode(ch->data, ch->encoding);
            out << ch->name;
            out << ch->encoding;
            out << data->getSize();
            out.write(data->getData(), data->getSize());
            ch->data->setDirty(false);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageBroadcastModule::updateSharedData(SharedIStream& in)
{
    int numChannels = 0;
    in >> numChannels;

    for(int i = 0; i < numChannels; i++)
    {
        String name;
        in >> name;
        Channel* ch = myChannels[name];
        if(ch != NULL)
        {
            ImageUtils::ImageFormat fmt;
            size_t size;
            in >> fmt;
            in >> size;
            oassert(fmt == ch->encoding);
            ByteArray a(size);
            in.read(a.getData(), size);
            
            Ref<PixelData> pixels = ImageUtils::decode(a.getData(), a.getSize());
            ch->data->copyFrom(pixels);
        }
        else
        {
            ofmsg("ImageBroadcastModule::updateSharedData: cannot find channel %1%", %name);
        }
    }
}
