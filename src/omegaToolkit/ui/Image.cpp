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
 *	A widget displaying a single image
 ******************************************************************************/
#include "omega/Renderer.h"
#include "omegaToolkit/ui/Image.h"
#include "omega/DrawInterface.h"
#include "omegaToolkit/ui/Container.h"

#include "omega/glheaders.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
Image* Image::create(Container* container)
{
	Image* img = new Image(Engine::instance());
	container->addChild(img);
	return img;
}

///////////////////////////////////////////////////////////////////////////////
Image::Image(Engine* srv):
	Widget(srv),
	myData(NULL),
	myFlipFlags(DrawInterface::FlipY)
{
	// By default images are set to not enabled, and won't take part in navigation.
	setEnabled(false);
	setNavigationEnabled(false);

	// Set the default shader.
	setShaderName("ui/widget-image");
}

///////////////////////////////////////////////////////////////////////////////
Image::~Image() 
{

}

///////////////////////////////////////////////////////////////////////////////
Renderable* Image::createRenderable()
{
	return new ImageRenderable(this);
}

///////////////////////////////////////////////////////////////////////////////
void Image::setData(PixelData* value) 
{ 
	myData = value; 
	setSize(Vector2f(myData->getWidth(), myData->getHeight()));
	refresh(); 
}

///////////////////////////////////////////////////////////////////////////////
void Image::flipX(bool value)
{
	if(value) myFlipFlags |= DrawInterface::FlipX;
	else myFlipFlags &= ~DrawInterface::FlipX;
}

///////////////////////////////////////////////////////////////////////////////
void Image::flipY(bool value)
{
	if(value) myFlipFlags |= DrawInterface::FlipY;
	else myFlipFlags &= ~DrawInterface::FlipY;
}

///////////////////////////////////////////////////////////////////////////////
void ImageRenderable::refresh()
{
	WidgetRenderable::refresh();
	myTextureUniform = glGetUniformLocation(myShaderProgram, "unif_Texture");
}

///////////////////////////////////////////////////////////////////////////////
ImageRenderable::~ImageRenderable() 
{
}

///////////////////////////////////////////////////////////////////////////////
void ImageRenderable::drawContent(const DrawContext& context)
{
	WidgetRenderable::drawContent(context);

	PixelData* tex = myOwner->getData();
	if(tex != NULL)
	{
		DrawInterface* di = getRenderer();
		di->fillTexture(tex);
		di->textureRegion(0, 0, 1, 1);

		if(myTextureUniform != 0)
		{
			glUniform1i(myTextureUniform, 0);
		}

		if(myOwner->isStereo())
		{
			DrawContext::Eye eye = context.eye;
			if(eye == DrawContext::EyeLeft)
			{
				di->textureRegion(0, 0, 0.5f, 1);
			}
			else if(eye == DrawContext::EyeRight)
			{
				di->textureRegion(0.5f, 0, 0.5f, 1);
			}
		}
		else
		{
			Vector2f size = myOwner->getSize();
			di->rect(0, 0, size[0], size[1]);
		}
	}
	else
	{
		refresh();
	}
}
