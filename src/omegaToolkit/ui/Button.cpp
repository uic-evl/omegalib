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
 * What's in this file:
 *	A standard button widget
 ******************************************************************************/
#include "omegaToolkit/ui/Button.h"
#include "omegaToolkit/UiModule.h"
//#include "omegaToolkit/ui/DefaultSkin.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

NameGenerator sButtonNameGenerator("Button");

///////////////////////////////////////////////////////////////////////////////
Button* Button::create(Container* container)
{
	WidgetFactory* wf = UiModule::instance()->getWidgetFactory();
	Button* btn = wf->createButton(sButtonNameGenerator.generate(), container);
	return btn;
}

///////////////////////////////////////////////////////////////////////////////
Button::Button(Engine* srv):
	AbstractButton(srv),
	myLabel(srv),
	myImage(srv)
{
	myLabel.ref();
	myImage.ref();
	//addChild(&myLabel);
	//myLabel.setText(name);
	setMaximumHeight(22);
	myColor = Color(0.2f, 0.2f, 0.2f);
	myImageEnabled = false;
    myTextEnabled = true;
	setAutosize(true);
	//setDebugModeEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
Button::~Button()
{

}

///////////////////////////////////////////////////////////////////////////////
void Button::autosize()
{
	myLabel.autosize();
	myImage.autosize();

    // HACK: we add some default margin to the label.
    Vector2f size = Vector2f::Zero();
    if(myTextEnabled)
    {
        size = myLabel.getSize();
    }

	size[0] += myImage.getSize()[0];
	size[1] = max(size[1], myImage.getSize()[1]);
	myLabel.setHeight(size[1]);

	// Commented: avoid stretching the image.
	//myImage.setHeight(size[1]);
	if(myCheckable)
	{
		size += Vector2f(size[1] + 10, 4);
	}
	else
	{
		size += Vector2f(10, 4);
	}
	if(isImageEnabled())
	{
		size.x() += myImage.getSize().x();
	}

	setSize(size);
}

///////////////////////////////////////////////////////////////////////////////
void Button::update(const omega::UpdateContext& context)
{
	AbstractButton::update(context);
	myLabel.update(context);
	myImage.update(context);
}

///////////////////////////////////////////////////////////////////////////////
void Button::handleEvent(const Event& evt)
{
	if(isPointerInteractionEnabled())
	{
		if(evt.getServiceType() == Event::ServiceTypePointer || evt.getServiceType() == Event::ServiceTypeWand)
		{
			Vector2f point  = Vector2f(evt.getPosition().x(), evt.getPosition().y());
			point = transformPoint(point);
			if(simpleHitTest(point))
			{
				if(evt.isButtonDown(UiModule::getClickButton()))		
				{
					myPressed = true;
					myPressedStateChanged = true;
					evt.setProcessed();
					playPressedSound();
				}
				else if(myPressed && evt.getType() == Event::Up) // Need to check buton up like this because mouse service takes away button flag on up button events.
				{
					myPressed = false;
					myPressedStateChanged = true;
					evt.setProcessed();
				}
			}
		}
	}
	if(!evt.isProcessed() && isGamepadInteractionEnabled())
	{
		if(isActive())
		{
			if(evt.isButtonDown(UiModule::getConfirmButton()))
			{
				myPressed = false;
				myPressedStateChanged = true;
				playPressedSound();
			}
		}
	}
	AbstractButton::handleEvent(evt);
}

///////////////////////////////////////////////////////////////////////////////
void Button::playPressedSound()
{
	if(getEngine()->getSoundEnvironment() != NULL)
	{
		if(SystemManager::settingExists("config/sound"))
		{
			Sound* sound = getEngine()->getSoundEnvironment()->getSound("selectMenuSound");
			if( sound != NULL )
			{
				SoundInstance* inst = new SoundInstance(sound);
				inst->setLocalPosition( getContainer()->get3dSettings().position );
				inst->play();
			}
		}
	}
}
