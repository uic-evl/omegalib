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
#ifndef __RENDERABLE_H__
#define __RENDERABLE_H__

#include "osystem.h"
#include "RenderPass.h"
#include "IRendererCommand.h"
#include "Renderer.h"

namespace omega {
	class Engine;
	class Renderable;

	///////////////////////////////////////////////////////////////////////////////////////////////
	struct OMEGA_API RenderableCommand: public IRendererCommand
	{
		enum Command { Initialize, Dispose, Refresh };
		Ref<Renderable> renderable;
		Command command;

		RenderableCommand(Renderable* r, Command c): renderable(r), command(c) {}
		virtual void execute(Renderer* r);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API Renderable: public ReferenceType
	{
	public:
		Renderable();
		virtual ~Renderable();

		void setClient(Renderer* client);
		Renderer* getClient();
		DrawInterface* getRenderer();

		void postDisposeCommand();
		void postInitializeCommand();
		void postRefreshCommand();

		virtual void initialize() {}
		virtual void dispose() {}
		virtual void refresh() {}
		virtual void draw(const DrawContext& context) = 0;

	private:
		Ref<Renderer> myClient;

		//Ref<RenderableCommand> myDisposeCommand;
		//Ref<RenderableCommand> myRefreshCommand;
		//Ref<RenderableCommand> myInitializeCommand;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Base class for objects that can create renderables.
	class OMEGA_API RenderableFactory: public ReferenceType
	{
	public:
		RenderableFactory();
		virtual ~RenderableFactory();
		virtual Renderable* createRenderable() = 0;
		virtual void initialize(Engine* srv);
		void dispose();
		void refresh();
		virtual bool isInitialized();
		Renderable* getRenderable(Renderer* client);
		Engine* getEngine();

	protected:
		Renderable* addRenderable(Renderer* cli);

	private:
		bool myInitialized;
		Engine* myServer;
		List< Ref<Renderable> > myRenderables;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline void Renderable::setClient(Renderer* value)
	{ myClient = value; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline Renderer* Renderable::getClient()
	{ return myClient; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline bool RenderableFactory::isInitialized()
	{ return myInitialized; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	inline Engine* RenderableFactory::getEngine()
	{ return myServer; }

}; // namespace omega

#endif