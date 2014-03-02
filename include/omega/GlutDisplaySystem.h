/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
#ifndef __GLUT_DISPLAY_SYSTEM_H__
#define __GLUT_DISPLAY_SYSTEM_H__

#include "DisplaySystem.h"
#include "RenderTarget.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! Implements a display system based on GLUT, offering a single render window and mouse input support.
	//! Note: this display system is not well maintained. It may not work / work partially. Developers are advised to
	//! use EqualizerDisplaySystem in most circumstances.
	class OMEGA_API GlutDisplaySystem: public DisplaySystem
	{
	public:
		GlutDisplaySystem();
		virtual ~GlutDisplaySystem();

		// sets up the display system. Called before initalize.
		void setup(Setting& setting);

		virtual void initialize(SystemManager* sys); 
		virtual void run(); 
		virtual void cleanup(); 

		void updateProjectionMatrix();

		DisplaySystemType getId() { return DisplaySystem::Glut; }

		Engine* getApplicationServer() { return myAppServer; }
		Renderer* getApplicationClient() { return myAppClient; }

		virtual Vector2i getCanvasSize() { return myResolution; }
		virtual Ray getViewRay(Vector2i position) { return Ray(); }

		RenderTarget* getFrameBuffer() { return myFrameBuffer; }
		GpuContext* getGpuContext() { return myGpuContext; }

	private:
		// Display config
		Setting* mySetting;
		Vector2i myResolution;
		int myFov;
		float myAspect;
		double myNearz;
		double myFarz;

		SystemManager* mySys;
		Renderer* myAppClient;
		Engine* myAppServer;
		RenderTarget* myFrameBuffer;

		GpuContext* myGpuContext;
	};
}; // namespace omega

#endif