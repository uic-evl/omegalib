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
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/DefaultSkin.h"
#include "omegaToolkit/ui/Image.h"
#include "omegaToolkit/UiRenderPass.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit;
using namespace omega;

UiModule* UiModule::mysInstance = NULL;
Event::Flags UiModule::mysConfirmButton = Event::Button3;
Event::Flags UiModule::mysCancelButton = Event::Button4;
Event::Flags UiModule::mysClickButton = Event::Button4;
	
///////////////////////////////////////////////////////////////////////////////////////////////////
UiModule* UiModule::createAndInitialize()
{
	if(mysInstance == NULL)
	{
		mysInstance = new UiModule();
		ModuleServices::addModule(mysInstance);
		mysInstance->doInitialize(Engine::instance());
	}
	return mysInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
UiModule::UiModule():
	EngineModule("UiModule"),
	myWidgetFactory(NULL),
	myPointerInteractionEnabled(true),
	myGamepadInteractionEnabled(false),
	myActiveWidget(NULL),
	myCullingEnabled(true)
{
	mysInstance = this;
	// This module has high priority. It will receive events before modules with lower priority.
	setPriority(EngineModule::PriorityHigh);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::initialize()
{
	omsg("UiModule initializing...");

	myWidgetFactory = new ui::DefaultWidgetFactory(getEngine());
	myUi = new ui::Container(getEngine());
	myUi->setAutosize(false);
	myUi->setLayout(ui::Container::LayoutFree);
	myUi->setBlendMode(ui::Widget::BlendNormal);
	
	// If we let the engine act as the default event handler we end up having infinite loops.
	// The engine shouldn't really be acting as a default handler. Only user code should.
	//myUi->setUIEventHandler(getEngine());

    Config* cfg = getEngine()->getSystemManager()->getAppConfig();

	//myLocalEventsEnabled = cfg->getBoolValue("config/ui/enableLocalEvents", true);
	myLocalEventsEnabled = true;

	//if(cfg->exists("config/ui/images"))
	//{
	//	const Setting& stImages = cfg->lookup("config/ui/images");
	//	initImages(stImages);
	//}

	if(SystemManager::settingExists("config/ui"))
	{
		Setting& sUi = SystemManager::settingLookup("config/ui");
		mysConfirmButton = Event::parseButtonName(Config::getStringValue("confirmButton", sUi, "Button3"));
		mysCancelButton = Event::parseButtonName(Config::getStringValue("cancelButton", sUi, "Button4"));
		mysClickButton = Event::parseButtonName(Config::getStringValue("clickButton", sUi, "Button1"));
		myGamepadInteractionEnabled = Config::getBoolValue("gamepadInteractionEnabled", sUi, myGamepadInteractionEnabled);
		myPointerInteractionEnabled = Config::getBoolValue("pointerInteractionEnabled", sUi, myPointerInteractionEnabled);
	}

	omsg("UiModule initialization OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::dispose()
{
	omsg("UiModule::dispose");

	getEngine()->removeRenderPass("UiRenderPass");

	myActiveWidget = NULL;
	myWidgetFactory = NULL;
	myUi = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
UiModule::~UiModule()
{
	omsg("~UiModule");
	mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::initializeRenderer(Renderer* r)
{
	UiRenderPass* uirp = new UiRenderPass(r, "UiRenderPass");
	r->addRenderPass(uirp);
	uirp->setUiRoot(myUi);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::update(const UpdateContext& context)
{
	myUi->update(context);

    Vector2i displaySize = SystemManager::instance()->getDisplaySystem()->getCanvasSize();

	const Rect& vp = Rect(0, 0, displaySize[0], displaySize[1]);

	// Update the root container size if necessary.
	if((myUi->getPosition().cwiseNotEqual(vp.min.cast<omicron::real>())).all() ||
		myUi->getSize().cwiseNotEqual(vp.max.cast<omicron::real>()).all())
	{
		myUi->setPosition(vp.min.cast<omicron::real>());
		myUi->setSize(Vector2f(vp.width(), vp.height()));
		/*ofmsg("ui viewport update: position = %1% size = %2% %3%",
			%vp.min %vp.width() %vp.height());*/
	}

	// Make sure all widget sizes are up to date (and perform autosize where necessary).
	myUi->updateSize();

	// Layout ui.
	myUi->layout();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::handleEvent(const Event& evt)
{
	// If we have an active widget, it always gets the first chance of processing the event.
	if(myActiveWidget != NULL)
	{
		myActiveWidget->handleEvent(evt);
	}
	if(!evt.isProcessed())
	{
		myUi->handleEvent(evt);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
ui::Container* UiModule::getUi()
{
	return myUi;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::activateWidget(ui::Widget* w)
{
	if(myActiveWidget != NULL)
	{
		//ofmsg("Deactivating widget %1% (%2%)", %myActiveWidget->getId() %myActiveWidget->getName());
		myActiveWidget->setActive(false);
	}
	myActiveWidget = w;
	if(myActiveWidget != NULL)
	{
		//ofmsg("Activating widget %1% (%2%)", %w->getId() %w->getName());
		myActiveWidget->setActive(true);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
ui::Container* UiModule::createExtendedUi(const String& name, uint mask, int rendererId)
{
	Ref<ExtendedUiData> euid = new ExtendedUiData();

	euid->container = new ui::Container(getEngine());
	euid->container->setAutosize(false);
	euid->container->setLayout(ui::Container::LayoutFree);
	euid->container->setName(name);

	euid->renderer = getEngine()->getRendererByContextId(rendererId);
	if(euid->renderer == NULL)
	{
		ofwarn("UiModule::createExtendedUi: could not find renderer context id %1%", %rendererId);
		return NULL;
	}

	euid->renderPass = new UiRenderPass(euid->renderer, name);
	euid->renderPass->setCameraMask(mask);
	euid->renderer->addRenderPass(euid->renderPass);
	euid->renderPass->setUiRoot(euid->container);

	myExtendedUiList.push_back(euid);

	return euid->container;
}

///////////////////////////////////////////////////////////////////////////////////////////////
ui::Container* UiModule::getExtendedUi(const String& name)
{
	foreach(ExtendedUiData* euid, myExtendedUiList)
	{
		if(euid->container->getName() == name) return euid->container;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void UiModule::destroyExtendedUi(const String& name)
{
	ExtendedUiData* todelete = NULL;

	foreach(ExtendedUiData* euid, myExtendedUiList)
	{
		if(euid->container->getName() == name) todelete = euid;
	}

	// Found extended ui. delete it.
	if(todelete != NULL)
	{
		todelete->renderer->removeRenderPass(todelete->renderPass);
		myExtendedUiList.remove(todelete);
	}
}
