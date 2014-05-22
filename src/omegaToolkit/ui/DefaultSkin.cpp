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
#include "omegaToolkit/ui/DefaultSkin.h"
#include "omega/DrawInterface.h"
#include "omega/glheaders.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

Color sBaseColor = Color(0.9f, 0.9f, 1.0f, 1.0f);

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultButtonRenderable::drawContent(const DrawContext& context)
{
	WidgetRenderable::drawContent(context);

	DrawInterface* painter = getRenderer();

	Color col = sBaseColor;
	if(myOwner->isPointerInside())
	{
		col = myOwner->getFactory()->getFocusColor();
	}

	Vector2f size = myOwner->getSize();

	// If button is checkable, draw check box.
	float checkSize = 0;
	if(myOwner->isCheckable())
	{
		if(myOwner->isRadio())
		{
			size[0] -= (size[1] + 4);
			Vector2f radioBoxPosition = Vector2f(size[1] / 2, size[1] / 2);
			painter->drawCircle(radioBoxPosition, (size[1] - 4) / 2, Color::White, 12);
			painter->drawCircle(radioBoxPosition, (size[1] - 4) / 2 - 2, Color::Black, 12);

			if(myOwner->isChecked())
			{
				painter->drawCircle(radioBoxPosition, (size[1] - 4) / 2 - 4, Color::Lime, 12);
			}
		}
		else
		{
			size[0] -= (size[1] + 4);
			Vector2f checkBoxSize = Vector2f(size[1] - 4, size[1] - 4);
			Vector2f checkBoxPosition = Vector2f(0, 4);
			painter->drawRectOutline(checkBoxPosition, checkBoxSize, sBaseColor);

			if(myOwner->isChecked())
			{
				checkBoxSize -= Vector2f(5, 5);
				checkBoxPosition += Vector2f(2, 2);
				painter->drawRect(checkBoxPosition, checkBoxSize, Color::Lime);
			}
		}
		checkSize = size[1] + 4;
		myOwner->getLabel()->setPosition(Vector2f(checkSize, 0));
	}

	if(myOwner->isImageEnabled())
	{
		myOwner->getImage()->setPosition(checkSize + 4, 0);
		ImageRenderable* ir = (ImageRenderable*)myOwner->getImage()->getRenderable(getClient());
		if(ir)
		{
			ir->draw(context);
		}
		myOwner->getLabel()->setPosition(Vector2f(checkSize + 4 + myOwner->getImage()->getSize()[0], 0));
	}
    if(myOwner->isTextEnabled())
    {
        myOwner->getLabel()->setColor(col);
        LabelRenderable* lr = (LabelRenderable*)myOwner->getLabel()->getRenderable(getClient());
        if(lr)
        {
            lr->draw(context);
        }
    }

	myAnim *= 0.8f;
	if(myOwner->isPressed()) myAnim = 1.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultSliderRenderable::drawContent(const DrawContext& context)
{
	WidgetRenderable::drawContent(context);

	DrawInterface* painter = getRenderer();

	Vector2f sliderPos = myOwner->getSliderPosition();
	Vector2f sliderSize = myOwner->getSliderSize();

	Color col = sBaseColor;
	if(myOwner->isActive())
	{
		col = Color::Lime;
	}
	painter->drawRectOutline(Vector2f::Zero(), myOwner->getSize(), col);
	painter->drawRect(sliderPos, sliderSize, Color::Gray);
	painter->drawRectOutline(sliderPos, sliderSize, col);
}

