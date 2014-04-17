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
#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__

#include "osystem.h"
//#include "GpuManager.h"
#include "Texture.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Forward declarations
	class Texture;

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API RenderTarget: public GpuResource
	{
	friend class Renderer;
	public:
		enum Type {
			//! Render to the main framebuffer. Supports readback targets.
			RenderOnscreen, 
			//! Render to an offscreen buffer. Supports readback targets.
			RenderOffscreen, 
			//! Render to a texture. Supports texture and readback targets.
			RenderToTexture};

	public:
		//! Render target configuration
		//@{
		int getWidth();
		int getHeight();
		Type getType();
		void setTextureTarget(Texture* color, Texture* depth = NULL);
		void setReadbackTarget(PixelData* color, PixelData* depth = NULL);
		void setReadbackTarget(PixelData* color, PixelData* depth, const Rect& readbackViewport);
        void clearDepth(bool enabled);
        void clearColor(bool enabled);
        //@}

		//! Drawing
		//@{
		void bind();
		void unbind();
		bool isBound();
		void readback();
		void clear();
		//@}

		GLuint getId() { return myId; };
		virtual void dispose();

	protected:
		// Only renderer can allocate Render targets.
		RenderTarget(GpuContext* context, Type type, GLuint id = 0);
		~RenderTarget();

	private:
		GLuint myId;
		Type myType;
		bool myBound;

        bool myClearDepth;
        bool myClearColor;

		// Render buffer stuff
		GLuint myRbColorId;
		GLuint myRbDepthId;
		int myRbWidth;
		int myRbHeight;

		// Target stuff
		Texture* myTextureColorTarget;
		Texture* myTextureDepthTarget;

		PixelData* myReadbackColorTarget;
		PixelData* myReadbackDepthTarget;
		Rect myReadbackViewport;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline RenderTarget::Type RenderTarget::getType() 
	{ return myType; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline bool RenderTarget::isBound()
	{ return myBound; }
}; // namespace omega

#endif