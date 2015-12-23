/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory,  
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
 *	A slider widget
 ******************************************************************************/
#include "omegaToolkit/ui/Slider.h"
#include "omegaToolkit/UiModule.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

NameGenerator sSliderNameGenerator("Slider");

///////////////////////////////////////////////////////////////////////////////
Slider* Slider::create(Container* container)
{
	WidgetFactory* wf = UiModule::instance()->getWidgetFactory();
	Slider* slider = wf->createSlider(sSliderNameGenerator.generate(), container);
	return slider;
}

///////////////////////////////////////////////////////////////////////////////
Slider::Slider(Engine* srv):
	Widget(srv),
	myTicks(100),
	myValue(0),
	myDeferUpdate(false),
	myValueChanged(false),
	myPressed(false),
	myIncrement(0)
{
	// Set the default size
	setHeight(22);
	setWidth(100);

	setEnabled(true);
	setNavigationEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
Slider::~Slider()
{

}

///////////////////////////////////////////////////////////////////////////////
void Slider::handleEvent(const Event& evt)
{
	Widget::handleEvent(evt);
	if(isPointerInteractionEnabled())
	{
		Vector2f point = Vector2f(evt.getPosition().x(), evt.getPosition().y());
	
		point = transformPoint(point);

		Vector2f sliderPos = getSliderPosition();
		Vector2f sliderSize = getSliderSize();

		if(evt.getType() == Event::Up)
		{
			myPressed = false;
			if(myValueChanged)
			{
				Event e;
				e.reset(Event::ChangeValue, Service::Ui, getId());
				dispatchUIEvent(e);
			}
		}

		if(simpleHitTest(point, sliderPos, sliderSize))
		{
			if(evt.getType() == Event::Down)
			{
				myPressed = true;
                myPressPos = point[0];
			}
			evt.setProcessed();
		}
		if(simpleHitTest(point))
		{
			if(myPressed && evt.getType() == Event::Move)
			{
                int dx = point[0] - myPressPos;
                int dValue = dx * myTicks / mySize[0];
				int newValue = myValue + dValue;
				if(newValue < 0) newValue = 0;
				if(newValue > (myTicks - 1)) newValue = myTicks - 1;

				if(newValue != myValue)
				{
					myValue = newValue;
                    myPressPos = point[0];
					if(!myDeferUpdate)
					{
						Event e;
						e.reset(Event::ChangeValue, Service::Ui, getId());
						dispatchUIEvent(e);
					}
					else
					{
						myValueChanged = true;
					}
				}
			}
			evt.setProcessed();
		}
	}
	if(isGamepadInteractionEnabled())
	{
		if(evt.isButtonDown(Event::ButtonLeft))
		{
			evt.setProcessed();
			myIncrement = -1;
			// Full slider change takes 2 seonds
			myIncrementTimeStep = 2.0 / myTicks;
			// Force an immediate change
			myIncrementTimer = myIncrementTimeStep;
		}
		else if(evt.isButtonDown(Event::ButtonRight))
		{
			evt.setProcessed();
			myIncrement = 1;
			// Full slider change takes 2 seonds
			myIncrementTimeStep = 2.0 / myTicks;
			// Force an immediate change
			myIncrementTimer = myIncrementTimeStep;
		}
		else if(evt.getType() == Event::Up)
		{
			myIncrement = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void Slider::update(const omega::UpdateContext& context) 
{
	Widget::update(context);
	if(myIncrement != 0)
	{
		myIncrementTimer += context.dt;
		if(myIncrementTimer > myIncrementTimeStep)
		{
			myIncrementTimer = 0;
			myValue += myIncrement;
			if(myValue < 0) myValue = 0;
			else if(myValue >= myTicks) myValue = myTicks - 1;
			if(!myDeferUpdate)
			{
				Event e;
				e.reset(Event::ChangeValue, Service::Ui, getId());
				dispatchUIEvent(e);
			}
			else
			{
				myValueChanged = true;
			}
		}
	}
	//float slSize = mySize[0] / (float(myMaxValue - myMinValue));
	//mySliderSize = slSize > 20 ? slSize : 20;
};

///////////////////////////////////////////////////////////////////////////////
Vector2f Slider::getSliderSize()
{
	//return Vector2f(mySize[0] / myTicks, mySize[1] + 10);
	return Vector2f(20, mySize[1] - 4);
}

///////////////////////////////////////////////////////////////////////////////
Vector2f Slider::getSliderPosition()
{
	Vector2f position;
	Vector2f size = getSliderSize();

	position[0] = (myValue * (mySize[0] - 10) / (myTicks - 1)); // - size[0] / 2;
	position[1] = -(size[1] - mySize[1]) / 2;
	return position;
}
