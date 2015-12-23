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
*	A texture source that fills a texture with a string.
******************************************************************************/
#include "StringTextureSource.h"
#include "omega/Renderer.h"
#include <omegaGl.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
void StringTextureSource::refreshTexture(Texture* texture, const DrawContext& context)
{
    // Compute text size.
    Vector2f textSize = Font::getTextSize(myText, myFontStyle);
    DrawInterface* di = context.renderer->getRenderer();

    // Initialize the texture and render target (if needed)
    if(!texture->isInitialized()) texture->initialize(textSize[0], textSize[1]);
    if(myRenderTarget == NULL)
    {
        myRenderTarget = context.renderer->createRenderTarget(RenderTarget::RenderToTexture);
        myRenderTarget->setTextureTarget(texture);
    }

    // Render the string to the texture.
    myRenderTarget->bind();
    myRenderTarget->clear();

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, textSize[0], textSize[1]);
                
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, textSize[0], textSize[1], 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    Font* fnt = di->getFont(myFontStyle);
    di->drawText(myText, fnt, Vector2f::Zero(), Font::HALeft | Font::VATop, myFontColor);
    di->drawRect(Vector2f::Zero(), Vector2f(1000, 1000), Color::White);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    myRenderTarget->unbind();
}
