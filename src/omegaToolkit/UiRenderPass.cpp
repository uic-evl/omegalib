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
#include "omegaToolkit/UiRenderPass.h"
#include "omega/Renderer.h"
#include "omega/Engine.h"
#include "omega/DrawInterface.h"
#include "omega/SceneNode.h"
#include "omegaToolkit/ui/Container.h"
#include "omegaToolkit/UiModule.h"
#include "omega/DisplaySystem.h"
#include "omega/glheaders.h"

using namespace omega;
using namespace omegaToolkit;

Lock sLock;

///////////////////////////////////////////////////////////////////////////////////////////////////
RenderPass* UiRenderPass::createInstance(Renderer* client) 
{ 
	return new UiRenderPass(client, "UiRenderPass"); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
UiRenderPass::UiRenderPass(Renderer* client, const String& name): 
	RenderPass(client, name, 10), 
	myUiRoot(NULL) 
{
	StatsManager* sm = getClient()->getEngine()->getSystemManager()->getStatsManager();
	myDrawTimeStat = sm->createStat("ui draw", Stat::Time);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiRenderPass::render(Renderer* client, const DrawContext& context)
{
	sLock.lock();
	myDrawTimeStat->startTiming();

	if(context.task == DrawContext::SceneDrawTask)
	{
		client->getRenderer()->beginDraw3D(context);
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// This is a bit of a hack. DIsable depth testing for ui stuff. We will take care of ordering.
		// This may lead to depth inconsistencies wrt the background scene when drawing 3d menus, but we want te
		// menus to always be visible and unoccluded by geometry.
		glDisable(GL_DEPTH_TEST);

		ui::Container* ui = myUiRoot;
		Renderable* uiRenderable = ui->getRenderable(client);
		if(uiRenderable != NULL)
		{
			uiRenderable->draw(context);
		}

		glPopAttrib();
		client->getRenderer()->endDraw();
	}
	else if(context.task == DrawContext::OverlayDrawTask)
	{
		Vector2i displaySize;
		// check if the tile is part of a canvas (a multi-tile grid). If it is,
		// get the canvas resolution. Otherwise simply use the tile resolution.
		if(context.tile->isInGrid)
		{
			DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
			displaySize = ds->getCanvasSize();
		}
		else
		{
			displaySize = context.tile->pixelSize;
		}

		client->getRenderer()->beginDraw2D(context);
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		ui::Container* ui = myUiRoot;
		const Rect& vp = Rect(0, 0, displaySize[0], displaySize[1]);

		// Update the root container size if necessary.
		if((ui->getPosition().cwiseNotEqual(vp.min.cast<omicron::real>())).all() ||
			ui->getSize().cwiseNotEqual(vp.max.cast<omicron::real>()).all())
		{
			ui->setPosition(vp.min.cast<omicron::real>());
			ui->setSize(Vector2f(vp.width(), vp.height()));
			/*ofmsg("ui viewport update: position = %1% size = %2% %3%",
				%vp.min %vp.width() %vp.height());*/
		}

		// Make sure all widget sizes are up to date (and perform autosize where necessary).
		ui->updateSize(client);

		// Layout ui.
		ui->layout();

		Renderable* uiRenderable = ui->getRenderable(client);
		if(uiRenderable != NULL)
		{
			uiRenderable->draw(context);
		}

		glPopAttrib();
		client->getRenderer()->endDraw();
	}

	myDrawTimeStat->stopTiming();
	sLock.unlock();
}
