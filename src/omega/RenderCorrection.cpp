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
#include "omega/Renderer.h"
#include "omega/ImageUtils.h"
#include "omega/Texture.h"
#include "omega/PixelData.h"
#include "omega/DisplaySystem.h"
#include "omega/DrawContext.h"
#include "omega/glheaders.h"

using namespace omega;

////////////////////////////////////////////////////////////////////////////////
WarpCorrection::WarpCorrection() :
    task(NULL)
{
    // EMPTY!
}

void WarpCorrection::updateViewport(const Rect& vp)
{
    if(geometry)
    {
        geometry->updateViewport(vp);
    }
}

void WarpCorrection::prepare(Renderer* client, const DrawContext& context)
{
    if((geometry == NULL) && context.tile->warpMeshFilename != "default")
    {
        Ref<WarpMeshGrid> grid = WarpMeshUtils::loadWarpMeshGrid(context.tile->warpMeshFilename);
        geometry = new WarpMeshGeometry();
        geometry->initialize(context, *grid);

        // task = WarpMeshUtils::loadWarpMeshGridAsync(context.tile->warpMeshFilename);
    }
}

void WarpCorrection::render(Renderer* client, const DrawContext& context)
{
//    if(task && task->isComplete())
    if(geometry)
    {
        geometry->render(client, context);
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

void EdgeBlendCorrection::updateViewport(const Rect& vp)
{

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

        Vector2f viewportPos = context.viewport.min.cast<omicron::real>();
        Vector2f viewportSize = context.viewport.size().cast<omicron::real>();

        glColor4f(1, 1, 1, 1);
        context.drawInterface->drawRectTexture(t, viewportPos, viewportSize);
    }
}

void EdgeBlendCorrection::dispose()
{
    // EMPTY!
}

///////////////////////////////////////////////////////////////////////////

RenderCorrection::RenderCorrection() :
    readbackTarget(NULL),
    readbackTexture(NULL)
{
    // EMPTY!
}

void RenderCorrection::initialize(Renderer* client, const DrawContext& context)
{
    if((context.tile->pixelSize[0] < 1) || (context.tile->pixelSize[1] < 1))
        return;

    if((context.tile->correctionMode == DisplayTileConfig::WarpCorrection) ||
       (context.tile->correctionMode == DisplayTileConfig::PreWarpEdgeBlendCorrection) ||
       (context.tile->correctionMode == DisplayTileConfig::PostWarpEdgeBlendCorrection))
    {
        if(readbackTarget == NULL)
        {
            readbackTarget = context.gpuContext->createRenderTarget(RenderTarget::RenderToTexture);
        }

        if(readbackTexture== NULL)
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

        // set the default target to the center
        readbackTarget->setTextureTarget(readbackTexture);

        if((context.tile->warpMeshFilename != "default"))
        {
            warpCorrection = new WarpCorrection();
            warpCorrection->prepare(client, context);
        }
    }

    if((context.tile->correctionMode == DisplayTileConfig::EdgeBlendCorrection) ||
       (context.tile->correctionMode == DisplayTileConfig::PreWarpEdgeBlendCorrection) ||
       (context.tile->correctionMode == DisplayTileConfig::PostWarpEdgeBlendCorrection))
    {
        edgeBlendCorrection = new EdgeBlendCorrection();
        edgeBlendCorrection->prepare(client, context);
    }
}

void RenderCorrection::bind(Renderer* client, const DrawContext& context)
{
    if (readbackTarget != NULL)
    {
        readbackTarget->bind();
        clear(client, context);
    }
}

void RenderCorrection::unbind(Renderer* client, const DrawContext& context)
{
    if (readbackTarget != NULL)
    {
        // apply pre-warp edge blend as an overlay
        if((context.tile->correctionMode == DisplayTileConfig::PreWarpEdgeBlendCorrection) &&
            edgeBlendCorrection != NULL)
        {
            context.drawInterface->beginDraw2D(context);
            edgeBlendCorrection->render(client, context);
            context.drawInterface->endDraw();
        }

        readbackTarget->unbind();
    }
}

void RenderCorrection::clear(Renderer *client, const DrawContext &context)
{
    GLfloat bgColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, bgColor);
    if(context.task == DrawContext::OverlayDrawTask)
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);

}

void RenderCorrection::updateViewport(const Rect& vp)
{
    viewport = vp;
    if(warpCorrection != NULL)
    {
        warpCorrection->updateViewport(vp);
    }
}

void RenderCorrection::prepare(Renderer* client, const DrawContext& context)
{

}

void RenderCorrection::render(Renderer* client, const DrawContext& context)
{
    // render warp mesh with readback texture applied
    if ((readbackTexture != NULL) && (warpCorrection != NULL))
    {
        context.drawInterface->beginDraw2D(context);

        glEnable(GL_TEXTURE_2D);
        readbackTexture->bind(GpuContext::TextureUnit0);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

        warpCorrection->render(client, context);

        glColor3f(1, 1, 1);
        readbackTexture->unbind();
        glDisable(GL_TEXTURE_2D);

        context.drawInterface->endDraw();
    }

    // apply post-warp edge blend as an overlay
    if((context.tile->correctionMode == DisplayTileConfig::PostWarpEdgeBlendCorrection) &&
        edgeBlendCorrection != NULL)
    {
        context.drawInterface->beginDraw2D(context);
        edgeBlendCorrection->render(client, context);
        context.drawInterface->endDraw();
    }
}

///////////////////////////////////////////////////////////////////////////

