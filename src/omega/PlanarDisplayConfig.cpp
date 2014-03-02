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
#include "omega/PlanarDisplayConfig.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////
bool PlanarDisplayConfig::buildConfig(DisplayConfig& cfg, Setting& scfg)
{
	bool autoOffsetWindows = Config::getBoolValue("autoOffsetWindows", scfg);

	Vector2i numTiles = Config::getVector2iValue("numTiles", scfg);
	cfg.tileGridSize = numTiles;

	cfg.canvasPixelSize = numTiles.cwiseProduct(cfg.tileResolution);
	ofmsg("canvas pixel size: %1%", %cfg.canvasPixelSize);

	Vector2f lcdSize = cfg.tileSize - cfg.bezelSize;
	float tw = lcdSize[0];
	float th = lcdSize[1];

	Vector3f canvasTopLeft = Vector3f(
		-cfg.referenceTile[0] * cfg.tileSize[0] - cfg.tileSize[0] / 2,
		cfg.referenceTile[1] * cfg.tileSize[1] + cfg.tileSize[1] / 2,
		0) + cfg.referenceOffset;

	// Fill up the tile position / orientation data.
	// Compute the edge coordinates for all sides
	for(int x = 0; x < numTiles[0]; x ++)
	{
		for(int y = 0; y < numTiles[1]; y ++)
		{
			// Use the indices to create a tile name in the form t<X>x<Y> (i.e. t1x0).
			// This is kind of hacking, because it forces tile names to be in that form for cylindrical configurations, 
			// but it works well enough.
			String tileName = ostr("t%1%x%2%", %x %y);
			if(cfg.tiles.find(tileName) == cfg.tiles.end())
			{
				ofwarn("PlanarDisplayConfig::buildConfig: could not find tile '%1%'", %tileName);
			}
			else
			{
				DisplayTileConfig* tc = cfg.tiles[tileName];
				cfg.tileGrid[x][y] = tc;

				tc->enabled = true;
				tc->isInGrid = true;
				tc->gridX = x;
				tc->gridY = y;

				// Compute the display corners for a planar display configuration
				tc->topLeft = canvasTopLeft + Vector3f(x * tw, -(y * th), 0);
				tc->bottomLeft = tc->topLeft + Vector3f(0, -th, 0);
				tc->bottomRight = tc->topLeft + Vector3f(tw, -th, 0);

				tc->center = (tc->topLeft + tc->bottomRight) / 2;

				if(autoOffsetWindows)
				{
					int winX = x * cfg.tileResolution[0] + cfg.windowOffset[0];
					int winY = y * cfg.tileResolution[1] + cfg.windowOffset[1];
					tc->position = Vector2i(winX, winY);
				}

				tc->offset.x() = x * cfg.tileResolution[0];
				tc->offset.y() = y * cfg.tileResolution[1];

				// Save the tile viewport
				//tc->viewport = Vector4f(tileViewportX, tileViewportY, tileViewportWidth, tileViewportHeight);

				//tileViewportY += tileViewportHeight;
			}
		}
		//tileViewportY = 0.0f;
		//tileViewportX += tileViewportWidth;
	}

	return true;
}
