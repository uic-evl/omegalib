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
 *	The omegalib renderer is the entry point for all of omegalib rendering code.
 *	The renderer does not draw anything: it just manages rendering resources, 
 *	cameras and render passes.
 ******************************************************************************/
#include "omega/Renderer.h"
#include "omega/Engine.h"
#include "omega/DisplaySystem.h"
#include "omega/Texture.h"
#include "omega/PythonInterpreter.h"
#include "omega/glheaders.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(Engine* engine)
{
	myRenderer = new DrawInterface();
	myServer = engine;
	myServer->addRenderer(this);
}

///////////////////////////////////////////////////////////////////////////////
Texture* Renderer::createTexture()
{
	Texture* tex = new Texture(this->myGpuContext);
	myResources.push_back(tex);
	return tex;
}

///////////////////////////////////////////////////////////////////////////////
RenderTarget* Renderer::createRenderTarget(RenderTarget::Type type)
{
	RenderTarget* rt = new RenderTarget(this->myGpuContext, type);
	myResources.push_back(rt);
	return rt;
}

///////////////////////////////////////////////////////////////////////////////
bool RenderPassSortOp(RenderPass* p1, RenderPass* r2)
{ return p1->getPriority() < r2->getPriority(); }

///////////////////////////////////////////////////////////////////////////////
void Renderer::addRenderPass(RenderPass* pass)
{
	myRenderPassLock.lock();
	ofmsg("Renderer(%1%): adding render pass %2%", %getGpuContext()->getId() %pass->getName());
	myRenderPassList.push_back(pass);
	// Re-sort the render pass list. Render passes implement a comparison 
	// operator to perform the sorting based on their priority.
	myRenderPassList.sort(RenderPassSortOp);
	myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::removeRenderPass(RenderPass* pass)
{
	myRenderPassLock.lock();
	pass->requestDispose();
	myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::removeAllRenderPasses()
{
	myRenderPassLock.lock();
	foreach(RenderPass* rp, myRenderPassList) rp->requestDispose();
	myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
RenderPass* Renderer::getRenderPass(const String& name)
{
	foreach(RenderPass* rp, myRenderPassList)
	{
		if(rp->getName() == name) return rp;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::initialize()
{
	ofmsg("@Renderer::Initialize: id = %1%", %getGpuContext()->getId());

	// Create the default font.
	const FontInfo& fi = myServer->getDefaultFont();
	if(fi.size != 0)
	{
		Font* fnt = myRenderer->createFont(fi.name, fi.filename, fi.size);
		myRenderer->setDefaultFont(fnt);
	}

	StatsManager* sm = getEngine()->getSystemManager()->getStatsManager();
	myFrameTimeStat = sm->createStat(ostr("ctx%1% frame", %getGpuContext()->getId()), StatsManager::Time);
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::queueCommand(IRendererCommand* cmd)
{
	myRenderCommandLock.lock();
	myRenderableCommands.push(cmd);
	myRenderCommandLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::startFrame(const FrameInfo& frame)
{
	myFrameTimeStat->startTiming();
	foreach(Ref<Camera> cam, myServer->getCameras())
	{
		cam->startFrame(frame);
	}
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::finishFrame(const FrameInfo& frame)
{
	foreach(Ref<Camera> cam, myServer->getCameras())
	{
		cam->finishFrame(frame);
	}

	bool shuttingDown = SystemManager::instance()->isExitRequested();

	// Dispose of unused resources. When shutting down, clean everything.
	List<GpuResource*> txlist;
	foreach(GpuResource* tex, myResources)
	{
		if(tex->refCount() == 1 || shuttingDown)
		{
			tex->dispose();
			txlist.push_back(tex);
		}
	}
	foreach(GpuResource* gr, txlist) myResources.remove(gr);
	myFrameTimeStat->stopTiming();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::clear(DrawContext& context)
{
    foreach(Ref<Camera> cam, myServer->getCameras())
    {
        cam->clear(context);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::draw(DrawContext& context)
{
	myRenderPassLock.lock();
	// First of all make sure all render passes are initialized.
	foreach(RenderPass* rp, myRenderPassList)
	{
		if(!rp->isInitialized()) rp->initialize();
	}
	// Now check if some render passes need to be disposed
	List<RenderPass*> tbdisposed;
	foreach(RenderPass* rp, myRenderPassList)
	{
		if(rp->needsDispose())
		{
			rp->dispose();
			tbdisposed.push_back(rp);
		}
	}
	foreach(RenderPass* rp, tbdisposed) myRenderPassList.remove(rp);
	myRenderPassLock.unlock();

	// Execute renderable commands.
	myRenderCommandLock.lock();
	while(!myRenderableCommands.empty())
	{
		myRenderableCommands.front()->execute(this);
		myRenderableCommands.pop();
	}
	myRenderCommandLock.unlock();

	foreach(Ref<Camera> cam, myServer->getCameras())
	{
		// See if camera is enabled for the current client and draw context.
		if(cam->isEnabledInContext(context))
		{
			// Begin drawing with the camera: get the camera draw context.
			cam->beginDraw(context);
			innerDraw(context, cam);
			cam->endDraw(context);
		}
	}

	// Draw once for the default camera (using the passed main draw context).
	// NOTE: We use the draw context returned by the camera because in principle
	// the camera may adjust the context before drawing. In practice, for the 
	// default camera the context should stay the same as what is passed to this
	// method.
	Camera* cam = myServer->getDefaultCamera();
	if(cam->isEnabledInContext(context))
	{
		cam->beginDraw(context);
		innerDraw(context, myServer->getDefaultCamera());
		cam->endDraw(context);
	}
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::innerDraw(const DrawContext& context, Camera* cam)
{
	// NOTE: Scene.draw traversal only runs for cameras that do not have a mask specified
	if(cam->getMask() == 0 && context.task == DrawContext::SceneDrawTask)
	{
		getRenderer()->beginDraw3D(context);

		// Run the draw method on scene nodes (was previously in DefaultRenderPass)
		// This will traverse the scene graph and invoke the draw method on all scene objects attached to nodes.
		// When stereo rendering, the traversal will happen once per eye.
		SceneNode* node = getEngine()->getScene();
		node->draw(context);

		// Draw 3d pointers.
		// We call drawPointers for scene draw tasks too because we may be drawing pointers in wand mode 
		//myServer->drawPointers(this, &state);

		getRenderer()->endDraw();
	}

	myRenderPassLock.lock();
	// Execute all render passes in order. 
	foreach(RenderPass* pass, myRenderPassList)
	{
		if(pass->isInitialized())
		{
			// Run the pass if both its mask and the camera mask are 0 (left unspecified)
			// Alternatively, run the pass if at least one of the mask bits is set on both the camera an the pass
			if((cam->getMask() == 0 && pass->getCameraMask() == 0) ||
				((cam->getMask() & pass->getCameraMask()) != 0))
			{
				pass->render(this, context);
			}
		}
	}
	myRenderPassLock.unlock();

	// Draw the pointers
	// NOTE: Pointer only run for cameras that do not have a mask specified
	if(cam->getMask() == 0 && context.task == DrawContext::OverlayDrawTask && 
		context.eye == DrawContext::EyeCyclop)
	{
		getRenderer()->beginDraw2D(context);

		// Let the interpreter handle scriptable draw callbacks.
		PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
		pi->draw(context, cam);
		
		myServer->drawPointers(this, context);
	
		getRenderer()->endDraw();
	}
}


