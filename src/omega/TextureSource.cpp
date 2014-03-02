/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#include "omega/TextureSource.h"
#include "omega/ApplicationBase.h"
#include "omega/Renderer.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////
Texture* TextureSource::getTexture(const DrawContext& context)
{
	uint id = context.gpuContext->getId();
	if(myTextures[id].isNull())
	{
		myTextures[id] = context.renderer->createTexture();
		myTextureUpdateFlags |= 1 << id;
	}

	// See if the texture needs refreshing
	if(myDirty && (myTextureUpdateFlags & (1 << id)))
	{
		refreshTexture(myTextures[id], context);
		myTextureUpdateFlags &= ~(1 << id);

		// If no other texture needs refreshing, reset the dirty flag
		if(!myTextureUpdateFlags) myDirty = false;
	}

	return myTextures[id];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TextureSource::attachTexture(Texture* tex, const DrawContext& context)
{
	uint id = context.gpuContext->getId();
	// If a texture already exists for this context it will be deattached and will not be refreshed
	// by this object anymore. Texture ref counting should take care of deletion when needed.
	myTextures[id] = tex;
	// always refresh the texture
	refreshTexture(myTextures[id], context);
	// Make sure the refresh flag for this texture is reset.
	myTextureUpdateFlags &= ~(1 << id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TextureSource::setDirty(bool value)
{
	myDirty = value;
	if(myDirty)
	{
		// mark textures as needing update
		for(int i = 0; i < GpuContext::MaxContexts; i++)
		{
			// if the ith texture exists, set the ith bit in the update mask.
			if(!myTextures[i].isNull()) myTextureUpdateFlags |= 1 << i;
		}
	}
}
