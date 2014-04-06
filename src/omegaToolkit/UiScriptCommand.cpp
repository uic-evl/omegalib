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
#include "omega/PythonInterpreter.h"
#include "omegaToolkit/UiScriptCommand.h"
#include "omegaToolkit/ui/AbstractButton.h"
//#include "omegaToolkit/ui/Slider.h"

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

/////////////////////////////////////////////////////////////////////////////////////////////////
UiScriptCommand::UiScriptCommand():
	myUiEventsOnly(false)
{
	myUI = UiModule::instance();
	myInterpreter = SystemManager::instance()->getScriptInterpreter();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
UiScriptCommand::UiScriptCommand(const String& command):
	myCommand(command),
	myUiEventsOnly(false)
{
	myUI = UiModule::instance();
	myInterpreter = SystemManager::instance()->getScriptInterpreter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UiScriptCommand::handleEvent(const Event& evt)
{
	if(myUiEventsOnly && evt.getServiceType() != Event::ServiceTypeUi) return;
	
	if(myInterpreter != NULL)
	{
		if(evt.getType() == Event::Toggle)
		{
			AbstractButton* btn = Widget::getSource<AbstractButton>(evt);
			if(btn != NULL)
			{
				String expr = StringUtils::replaceAll(myCommand, "%value%", ostr("%1%", %btn->isChecked()));
				myInterpreter->evalEventCommand(expr, evt);
				evt.setProcessed();
			}
		}
		else if(evt.getType() == Event::ChangeValue)
		{
			Slider* sld = Widget::getSource<Slider>(evt);
			if(sld != NULL)
			{
				int value = sld->getValue();
				String expr = StringUtils::replaceAll(myCommand, "%value%", ostr("%1%", %value));
				myInterpreter->evalEventCommand(expr, evt);
				evt.setProcessed();
			}
            else
            {
                TextBox* tb = Widget::getSource<TextBox>(evt);
                if(tb != NULL)
                {
                    String value = tb->getText();
                    String expr = StringUtils::replaceAll(myCommand, "%value%", ostr("%1%", %value));
                    myInterpreter->evalEventCommand(expr, evt);
                    evt.setProcessed();
                }
            }
        }
		else if(evt.getType() == Event::Click)
		{
			myInterpreter->evalEventCommand(myCommand, evt);
			evt.setProcessed();
		}
	}
}

