/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2016		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *  Koosha Mirhosseini		koosha.mirhosseini@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2016, Electronic Visualization Laboratory,  
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
 *	Contains utility functions used to draw and manage graphic resources
 ******************************************************************************/
#include "omega/DrawContext.h"
#include "omega/Renderer.h"
#include "omega/DisplaySystem.h"
#include "omega/Camera.h"
#include "omega/RenderCorrection.h"
#include "omega/glheaders.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
DrawContext::DrawContext():
    quadInitialized(0),
    stencilInitialized(0),
    camera(NULL),
    renderCorrection(NULL),
    stencilMaskWidth(0),
    stencilMaskHeight(0)
{
    drawInterface = new DrawInterface();
}
///////////////////////////////////////////////////////////////////////////////
DrawContext::~DrawContext()
{
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::pushTileConfig(DisplayTileConfig* newtile)
{
    // If the custom tile is marked as part of a tile grid, the tile inherits 
    // the canvas rects of the current tile. This insures that
    // camera overlaps, viewport and transform calculations work as expected for
    // custom tiles.
    if(newtile->isInGrid)
    {
        const Vector2i& cs = tile->displayConfig.getCanvasRect().size();
        const Vector2f vp = camera->getViewPosition();
        const Vector2f vs = camera->getViewSize();

        newtile->activeCanvasRect = tile->activeCanvasRect;
        newtile->activeRect = tile->activeRect;
        newtile->offset = tile->offset;
        newtile->position = tile->activeRect.min - tile->activeCanvasRect.min;
        newtile->position += Vector2i(vp[0] * cs[0], vp[1] * cs[1]);

        // Compute the tile size based on the camera view size and the canvas pixel
        // size.
        newtile->pixelSize = Vector2i(vs[0] * cs[0], vs[1] * cs[1]);
    }
    else
    {
        // For non-grid tiles, the tile active rect is just the full tile
        newtile->activeRect.min = Vector2i::Zero();
        newtile->activeRect.max = newtile->pixelSize;
    }

    tileStack.push(tile); 
    tile = newtile;
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::popTileConfig()
{
    tile = tileStack.front(); tileStack.pop();
}

///////////////////////////////////////////////////////////////////////////////
DisplayTileConfig::StereoMode DrawContext::getCurrentStereoMode()
{
    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    if(dcfg.forceMono) return DisplayTileConfig::Mono;
    if(tile->stereoMode == DisplayTileConfig::Default) return dcfg.stereoMode;
    return tile->stereoMode;
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::prepare(uint64 frameNum)
{
    this->frameNum = frameNum;
    renderer->prepare(*this);
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::drawFrame(uint64 frameNum)
{
    //omsg("----------------------- FRAME BEGIN");

    // If needed, increase the stencil update countdown.
    if(stencilInitialized < 0) stencilInitialized++;

    // If the current tile is not enabled, return now.
    if(!tile->enabled) return;

    this->frameNum = frameNum;

    FrameInfo curFrame(frameNum, gpuContext);

    // Clear the active main frame buffer.
    //clear();
    renderer->clear(*this);
    if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
    {\
        renderCorrection->clear(renderer, *this);
    }

    // Signal the start of a new frame
    renderer->startFrame(curFrame);

    // Setup the stencil buffer if needed.
    // The stencil buffer is set up if th tile is using an interleaved mode (line or pixel)
    // or if the tile is left in default mode and the global stereo mode is an interleaved mode
    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    if(tile->stereoMode == DisplayTileConfig::LineInterleaved ||
        tile->stereoMode == DisplayTileConfig::ColumnInterleaved ||
        tile->stereoMode == DisplayTileConfig::PixelInterleaved ||
        (tile->stereoMode == DisplayTileConfig::Default && (
                dcfg.stereoMode == DisplayTileConfig::LineInterleaved ||
                dcfg.stereoMode == DisplayTileConfig::ColumnInterleaved ||
                dcfg.stereoMode == DisplayTileConfig::PixelInterleaved)))
    {
        // If the window size changed, we will have to recompute the stencil mask
        // We need to postpone this a few frames, since the underlying window and
        // framebuffer may have not been rezized be the OS yet. We use a countdown
        // field for this. Keeping this value a bit high also makes sure the
        // display does not keep flashing as the window moves around.
        // The larger the number, the less likely the screen is to flash, but the
        // longer the stencil will take to refresh once the window stops moving.
        if(stencilMaskWidth != tile->activeRect.width() ||
            stencilMaskHeight != tile->activeRect.height())
        {
            stencilInitialized = -30;
            stencilMaskWidth = tile->activeRect.width();
            stencilMaskHeight = tile->activeRect.height();
        }

        // If stencil is not initialized recompute
        // the stencil mask.
        if(stencilInitialized == 0)
        {
            initializeStencilInterleaver();
        }
    }
    
    if(getCurrentStereoMode() == DisplayTileConfig::Quad)
    {
        if (quadInitialized == 0)
        {
            initializeQuad();
        }
    }
    
    if(getCurrentStereoMode() == DisplayTileConfig::Mono)
    {
        eye = DrawContext::EyeCyclop;
        // Draw scene
        task = DrawContext::SceneDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        // Draw overlay
        task = DrawContext::OverlayDrawTask;
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }
    }
    else if(getCurrentStereoMode() == DisplayTileConfig::Quad)
    {
        // Draw left eye scene and overlay
        glDrawBuffer(GL_BACK_LEFT);
        glClear( GL_COLOR_BUFFER_BIT );
        glClear( GL_DEPTH_BUFFER_BIT );
        renderer->clear(*this);
        eye = DrawContext::EyeLeft;
        task = DrawContext::SceneDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        task = DrawContext::OverlayDrawTask;
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }

        // Draw right eye scene and overlay
        glDrawBuffer(GL_BACK_RIGHT);
        glClear( GL_COLOR_BUFFER_BIT );
        glClear( GL_DEPTH_BUFFER_BIT );
        renderer->clear(*this);
        eye = DrawContext::EyeRight;
        task = DrawContext::SceneDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        task = DrawContext::OverlayDrawTask;
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }

        // Draw mono overlay
        eye = DrawContext::EyeCyclop;
        task = DrawContext::OverlayDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }
    }
    else
    {
        // Draw left eye scene and overlay
        eye = DrawContext::EyeLeft;
        task = DrawContext::SceneDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        task = DrawContext::OverlayDrawTask;
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }

        // Draw right eye scene and overlay
        eye = DrawContext::EyeRight;
        task = DrawContext::SceneDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
        }
        renderer->draw(*this);
        task = DrawContext::OverlayDrawTask;
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->render(renderer, *this);
        }

        // Use viewport for last eye for overlay rendercorrection
        Rect rcViewport = viewport;

        // Draw mono overlay
        eye = DrawContext::EyeCyclop;
        task = DrawContext::OverlayDrawTask;
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->bind(renderer, *this);
            renderCorrection->clear(renderer, *this);
        }
        renderer->draw(*this);
        if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
        {
            renderCorrection->unbind(renderer, *this);
            renderCorrection->updateViewport(rcViewport);
            renderCorrection->render(renderer, *this);
        }
    }

    // Signal the end of this frame.
    renderer->finishFrame(curFrame);

    oassert(!oglError);
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::updateViewport()
{
    // Allocate the readback buffer and texture if we need to apply post-draw corrections
    if(isRenderCorrectionEnabled())
    {
        if(renderCorrection == NULL)
        {
            renderCorrection = new RenderCorrection();
            renderCorrection->initialize(renderer, *this);
        }
    }

    // If this tile is not part of a tile grid, no canvas rect computations are
    // needed. The viewport is just the full tile.
    // NOTE: we could still take the camera view position/size into account here...
    if(!tile->isInGrid)
    {
        viewport.min = Vector2i::Zero();
        viewport.max = tile->pixelSize;
        return;
    }
    
    DisplaySystem* ds = renderer->getDisplaySystem();
    //DisplayConfig& dcfg = ds->getDisplayConfig();

    const Rect& cr = tile->displayConfig.getCanvasRect();
    Vector2f vp = camera->getViewPosition();
    Vector2f vs = camera->getViewSize();

    // View rect contains the camera view rectangle in pixel coordinates.
    Rect viewRect((int)(vp[0] * cr.width()), (int)(vp[1] * cr.height()),
        (int)(vs[0] * cr.width()), (int)(vs[1] * cr.height()));

    // Compute the intersection between the view rect and the local canvas rect
    std::pair<bool, Rect> vprect = viewRect.getIntersection(tile->activeCanvasRect);

    // If intersection is null, there is nothing to render for this tile/camera.
    // just return now. Note that we should not be even getting here in the
    // first place, since Camera::isEnabledInContext should return false for
    // this context.
    if(!vprect.first) return;

    // Get the viewport coordinates
    int pvpx = vprect.second.x() - tile->activeCanvasRect.x();
    int pvpy = vprect.second.y() - tile->activeCanvasRect.y();
    int pvpw = vprect.second.width();
    int pvph = vprect.second.height();
    pvpy = tile->activeRect.height() - (pvpy + pvph);

    // Setup side-by-side stereo if needed.
    if(isSideBySideStereoEnabled())
    {
        // Do we want to invert stereo?
        bool invertStereo = ds->getDisplayConfig().invertStereo || tile->invertStereo; 

        if(eye == DrawContext::EyeLeft)
        {
            if(invertStereo)
            {
                viewport = Rect(pvpx + pvpw / 2, pvpy, pvpw / 2, pvph);
            }
            else
            {
                viewport = Rect(pvpx, pvpy, pvpw / 2, pvph);
            }
        }
        else if(eye == DrawContext::EyeRight)
        {
            if(invertStereo)
            {
                viewport = Rect(pvpx, pvpy, pvpw / 2, pvph);
            }
            else
            {
                viewport = Rect(pvpx + pvpw / 2, pvpy, pvpw / 2, pvph);
            }
        }
        else
        {
            viewport = Rect(pvpx, pvpy, pvpw, pvph);
        }
    }
    else
    {
        viewport = Rect(pvpx, pvpy, pvpw, pvph);
    }

    drawInterface->setScissor(viewport);
    if(isRenderCorrectionEnabled() && (renderCorrection != NULL))
    {
        renderCorrection->updateViewport(viewport);
    }
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::setupStereo()
{
    if (getCurrentStereoMode() == DisplayTileConfig::Quad)
    {
        DisplaySystem* ds = renderer->getDisplaySystem();
        DisplayConfig& dcfg = ds->getDisplayConfig();
        if(quadInitialized)
        {
            if(dcfg.forceMono || eye == DrawContext::EyeCyclop)
            {
                glDrawBuffer(GL_BACK); //to avoid interaction with quad content
            }
            else
            {
                if(eye == DrawContext::EyeLeft)
                {
                    glDrawBuffer(GL_BACK_LEFT);
                }
                else if(eye == DrawContext::EyeRight)
                {
                    glDrawBuffer(GL_BACK_RIGHT);
                }
            }
        }
    }
    
    else if (getCurrentStereoMode() == DisplayTileConfig::LineInterleaved)
    {
        DisplaySystem* ds = renderer->getDisplaySystem();
        DisplayConfig& dcfg = ds->getDisplayConfig();
        
        if(stencilInitialized)
        {
            if(dcfg.forceMono || eye == DrawContext::EyeCyclop)
            {
                glStencilFunc(GL_ALWAYS,0x2,0x2); // to avoid interaction with stencil content
            }
            else
            {
                if(eye == DrawContext::EyeLeft)
                {
                    glStencilFunc(GL_NOTEQUAL,0x2,0x2); // draws if stencil <> 1
                }
                else if(eye == DrawContext::EyeRight)
                {
                    glStencilFunc(GL_EQUAL,0x2,0x2); // draws if stencil <> 0
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::setupInterleaver()
{
    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    
    // Configure stencil test when rendering interleaved with stencil is enabled.
    if(stencilInitialized)
    {
        if(dcfg.forceMono || eye == DrawContext::EyeCyclop)
        {
            // Disable stencil
            glStencilFunc(GL_ALWAYS,0x2,0x2); // to avoid interaction with stencil content
        }
        else
        {
            //glStencilMask(0x2);
            if(eye == DrawContext::EyeLeft)
            {
                glStencilFunc(GL_NOTEQUAL,0x2,0x2); // draws if stencil <> 1
            }
            else if(eye == DrawContext::EyeRight)
            {
                glStencilFunc(GL_EQUAL,0x2,0x2); // draws if stencil <> 0
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::initializeQuad()
{
    GLboolean g_valid3D = false;
    glGetBooleanv(GL_STEREO, &g_valid3D);
    
    int gliWindowWidth = tile->activeRect.width();
    int gliWindowHeight = tile->activeRect.height();
    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    
    glViewport(0,0,gliWindowWidth,gliWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (g_valid3D)
    {
        glEnable(GL_STEREO);
    }
    quadInitialized = 1;
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::initializeStencilInterleaver()
{
    int gliWindowWidth = tile->activeRect.width();
    int gliWindowHeight = tile->activeRect.height();
    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    GLint gliStencilBits;
    glGetIntegerv(GL_STENCIL_BITS,&gliStencilBits);

    //EqualizerDisplaySystem* ds = dynamic_cast<EqualizerDisplaySystem*>(SystemManager::instance()->getDisplaySystem());
    DisplayTileConfig::StereoMode stereoMode = tile->stereoMode;
    if(stereoMode == DisplayTileConfig::Default) stereoMode = dcfg.stereoMode;

    // seting screen-corresponding geometry
    glViewport(0,0,gliWindowWidth,gliWindowHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.5,gliWindowWidth + 0.5,0.5,gliWindowHeight + 0.5);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
        
        
    // clearing and configuring stencil drawing
    glDrawBuffer(GL_BACK);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x2);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE); // colorbuffer is copied to stencil
    glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS,0xFF,0xFF); // to avoid interaction with stencil content
    
    // drawing stencil pattern
    glColor4f(1,1,1,0);	// alpha is 0 not to interfere with alpha tests
    
    if(stereoMode == DisplayTileConfig::LineInterleaved)
    {
        // Do we want to invert stereo?
        bool invertStereo = ds->getDisplayConfig().invertStereo || tile->invertStereo;
        
        if(tile->activeRect.max[1] %2 != 0) invertStereo = !invertStereo;
        
        int startOffset = invertStereo ? -1 : -2;

        for(float gliY = startOffset; gliY <= gliWindowHeight; gliY += 2)
        {
            glLineWidth(1);
            glBegin(GL_LINES);
                glVertex2f(0, gliY);
                glVertex2f(gliWindowWidth, gliY);
            glEnd();	
        }
    }	
    else if(stereoMode == DisplayTileConfig::ColumnInterleaved)
    {
        // Do we want to invert stereo?
        bool invertStereo = ds->getDisplayConfig().invertStereo || tile->invertStereo; 
        int startOffset = invertStereo ? -1 : -2;

        for(float gliX = startOffset; gliX <= gliWindowWidth; gliX += 2)
        {
            glLineWidth(1);
            glBegin(GL_LINES);
                glVertex2f(gliX, 0);
                glVertex2f(gliX,gliWindowHeight);
            glEnd();	
        }
    }
    else if(stereoMode == DisplayTileConfig::PixelInterleaved)
    {
        for(float gliX=-2; gliX<=gliWindowWidth; gliX+=2)
        {
            glLineWidth(1);
            glBegin(GL_LINES);
                glVertex2f(gliX, 0);
                glVertex2f(gliX, gliWindowHeight);
            glEnd();	
        }
    }
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP); // disabling changes in stencil buffer
    glFlush();

    stencilInitialized = 1;
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::updateTransforms(
    const AffineTransform3& head, 
    const AffineTransform3& view, 
    float eyeSeparation,
    float nearZ,
    float farZ)
{
    // VIewmin an dviewmax are the normalized size / position of the current
    // view, with respect to the tile pixel position and size. These values
    // are used to adjust the tile physical corners when generating the
    // projection transform in updateTransforms.
    Vector2f a(1.0f / tile->pixelSize[0], 1.0f / tile->pixelSize[1]);

    // Position of viewport wrt tile origin (excluding tile position)
    Vector2f pm(
        tile->activeRect.x() + viewport.x(), 
        tile->activeRect.y() - viewport.y() + tile->activeRect.height() - viewport.height());

    Vector2f pM(viewport.width(), viewport.height());
    // If we are on side by side mode, the viewport is half horizontal size:
    // get back the original width here since we need it to compute the correct
    // transform
    if(isSideBySideStereoEnabled())
    {
        pm.x() = tile->activeRect.x();
        pM.x() *= 2;
    }

    // Normalized viewport position
    Vector2f viewMin = (pm - tile->position.cast<real>()).cwiseProduct(a);
    Vector2f viewMax = (pm + pM - tile->position.cast<real>()).cwiseProduct(a);

    viewMin = viewMin.cwiseMax(Vector2f::Zero());
    viewMax = viewMax.cwiseMin(Vector2f::Ones());

    DisplaySystem* ds = renderer->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    
    Vector3f pa = tile->bottomLeft;
    Vector3f pb = tile->bottomRight;
    Vector3f pc = tile->topLeft;
    
    if(tile->isHMD)
    {
        pa = head * pa;
        pb = head * pb;
        pc = head * pc;
    }

    // half eye separation
    float hes = eyeSeparation / 2;
    Vector3f pe = Vector3f::Zero();
    switch(eye)
    {
    case EyeLeft:
        pe[0] = -hes;
        break;
    case EyeRight:
        pe[0] = hes;
        break;
    }

    pe = dcfg.computeEyePosition(pe, head, *this);

    Vector3f vr = pb - pa;
    Vector3f vu = pc - pa;
    Vector3f vn = vr.cross(vu);

    //Vector2f viewSize = viewMax - viewMin;

    // Update tile corners based on local view position and size
    pb = pa + vr * viewMax[0];
    pc = pa + vu * (1.0f - viewMin[1]);
    pa = pa + vr * viewMin[0] + vu * (1.0f - viewMax[1]);

    vr.normalize();
    vu.normalize();
    vn.normalize();

    // Compute the screen corner vectors.
    Vector3f va = pa - pe;
    Vector3f vb = pb - pe;
    Vector3f vc = pc - pe;

    // Find distance from eye to screen plane.
    //Vector3f tm = pe - pa;
    float d = -(vn.dot(va));

    // Find the extent of the perpendicular projection.
    float l = vr.dot(va) * nearZ / d;
    float r = vr.dot(vb) * nearZ / d;
    float b = vu.dot(va) * nearZ / d;
    float t = vu.dot(vc) * nearZ / d;

    // Compute the projection matrix. 
    Transform3 oax;
    oax.setIdentity();
    oax(0,0) = 2 * nearZ / (r - l);
    oax(0,2) = (r + l) / (r - l);
    oax(1,1) = 2 * nearZ / (t - b);
    oax(1,2) = (t + b) / (t - b);
    oax(2,2) = - (farZ + nearZ) / (farZ - nearZ);
    oax(2,3) = - (2 * farZ * nearZ) / (farZ - nearZ);
    oax(3,2) = - 1;
    oax(3,3) = 0;

    projection = oax; 
    
    // Compute the view matrix. The view matrix has two main components:
    // - the navigational component given by myViewTransform, converts points
    //   from world space to 'camera' space (origin is determined by camera position / orientation)
    // - the screen plane component, given by the current tile orientation and head position.
    //   this component converts points from camera space to screen-oriented eye space 
    //   (that is, origin is at eye position, and orientation is determined by the screen plane,
    //   with positive Y being screen up vector, X being screen right vector and Z being screen normal)
    AffineTransform3 newBasis;
    newBasis.setIdentity();
    newBasis.data()[0] = vr[0];
    newBasis.data()[1] = vu[0];
    newBasis.data()[2] = vn[0];

    newBasis.data()[4] = vr[1];
    newBasis.data()[5] = vu[1];
    newBasis.data()[6] = vn[1];

    newBasis.data()[8] = vr[2];
    newBasis.data()[9] = vu[2];
    newBasis.data()[10] = vn[2];
    
    newBasis = newBasis.translate(-pe);

    modelview = newBasis * view;

    mvp = modelview * projection;
}

