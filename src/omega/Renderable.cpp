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
#include "omega/Renderable.h"
#include "omega/Engine.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderableCommand::execute(Renderer* r)
{
	switch(command)
	{
	case Initialize: renderable->initialize(); break;
	case Dispose: renderable->dispose(); renderable = NULL; break;
	case Refresh: renderable->refresh(); break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Renderable::Renderable():
	myClient(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Renderable::~Renderable()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::postDisposeCommand()
{
	myClient->queueCommand(new RenderableCommand(this, RenderableCommand::Dispose));
}

///////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::postInitializeCommand()
{
	myClient->queueCommand(new RenderableCommand(this, RenderableCommand::Initialize));
}

///////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::postRefreshCommand()
{
	myClient->queueCommand(new RenderableCommand(this, RenderableCommand::Refresh));
}

///////////////////////////////////////////////////////////////////////////////////////////////
DrawInterface* Renderable::getRenderer()
{ 
	return myClient->getRenderer(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RenderableFactory::RenderableFactory():
	myInitialized(false),
	myServer(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RenderableFactory::~RenderableFactory()
{
	dispose();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Renderable* RenderableFactory::addRenderable(Renderer* cli)
{
	Renderable* r = createRenderable();
	r->setClient(cli);
	myRenderables.push_back(r);
	r->postInitializeCommand();
	return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderableFactory::initialize(Engine* srv)
{
	if(!myInitialized)
	{
		//ofmsg("Initializing renderable factory: %1%", %toString());
		myServer = srv;
		foreach(Renderer* client, srv->getRendererList())
		{
			addRenderable(client);
		}
		myInitialized = true;
	}
	else
	{
		owarn("!RenderableFactory::initialize - renderable already initialized");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderableFactory::dispose()
{
	if(myInitialized || myRenderables.size() > 0)
	{
		//ofmsg("Disposing renderable factory: %1%", %toString());
		foreach(Renderable* r, myRenderables)
		{
			r->postDisposeCommand();
		}
		myRenderables.empty();
		myInitialized = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderableFactory::refresh()
{
	foreach(Renderable* r, myRenderables)
	{
		r->postRefreshCommand();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Renderable* RenderableFactory::getRenderable(Renderer* client)
{
	foreach(Renderable* r, myRenderables)
	{
		if(r->getClient() == client) return r;
	}
	// A renderable has not ben created yet for the specified client. do it now.
	return addRenderable(client);
}

