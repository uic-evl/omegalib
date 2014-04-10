/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
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
 *	A dynamic text field
 ******************************************************************************/
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/TextBox.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

NameGenerator sTextBoxNameGenerator("TextBox");

///////////////////////////////////////////////////////////////////////////////
TextBox* TextBox::create(Container* container)
{
    WidgetFactory* wf = UiModule::instance()->getWidgetFactory();
    TextBox* tb = wf->createTextBox(sTextBoxNameGenerator.generate(), container);
    return tb;
}

///////////////////////////////////////////////////////////////////////////////
TextBox::TextBox(Engine* srv) :
    Label(srv),
    myCaretPos(0)
{
    // By default labels are set to not enabled, and won't take part in navigation.
    setEnabled(true);
    setNavigationEnabled(true);
    setAutosize(false);

    // Set the default shader.
    setShaderName("ui/widget-label");

    setStyleValue("align", "middle-left");

    // Set default style
    setFillColor(Color::White);
    setFillEnabled(true);
    setColor(Color::Black);
}

///////////////////////////////////////////////////////////////////////////////
TextBox::~TextBox()
{

}

///////////////////////////////////////////////////////////////////////////////
void TextBox::updateSize()
{
    if(needLayoutRefresh())
    {
        if(myFont.size() == 0)
        {
            myFont = Engine::instance()->getDefaultFont().filename + " " +
                boost::lexical_cast<String>(Engine::instance()->getDefaultFont().size);
        }
        Vector2f size = Font::getTextSize("A", myFont);
        size[1] = size[1] + myAutosizeVerticalPadding;
        setHeight(size[1]);
    }
}

///////////////////////////////////////////////////////////////////////////////
void TextBox::activate()
{
    const Color& fc = getFactory()->getFocusColor();
    for(int i = 0; i < 4; i++)
    {
        getBorderStyle(i).width = 2;
        getBorderStyle(i).color = fc;
    }
}

///////////////////////////////////////////////////////////////////////////////
void TextBox::deactivate()
{
    for(int i = 0; i < 4; i++)
    {
        getBorderStyle(i).width = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
void TextBox::handleEvent(const Event& evt)
{
    Label::handleEvent(evt);
    if(isActive())
    {
        if(evt.getServiceType() == Event::ServiceTypeKeyboard && 
            evt.getType() == Event::Down)
        {
            // Process special keys
            if(evt.isFlagSet(Event::Enter))
            {
                Event e;
                e.reset(Event::ChangeValue, Service::Ui, getId());
                dispatchUIEvent(e);
            }
            else if(evt.isFlagSet(Event::Button5))
            {
                if(myCaretPos > 0)
                {
                    String s = getText();
                    String res = s.substr(0, myCaretPos - 1);
                    if(myCaretPos < s.size()) res.append(s.substr(myCaretPos));
                    setText(res);
                    myCaretPos--;
                    evt.setProcessed();
                }
            }
            else if(evt.isFlagSet(Event::Left))
            {

            }
            else if(evt.isFlagSet(Event::Right))
            {

            }
            else
            {
                char c;
                if(evt.getChar(&c))
                {
                    String s = getText();
                    s = s.insert(myCaretPos, 1, c);
                    setText(s);
                    myCaretPos++;
                }
            }
            evt.setProcessed();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void TextBox::update(const omega::UpdateContext& context)
{
    Label::update(context);
}

