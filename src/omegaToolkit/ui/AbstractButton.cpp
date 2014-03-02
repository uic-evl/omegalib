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
#include "omegaToolkit/ui/AbstractButton.h"
#include "omegaToolkit/ui/Container.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractButton::AbstractButton(Engine* server):
	Widget(server),
	myCheckable(false),
	myChecked(false),
	myRadio(false),
	myPressed(false),
	myPressedStateChanged(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractButton::~AbstractButton()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractButton::update(const omega::UpdateContext& context) 
{
	Widget::update(context);
	if(myPressedStateChanged)
	{
		// Button was pressed, and now it's not (that is, it has been clicked). Generate a click event.
		if(!myPressed)
		{
			// If button is checkable, toggle check state.
			if(myCheckable)
			{
				// If this button is a radio button that switched to checked state, uncheck all 
				// other radio buttons in the same container.
				// NOTE: For radio buttons, only the button that is being switched will dispatch an event.
				if(myRadio)
				{
					if(!myChecked)
					{
						myChecked = true;
						Container* c = getContainer();
						if(c != NULL)
						{
							int nc = c->getNumChildren();
							for(int i = 0; i < nc; i++)
							{
								AbstractButton* btn = dynamic_cast<AbstractButton*>(c->getChildByIndex(i));
								if(btn != NULL && btn != this && btn->isRadio())
								{
									btn->myChecked = false;
								}
							}
						}
						Event evt;
						evt.reset(Event::Toggle, Service::Ui, getId());
						dispatchUIEvent(evt);
					}
				}
				else
				{
					myChecked = !myChecked;
					Event evt;
					evt.reset(Event::Toggle, Service::Ui, getId());
					dispatchUIEvent(evt);
				}
			}
			else
			{
				Event evt;
				evt.reset(Event::Click, Service::Ui, getId());
				dispatchUIEvent(evt);
			}
		}
		
		myPressedStateChanged = false;
	}
};
