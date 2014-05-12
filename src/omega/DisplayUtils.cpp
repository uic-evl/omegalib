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
 * What's in this file
 *	Utility geometry methods for display systems like
 * 	- ray > pointer conversion
 *  - pointer > ray conversion
 ******************************************************************************/
#include "omega/DisplayUtils.h"
#include "omega/Camera.h"
#include "omega/Engine.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
Ray DisplayUtils::getViewRay(Vector2i position, const DisplayConfig& cfg)
{
    int channelWidth = cfg.tileResolution[0];
    int channelHeight = cfg.tileResolution[1];
    int displayWidth = cfg.canvasPixelSize[0];
    int displayHeight = cfg.canvasPixelSize[1];

    // NOTE: this is needed for porthole headles mode server, since we can't really generate
    // a ray the way the code is implemented now. At least this makes the call exit correctly
    // (otherwise we get an out of boinds addressing the tile grid).
    if(position[0] < 0 || position[0] > displayWidth || 
    	position[1] < 0 || position[1] > displayHeight)
    {
    //	ofwarn("EqualizerDisplaySystem::getViewRay: position out of bounds (%1%)", %position);
    	return Ray();
    }

    int channelX = position[0] / channelWidth;
    int channelY = position[1] / channelHeight;

    int x = position[0] % channelWidth;
    int y = position[1] % channelHeight;

    DisplayTileConfig* dtc = cfg.tileGrid[channelX][channelY];
    if(dtc != NULL && !dtc->disableMouse)
    {
        // We found a tile in the grid that contains this mouse pointer event, and the tile mouse processing is active.
        return getViewRay(Vector2i(x, y), dtc);
    }

    // No luck using the grid: go with the slower but generic method. Loop through the tiles until you find one that
    // contains the pointer event.
    typedef std::pair<String, DisplayTileConfig*> TileItem;
    foreach(TileItem ti, cfg.tiles)
    {
        if(!ti.second->disableMouse)
        {
            if(position[0] > ti.second->offset[0] &&
                position[1] > ti.second->offset[1] &&
                position[0] < ti.second->offset[0] + ti.second->pixelSize[0] &&
                position[1] < ti.second->offset[1] + ti.second->pixelSize[1])
            {
                Vector2i pos = position - ti.second->offset;
                return getViewRay(pos, ti.second);
            }
        }
    }

    // Suitable tile to process mouse pointer not found. return empty ray.
    return Ray();
}

///////////////////////////////////////////////////////////////////////////////
Ray DisplayUtils::getViewRay(Vector2i position, DisplayTileConfig* dtc)
{
    int x = position[0];
    int y = position[1];

    // Try to use the camera attached to the tile first. If the camera is not set, switch to the default camera.
    Camera* camera = dtc->camera;
    if(camera == NULL)
    {
        camera = Engine::instance()->getDefaultCamera();
    }

    // If the camera is still null, we may be running this code during initialization (before full
    // tile data as been set up). Just print a warning and return an empty ray.
    if(camera == NULL)
    {
        owarn("EqualizerDisplaySystem::getViewRay: null camera, returning default ray.");
        return Ray();
    }

    Vector3f head = camera->getHeadOffset();

    float px = (float)x / dtc->pixelSize[0];
    float py = 1 - (float)y / dtc->pixelSize[1];

    Vector3f& vb = dtc->bottomLeft;
    Vector3f& va = dtc->topLeft;
    Vector3f& vc = dtc->bottomRight;

    Vector3f vba = va - vb;
    Vector3f vbc = vc - vb;

    Vector3f p = vb + vba * py + vbc * px;
    Vector3f direction = p - head;
    
    p = camera->getOrientation() * p;
    p += camera->getPosition();
    
    direction = camera->getOrientation() * direction;
    direction.normalize();

    //ofmsg("channel: %1%,%2% pixel:%3%,%4% pos: %5% dir %6%", %channelX %channelY %x %y %p %direction);

    return Ray(p, direction);
}

///////////////////////////////////////////////////////////////////////////////
bool DisplayUtils::getViewRayFromEvent(const Event& evt, Ray& ray, const DisplayConfig& cfg, bool normalizedPointerCoords, Camera* camera)
{
    if(evt.getServiceType() == Service::Wand)
    {
        // If no camera is passed to this method, use the default camera.
        if(!camera)
        {
            camera = Engine::instance()->getDefaultCamera();
        }
        ray.setOrigin(camera->localToWorldPosition(evt.getPosition()));
        ray.setDirection(camera->localToWorldOrientation(evt.getOrientation()) * -Vector3f::UnitZ());

        return true;
    }
    else if(evt.getServiceType() == Service::Pointer)
    {
        // If the pointer already contains ray information just return it.
        if(evt.getExtraDataType() == Event::ExtraDataVector3Array &&
            evt.getExtraDataItems() >= 2)
        {
            ray.setOrigin(evt.getExtraDataVector3(0));
            ray.setDirection(evt.getExtraDataVector3(1));
        }
        else
        {
            Vector3f pos = evt.getPosition();
            // The pointer did not contain ray information: generate a ray now.
            if(normalizedPointerCoords)
            {
                pos[0] = pos[0] *  cfg.canvasPixelSize[0];
                pos[1] = pos[1] *  cfg.canvasPixelSize[1];
            }

            ray = getViewRay(Vector2i(pos[0], pos[1]), cfg);
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
std::pair<bool, Vector2f> DisplayUtils::getDisplayPointFromViewRay(const Ray& ray, const DisplayConfig& cfg, bool normalizedPointerCoords)
{
    typedef std::pair<bool, Vector2f> Result;

    if(cfg.rayToPointConverter != NULL)
    {
        Result res = cfg.rayToPointConverter->getPointFromRay(ray);

        // If needed, convert from normalized to pixel coordinates.
        if(res.first && !normalizedPointerCoords)
        {
            res.second[0] *= cfg.canvasPixelSize[0];
            res.second[1] *= cfg.canvasPixelSize[1];
        }
    }

    return Result(false, Vector2f::Zero());
}
