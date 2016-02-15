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
 *	Classes used to describe a display system configuration (network, screens, 
 *	system geometry etc.)
 ******************************************************************************/
#include "omega/DisplayConfig.h"
#include "omega/Engine.h"
#include "omega/CylindricalDisplayConfig.h"
#include "omega/PlanarDisplayConfig.h"

using namespace omega;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
void DisplayTileConfig::parseConfig(const Setting& sTile, DisplayConfig& cfg)
{
    settingData = &sTile;

    DisplayTileConfig* tc = this;

    tc->name = sTile.getName();

    String sm = Config::getStringValue("stereoMode", sTile, "default");
    StringUtils::toLowerCase(sm);

    if(sm == "default") tc->stereoMode = DisplayTileConfig::Default;
    else if(sm == "mono") tc->stereoMode = DisplayTileConfig::Mono;
        // 'interleaved' defaults to row interleaved
    else if(sm == "interleaved") tc->stereoMode = DisplayTileConfig::LineInterleaved;
    else if(sm == "rowinterleaved") tc->stereoMode = DisplayTileConfig::LineInterleaved;
    else if(sm == "columninterleaved") tc->stereoMode = DisplayTileConfig::ColumnInterleaved;
    else if(sm == "sidebyside") tc->stereoMode = DisplayTileConfig::SideBySide;
    else if(sm == "anaglyphredcyan") tc->stereoMode = DisplayTileConfig::AnaglyphRedCyan;
    else if(sm == "anaglyphgreenmagenta") tc->stereoMode = DisplayTileConfig::AnaglyphGreenMagenta;
                
    tc->invertStereo = Config::getBoolValue("invertStereo", sTile);
    // CHANGE v10.1 - 15Nov15
    // Display tiles are enabled by default.
    tc->enabled = Config::getBoolValue("enabled", sTile, true);
                    
    //tc.index = index;
    //tc.interleaved = Config::getBoolValue("interleaved", sTile);
    tc->device = Config::getIntValue("device", sTile);

    tc->center = Config::getVector3fValue("center", sTile, Vector3f::Zero());
    tc->yaw = Config::getFloatValue("yaw", sTile, 0);
    tc->pitch = Config::getFloatValue("pitch", sTile, 0);

    tc->position = Config::getVector2iValue("position", sTile);
    tc->disableScene = Config::getBoolValue("disableScene", sTile);

    tc->offscreen = Config::getBoolValue("offscreen", sTile, false);
    tc->borderless = Config::getBoolValue("borderless", sTile, cfg.borderless);

    tc->disableMouse = Config::getBoolValue("disableMouse", sTile, false);

    tc->isHMD = Config::getBoolValue("isHMD", sTile, false);

    //tc->viewport = Config::getVector4fValue("viewport", sTile, tc->viewport);

    // If the tile config contains a size entry use it, oterwise use the default tile and bezel size data
    if(sTile.exists("size"))
    {
        tc->size = Config::getVector2fValue("size", sTile);
    }
    else
    {
        tc->size = cfg.tileSize - cfg.bezelSize;
    }

    if(sTile.exists("resolution"))
    {
        tc->pixelSize = Config::getVector2iValue("resolution", sTile);
    }
    else
    {
        tc->pixelSize = cfg.tileResolution;
    }

    if(sTile.exists("offset"))
    {
        tc->offset = Config::getVector2iValue("offset", sTile);
    }
    else
    {
        std::vector<std::string> args = StringUtils::split(String(sTile.getName()), "xt");
        Vector2i index = Vector2i(atoi(args[0].c_str()), atoi(args[1].c_str()));
        tc->offset = index.cwiseProduct(tc->pixelSize);
        cfg.tileGrid[index[0]][index[1]] = tc;
    }

    // Parse custom grid options
    tc->isInGrid = Config::getBoolValue("isInGrid", sTile, true);
    if(tc->isInGrid)
    {
        tc->gridX = Config::getIntValue("gridX", sTile, 0);
        tc->gridY = Config::getIntValue("gridY", sTile, 0);
        cfg.tileGrid[tc->gridX][tc->gridY] = tc;
    }

    // Custom camera
    tc->cameraName = Config::getStringValue("cameraName", sTile, "");

    // Compute default values for this tile corners. These values may be overwritted by display config generators applied later on.
    computeTileCorners();
}

//////////////////////////////////////////////////////////////////////////////
void DisplayTileConfig::computeTileCorners()
{
    DisplayTileConfig* tc = this;

    float tw = tc->size[0];
    float th = tc->size[1];

    // Compute the display corners for custom display geometries
    Quaternion orientation = AngleAxis(tc->yaw * Math::DegToRad, Vector3f::UnitY()) * AngleAxis(-tc->pitch * Math::DegToRad, Vector3f::UnitX());
    // Define the default display up and right vectors
    Vector3f up = Vector3f::UnitY();
    Vector3f right = Vector3f::UnitX();
    
    // Compute the tile corners using the display center and oriented normal.
    up = orientation * up;
    right = orientation * right;

    // Reorient Z.
    right.z() = - right.z();

    tc->topLeft = tc->center + (up * th / 2) - (right * tw / 2);
    tc->bottomLeft = tc->center - (up * th / 2) - (right * tw / 2);
    tc->bottomRight = tc->center - (up * th / 2) + (right * tw / 2);
}

//////////////////////////////////////////////////////////////////////////////
bool DisplayTileConfig::rayIntersects(const Ray& ray)
{
    // Intersect with two triangles defining the tile surface
    Vector3f topRight = topLeft + (bottomRight - bottomLeft);
                
    pair<bool, omicron::real> intersect1 = Math::intersects(ray, 
        topLeft, bottomLeft, bottomRight,
        true, false);
    pair<bool, omicron::real> intersect2 = Math::intersects(ray, 
        topRight, topLeft, bottomRight,
        true, false);
    // If we found an intersection, we are done.
    return intersect1.first || intersect2.first;
}

//////////////////////////////////////////////////////////////////////////////
Vector3f DisplayTileConfig::getPixelPosition(int x, int y)
{
    // Normalize coords
    Vector2f point(x, 1 - y);
    point[0] = point[0] / pixelSize[0];
    point[1] = point[1] / pixelSize[1];

    Vector3f xb = bottomRight - bottomLeft;
    Vector3f yb = topLeft - bottomLeft;

    Vector3f position = topLeft + xb * point[0];
    position += yb * point[1];
    return position;
}

//////////////////////////////////////////////////////////////////////////////
void DisplayTileConfig::updateActiveRect(const Rect& canvasPixelRect)
{
    // If the tile is not part of a tile grid, we have nothing to do here.
    if(!isInGrid) return;
    
    Rect localRect(offset, offset + pixelSize);
    std::pair<bool, Rect> intersection = localRect.getIntersection(canvasPixelRect);

    if(intersection.first)
    {
        oflog(Debug, "[DisplayTileConfig] Tile %1% ON", %name);
        
        enabled = true;
        activeRect = Rect(
            intersection.second.min + position - offset,
            intersection.second.max + position - offset);

        activeCanvasRect.min = intersection.second.min - canvasPixelRect.min;
        activeCanvasRect.max = intersection.second.max - canvasPixelRect.min;
    }
    else
    {
        oflog(Debug, "[DisplayTileConfig] Tile %1% OFF", %name);
        
        enabled = false;
    }
}
