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
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/GpuResource.h"

namespace omega
{
	class PixelData;

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API Texture: public GpuResource
	{
	friend class Renderer;
	public:
		static void enablePboTransfers(bool value) { sUsePbo = value; }

	public:
		//! Initializes this texture object
		void initialize(int width, int height, uint format = 0); 
		bool isInitialized() { return myInitialized; }

		void writePixels(PixelData* data);
		void readPixels(PixelData* data);

		int getWidth();
		int getHeight();

		//! Texture operations
		//@{
		GLuint getGLTexture();
		void bind(GpuContext::TextureUnit unit);
		void unbind();
		void refresh();
		bool isBound();
		GpuContext::TextureUnit getTextureUnit();
		//@}

	protected:
		// Only renderer can allocate textures.
		Texture(GpuContext* context);

	private:
		static bool sUsePbo;

		bool myInitialized;
		GLuint myId;
		int myWidth;
		int myHeight;

		GLuint myPboId;

		GpuContext::TextureUnit myTextureUnit;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline int Texture::getWidth() 
	{ return myWidth; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline int Texture::getHeight() 
	{ return myHeight; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline GLuint Texture::getGLTexture() 
	{
		oassert(myInitialized);
		return myId; 
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline bool Texture::isBound()
	{
		return (myTextureUnit != GpuContext::TextureUnitInvalid ? true : false);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline GpuContext::TextureUnit Texture::getTextureUnit()
	{ return myTextureUnit; }
}; // namespace omega

#endif