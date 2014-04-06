/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2013							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
#ifndef __WIDGET_FACTORY_H__
#define __WIDGET_FACTORY_H__

#include "omegaToolkit/ui/Button.h"
#include "omegaToolkit/ui/Container.h"
#include "omegaToolkit/ui/Slider.h"
#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/ui/Image.h"
#include "omegaToolkit/ui/TextBox.h"

namespace omegaToolkit { namespace ui
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class WidgetFactory: public ReferenceType
	{
	public:
		WidgetFactory(Engine* server): 
            myServer(server),
            myFocusColor(Color::Lime),
            myLabelColor(Color::White)
        {}

		Engine* getEngine() { return myServer; }

		virtual Widget* createWidget(const String& name, Container* container)
		{
			Widget* widget = new Widget(myServer);
			widget->setName(name);
			container->addChild(widget);
			return widget;
		}

		virtual Button* createButton(const String& name, Container* container) = 0;
		virtual Slider* createSlider(const String& name, Container* container) = 0;

		virtual Button* createCheckButton(const String& name, Container* container)
		{
			Button* button = createButton(name, container);
			button->setName(name);
			button->setCheckable(true);
			return button;
		}


		virtual Image* createImage(const String& name, Container* container)
		{
			Image* image = new Image(myServer);
			container->addChild(image);
			return image;
		}

		virtual Label* createLabel(const String& name, Container* container, const String& text = "")
		{
			Label* lbl = new Label(myServer);
			lbl->setName(name);
			if(text.size()  == 0) lbl->setText(name);
			else lbl->setText(text);
			container->addChild(lbl);
			return lbl;
		}

        virtual TextBox* createTextBox(const String& name, Container* container)
        {
            TextBox* tb = new TextBox(myServer);
            tb->setName(name);
            container->addChild(tb);
            return tb;
        }

        virtual Container* createContainer(String name, Container* container,
			Container::Layout layout = Container::LayoutHorizontal)
		{
			Container* c = new Container(myServer);
			c->setName(name);
			c->setLayout(layout);
			container->addChild(c);
			return c;
		}

        //! Global styles
        //@{
        //! Sets the default color used to mark active widgets
        void setFocusColor(const Color& c) { myFocusColor = c; }
        //! Gets the default color used to mark active widgets
        const Color& getFocusColor() { return myFocusColor; }
        //! Sets the default label color
        void setLabelColor(const Color& c) { myLabelColor = c; }
        //! Gets the default label color
        const Color& getLabelColor() { return myLabelColor; }
        //@}

	private:
		Engine* myServer;
        Color myFocusColor;
        Color myLabelColor;
    };
}; // namespace ui
}; // namespace omega

#endif