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
#ifndef __DEFAULT_SKIN_H__
#define __DEFAULT_SKIN_H__

#include "omegaToolkit/ui/WidgetFactory.h"

namespace omegaToolkit { namespace ui {
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class OTK_API DefaultButtonRenderable: public WidgetRenderable
	{
	public:
		DefaultButtonRenderable(Button* owner): WidgetRenderable(owner), myOwner(owner) {}
		virtual ~DefaultButtonRenderable() {}
	protected:
		virtual void drawContent(const DrawContext& context);
	private:
		Button* myOwner;
		float myAnim;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class OTK_API DefaultSliderRenderable: public WidgetRenderable
	{
	public:
		DefaultSliderRenderable(Slider* owner): WidgetRenderable(owner), myOwner(owner) {}
		virtual ~DefaultSliderRenderable() {}
	protected:
		virtual void drawContent(const DrawContext& context);
	private:
		Slider* myOwner;
		float myAnim;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class DefaultButton: public Button
	{
	friend class DefaultButtonRenderable;
	public:
		DefaultButton(Engine* srv): Button(srv) { }
		virtual ~DefaultButton() {}
		virtual Renderable* createRenderable()  { return new DefaultButtonRenderable(this); }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class DefaultSlider: public Slider
	{
	friend class DefaultSliderRenderable;
	public:
		DefaultSlider(Engine* srv): Slider(srv) {}
		virtual ~DefaultSlider() {}
		virtual Renderable* createRenderable() { return new DefaultSliderRenderable(this); }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class DefaultWidgetFactory: public WidgetFactory
	{
	public:
		DefaultWidgetFactory(Engine* srv): WidgetFactory(srv) {}

		virtual Button* createButton(const String& name, Container* container)
		{
			Button* button = new DefaultButton(getEngine());
			button->setName(name);
			button->getLabel()->setText(name);
			container->addChild(button);
			return button;
		}

		virtual Slider* createSlider(const String& name, Container* container)
		{
			Slider* slider = new DefaultSlider(getEngine());
			container->addChild(slider);
			return slider;
		}
	};
};};
#endif