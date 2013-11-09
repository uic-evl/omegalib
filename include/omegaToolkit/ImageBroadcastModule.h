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
#ifndef __IMAGE_BROADCAST_MODULE__
#define __IMAGE_BROADCAST_MODULE__

#include "omegaToolkitConfig.h"
#include "omega/ImageUtils.h"
#include "omega/ModuleServices.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////
	class OTK_API ImageBroadcastModule: public EngineModule
	{
	public:
		static ImageBroadcastModule* instance();

		ImageBroadcastModule();
		~ImageBroadcastModule();

		void addChannel(PixelData* channel, const String& channelName, ImageUtils::ImageFormat format = ImageUtils::FormatJpeg, int quality = 100);
		// Remove a publisher or subscriber channel with the specified name
		void removeChannel(const String& channel);

		virtual void commitSharedData(SharedOStream& out);
		virtual void updateSharedData(SharedIStream& in);

	private:
		// Stores information about a publish/subscribe image channel.
		class Channel: public ReferenceType
		{
		public:
			Channel():
				encoding(ImageUtils::FormatJpeg),
				quality(100)
				{}
			
			String name;
			Ref<PixelData> data;
			ImageUtils::ImageFormat encoding;
			int quality;
		};

	private:
		static ImageBroadcastModule* mysInstance;

		typedef Dictionary<String, Ref<Channel> > ChannelDictionary;

		ChannelDictionary myChannels;
	};
}; // namespace omega

#endif