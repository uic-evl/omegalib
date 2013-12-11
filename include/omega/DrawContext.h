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
 *	The DrawContext class: Contains information about the context in which 
 *  drawing operations take place
 ******************************************************************************/
#ifndef __DRAWCONTEXT_H__
#define __DRAWCONTEXT_H__

#include "osystem.h"
#include "DisplayConfig.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Contains information about the current frame.
	struct FrameInfo
	{
		FrameInfo(uint64 frame, GpuContext* context): frameNum(frame), gpuContext(context) {}
		uint64 frameNum;
		GpuContext* gpuContext;
	};

	///////////////////////////////////////////////////////////////////////////
	//! Contains information about the context in which drawing operations 
	//! take place. DrawContext is a fully self-contained description of 
	//! rendering operations that make up a full frame. 
	struct DrawContext
	{
		DrawContext();

		enum Eye { EyeLeft , EyeRight, EyeCyclop };
		enum Task { SceneDrawTask, OverlayDrawTask };
		uint64 frameNum; // TODO: Substitute with frameinfo
		AffineTransform3 modelview;
		Transform3 projection;
		//! The viewMin and viewMax are normalized coordinates of the view bounds
		//! on the current tile (that is, the size of the current rendered view on
		//! the current tile). These values are computed intersecting the tile
		//! position and size on the global canvas with the active camera view
		//! position and size. The view minimum and maximum bounds influence the
		//! frustum shape and pixel viewport.
		Vector2f viewMin;
		Vector2f viewMax;
		//! The pixel viewport coordinates of this context with respect to the 
		//! owner window of the context.
		Rect viewport;
		//! The eye being rendered for this context.
		Eye eye;
		//! The current draw task.
		Task task;
		//! Information about the drawing channel associated with this context.
		//ChannelInfo* channel;
		const DisplayTileConfig* tile;
		RenderTarget* drawBuffer;
		GpuContext* gpuContext;
		Renderer* renderer;
        //! The camera currently rendering this context.
        Camera* camera;

		//! Tile stack
		//! Lets cameras push/pop tiles, to support rendering with custom tile 
		//! definitions
		//@{
		Queue<const DisplayTileConfig*> tileStack;
		void pushTileConfig(DisplayTileConfig* newtile)
		{ tileStack.push(tile); tile = newtile; }
		void popTileConfig()
		{ tile = tileStack.front(); tileStack.pop(); }
		//@}

		//! The drawFrame method is the 'entry point' called by the display 
		//! system to render a full frame. drawFrame does all required setup
		//! operations (viewport, stereo mode etc), and calls the Renderer draw 
		//! method mltiple times to draw active eyes for the scene and overlay
		//! layers. The renderer draw method in turn renders secondary cameras
		//! and performs drawing with all the active render passes.
		void drawFrame(uint64 frameNum);

		//! Updates the pixel viewport of this context, based on the actual tile
		//! viewport, active eye and stereo settings.
		void updateViewport();
		void setupInterleaver();
		void initializeStencilInterleaver(int gliWindowWidth, int gliWindowHeight);
		DisplayTileConfig::StereoMode getCurrentStereoMode();
		// Clears the frame buffer.
		void clear();
		bool stencilInitialized;

		//! Updates the viewport based on the view size and position an the size
		//! of the overall canvas
		void updateViewBounds(
			const Vector2f& viewPos, 
			const Vector2f& viewSize, 
			const Vector2i& canvasSize);

		//! Updates the modelview and projection matrices based on head / view
		//! transform and eye separation. Crrent eye is read from context.
		void updateTransforms(
			const AffineTransform3& head, 
			const AffineTransform3& view, 
			float eyeSeparation,
			float nearZ,
			float farZ);

		//! Return true if this draw context is supposed to draw something for
		//! the specified view rectangle
		bool overlapsView(
			const Vector2f& viewPos, 
			const Vector2f& viewSize, 
			const Vector2i& canvasSize) const;
	};
}; // namespace omega

#endif