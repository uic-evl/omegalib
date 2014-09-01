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
 *	A slider widget
 ******************************************************************************/
#ifndef __SLIDER_H__
#define __SLIDER_H__

#include "omegaToolkit/ui/AbstractButton.h"
#include "omegaToolkit/ui/Label.h"

namespace omegaToolkit { namespace ui {
	///////////////////////////////////////////////////////////////////////////
	class OTK_API Slider: public Widget
	{
	public:
		static Slider* create(Container* container);

	public:
		Slider(Engine* srv);
		virtual ~Slider();

		virtual void handleEvent(const omega::Event& evt);

		int getValue();
		void setValue(int value);

		int getTicks();
		void setTicks(int value);

		Vector2f getSliderSize();
		Vector2f getSliderPosition();

		void setDeferUpdate(bool value);
		bool getDeferUpdate();

	protected:
		virtual void update(const omega::UpdateContext& context);

	protected:
		bool myPressed;
		int myPressPos;

		bool myDeferUpdate;
		bool myValueChanged;

		int myValue;
		int myTicks;

		// Gamepad change
		int myIncrement;
		float myIncrementTimer;
		float myIncrementTimeStep;
	};

	///////////////////////////////////////////////////////////////////////////
	inline int Slider::getValue() 
	{ return myValue; }

	///////////////////////////////////////////////////////////////////////////
	inline void Slider::setValue(int value) 
	{ myValue = value; }

	///////////////////////////////////////////////////////////////////////////
	inline int Slider::getTicks() 
	{ return myTicks; }

	///////////////////////////////////////////////////////////////////////////
	inline void Slider::setTicks(int value) 
	{ myTicks = value; }

	///////////////////////////////////////////////////////////////////////////
	inline void Slider::setDeferUpdate(bool value)
	{ myDeferUpdate = value; }

	///////////////////////////////////////////////////////////////////////////
	inline bool Slider::getDeferUpdate()
	{ return myDeferUpdate; }
}; 
}; // namespace omegaToolkit

#endif
