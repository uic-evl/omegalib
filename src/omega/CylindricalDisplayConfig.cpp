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
 *	A display configuration builder that can be used for cylindrical display
 *  systems like CAVE2.
 ******************************************************************************/
#include "omega/CylindricalDisplayConfig.h"
#include "omega/DrawContext.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
Vector3f CylindricalDisplayConfig::computeEyePosition(
    const Vector3f headSpaceEyePosition,
    const AffineTransform3& headTransform,
    const DrawContext& dc)
{
    Vector3f pe = headSpaceEyePosition;
    // Transform eye with head position / orientation. After this, eye position
    // and tile coordinates are all in the same reference frame.
    if(dc.tile->displayConfig.panopticStereoEnabled)
    {
        // CAVE2 SIMPLIFICATION: We are just interested in adjusting the observer yaw
        AffineTransform3 ht = AffineTransform3::Identity();
        ht.translate(headTransform.translation());
        pe = ht.rotate(
        AngleAxis(-dc.tile->yaw * Math::DegToRad, Vector3f::UnitY())) * pe;
    }
    else
    {
        pe = headTransform * pe;
    }
    return pe;
}

///////////////////////////////////////////////////////////////////////////////
void CylindricalDisplayConfig::onCanvasChange(DisplayConfig& cfg)
{
    // Find pixel coordinates of center of new canvas
    const Rect& c = cfg.getCanvasRect();
    int px = c.x() + c.width() / 2;
    int py = c.y() + c.height() / 2;
    
    // Find the 3D position of that pixel
    std::pair<bool, Vector3f> res = cfg.getPixelPosition(px, py);
    if(res.first)
    {
        // Normalize direction (we don't care about Y)
        float l = sqrt(res.second[0]*res.second[0] + res.second[2]*res.second[2]);
        float x = res.second[0] / l;
        float z = res.second[2] / l;
        
        // compute angle between the two
        float a = acos(-z);
        if(x < 0) a = -a;
        
        // Set the canvas view transform
        cfg.canvasOrientation = AngleAxis(a, Vector3f::UnitY());
    }
}

///////////////////////////////////////////////////////////////////////////////
bool CylindricalDisplayConfig::buildConfig(DisplayConfig& cfg, Setting& scfg)
{
    // Register view and eye position transformation functions
    cfg.computeEyePosition = &CylindricalDisplayConfig::computeEyePosition;

    Vector2i numTiles = Config::getVector2iValue("numTiles", scfg);
    cfg.tileGridSize = numTiles;

    int numSides = numTiles.x();
    
    // Angle increment for each side (column)
    float sideAngleIncrement = Config::getFloatValue("sideAngleIncrement", scfg, 90);

    // Angle of side containing 0-index tiles.
    float sideAngleStart = Config::getFloatValue("sideAngleStart", scfg, -90);

    // Display system cylinder radius
    myRadius = Config::getFloatValue("radius", scfg, 5);

    // Number of vertical tiles in each side
    int numSideTiles = numTiles.y();

    // Offset of center of bottom tile.
    float yOffset = cfg.referenceOffset.y();

    // Save offset of low end of bottom tile (subtract center)
    myYOffset = yOffset - cfg.tileSize.y() / 2;

    // Save cylinder height
    myHeight = numSideTiles * cfg.tileSize.y();

    float tileViewportWidth = 1.0f / numTiles[0];
    float tileViewportHeight = 1.0f / numTiles[1];
    float tileViewportX = 0.0f;
    float tileViewportY = 0.0f;

    // Fill up the tile position / orientation data.
    // Compute the edge coordinates for all sides
    float curAngle = sideAngleStart;
    for(int x = 0; x < numSides; x ++)
    {
        float yPos = yOffset;
        for(int y = 0; y < numSideTiles; y ++)
        {
            // Use the indices to create a tile name in the form t<X>x<Y> (i.e. t1x0).
            // This is kind of hacking, because it forces tile names to be in that form for cylindrical configurations, 
            // but it works well enough.
            String tileName = ostr("t%1%x%2%", %x %y);
            if(cfg.tiles.find(tileName) == cfg.tiles.end())
            {
                ofwarn("CylindricalDisplayConfig::buildConfig: could not find tile '%1%'", %tileName);
            }
            else
            {
                DisplayTileConfig* tc = cfg.tiles[tileName];
                cfg.tileGrid[x][y] = tc;
                
                // 15Nov15 - line commented ut - tiles are enabled
                // by default. if they are set disabld in the configuration, they
                // should stay disabled here. This makes it possible to mark specific
                // tiles disabled even when using a display configuration builder
                // Example use case: mark broken tiles as disabled in a CAVE2 system
                //tc->enabled = true;
                tc->isInGrid = true;
                tc->gridX = x;
                tc->gridY = y;
                
                tc->yaw = curAngle;
                tc->pitch = 0;
                tc->center = Vector3f(
                    sin(curAngle * Math::DegToRad) * myRadius,
                    yPos,
                    -1 * cos(curAngle * Math::DegToRad) * myRadius);

                // Save the tile viewport
                //tc->viewport = Vector4f(tileViewportX, tileViewportY, tileViewportWidth, tileViewportHeight);
                
                // Compute this tile pixel offset.
                // Note that the tile row index is inverted wrt. pixel coordinates 
                // (tile row 0 is at the bottom of the cylinder, while pixel 
                // row 0 is at the top). We take this into account to compute
                // correct pixel offsets for each tile.
                tc->offset[0] = tc->pixelSize[0] * x;
                tc->offset[1] = tc->pixelSize[1] * (numSideTiles - y - 1);

                tc->computeTileCorners();
            }
            yPos += cfg.tileSize.y();
            tileViewportY += tileViewportHeight;
        }
        curAngle += sideAngleIncrement;
        tileViewportY = 0.0f;
        tileViewportX += tileViewportWidth;
    }
    return true;
}
