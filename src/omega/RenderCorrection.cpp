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
* What's in this file
*	Support for offscreen rendering to various targets
******************************************************************************/
#include "omega/RenderCorrection.h"
#include "omega/ImageUtils.h"
#include "omega/Texture.h"
#include "omega/PixelData.h"
#include "omega/DisplaySystem.h"
#include "omega/glheaders.h"

using namespace omega;

////////////////////////////////////////////////////////////////////////////////
WarpCorrection::WarpCorrection() :
    task(NULL)
{
    // EMPTY!
}

void WarpCorrection::prepare(Renderer* client, const DrawContext& context)
{
    if((task == NULL) && context.tile->warpMeshFilename != "default")
    {
        // task = new loadMeshAsync(context.tile->edgeBlendFilename);
    }
}

void WarpCorrection::render(Renderer* client, const DrawContext& context)
{
    if(task && task->isComplete())
    {
    }
}

void WarpCorrection::dispose()
{

}

////////////////////////////////////////////////////////////////////////////////
EdgeBlendCorrection::EdgeBlendCorrection() :
    task(NULL)
{
    // EMPTY!
}

void EdgeBlendCorrection::prepare(Renderer* client, const DrawContext& context)
{
    if((task == NULL) && context.tile->edgeBlendFilename != "default")
    {
        task = ImageUtils::loadImageAsync(context.tile->edgeBlendFilename);
    }
}
void EdgeBlendCorrection::render(Renderer* client, const DrawContext& context)
{
    if(task && task->isComplete())
    {
        Texture* t = task->getData().image->getTexture(context);

        context.drawInterface->beginDraw2D(context);

        Vector2f pos = context.tile->activeCanvasRect.min.cast<omicron::real>();
        Vector2f size = context.tile->activeCanvasRect.size().cast<omicron::real>();

        glColor3f(0, 1, 0);
        context.drawInterface->drawRectTexture(t, pos, size);
        glColor3f(1, 1, 1);

        context.drawInterface->endDraw();
    }
}

void EdgeBlendCorrection::dispose() {}

///////////////////////////////////////////////////////////////////////////

RenderCorrection::RenderCorrection() :
    readbackTarget(NULL),
    readbackTexture(NULL)
{

}

void RenderCorrection::initialize(Renderer* client, const DrawContext& context)
{
    if((context.tile->pixelSize[0] < 1) || (context.tile->pixelSize[1] < 1))
        return;

    if(readbackTarget == NULL)
    {
        readbackTarget = context.gpuContext->createRenderTarget(RenderTarget::RenderToTexture);
    }

    if(readbackTexture == NULL)
    {
        readbackTexture = context.gpuContext->createTexture();
        readbackTexture->initialize(context.tile->pixelSize[0], context.tile->pixelSize[1], Texture::Type2D, Texture::ChannelRGBA);
        readbackTarget->setTextureTarget(readbackTexture);
    }
    else if((readbackTexture->getWidth() != context.tile->pixelSize[0]) || (readbackTexture->getHeight() != context.tile->pixelSize[1]))
    {
        readbackTexture->dispose();
        readbackTexture->initialize(context.tile->pixelSize[0], context.tile->pixelSize[1], Texture::Type2D, Texture::ChannelRGBA);
        readbackTarget->setTextureTarget(readbackTexture);
    }
}

void RenderCorrection::bind(Renderer* client, const DrawContext& context)
{
    if (readbackTarget != NULL)
    {
        readbackTarget->bind();
    }
}

void RenderCorrection::unbind(Renderer* client, const DrawContext& context)
{
    if (readbackTarget != NULL)
    {
        readbackTarget->unbind();
    }
}

void RenderCorrection::render(Renderer* client, const DrawContext& context)
{
    if (readbackTexture != NULL)
    {
        context.drawInterface->beginDraw2D(context);

        Vector2f pos = context.tile->activeCanvasRect.min.cast<omicron::real>();
        Vector2f size = context.tile->activeCanvasRect.size().cast<omicron::real>();

        glColor3f(1, 0, 0);
        context.drawInterface->drawRectTexture(readbackTexture, pos, size);
        glColor3f(1, 1, 1);

        context.drawInterface->endDraw();
    }
}

///////////////////////////////////////////////////////////////////////////

