/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *  Koosha Mirhosseini		koosha.mirhosseini@gmail.com
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
#ifndef __DISPLAY_TILE_CONFIG__
#define __DISPLAY_TILE_CONFIG__

#include "osystem.h"
#include "SystemManager.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    // Forward declarations
    class SystemManager;
    class DisplaySystem;
    class RenderTarget;
    class ApplicationBase;
    class ChannelImpl;
    class GpuContext;
    class Renderer;
    class Camera;
    struct DrawContext;

    // Forward decl used in DisplayTileConfig
    class DisplayConfig;
    
    // Forward decl so we can add pointer in DisplayTileConfig;
    struct DisplayNodeConfig;

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API DisplayTileConfig: public ReferenceType
    {
    public:
      enum StereoMode { Mono, LineInterleaved, ColumnInterleaved, PixelInterleaved, SideBySide, Quad, Default };
      enum CorrectionMode { Passthru, EdgeBlendCorrection, WarpCorrection, PreWarpEdgeBlendCorrection, PostWarpEdgeBlendCorrection };

      DisplayTileConfig(DisplayConfig& dc) :
          displayConfig(dc),
            drawStats(false), 
            disableScene(false), 
            disableOverlay(false), 
            stereoMode(Mono),
            correctionMode(Passthru),
            flipWarpMesh(false),
            enabled(false),
            camera(NULL),
            id(0),
            flags(0),
            invertStereo(false),
            gridX(0),
            gridY(0),
            isInGrid(false),
            isHMD(false),
            settingData(NULL),
            pixelSize(Vector2i::Zero()),
            offset(Vector2i::Zero()),
            position(Vector2i::Zero()),
            node(NULL),
            device(0),
            yaw(0.0f),
            pitch(0.0f)
        {
            // EMPTY!
        }

        DisplayConfig& displayConfig;
        
        //! The node owning this tile. This is non-null for physical tiles. 
        //! For logical tiles not owned by any node, this value will be null.
        DisplayNodeConfig* node;

        //! Parse a configuration from a setting, using values from the display
        //! config defaults when needed.
        void parseConfig(const Setting& sTile, DisplayConfig& cfg);
        //! Computes the corner positions for the specified tile using 
        //! information stored in the tile and configuration like center, yaw 
        //! and pitch, lcd size and so on.
        void computeTileCorners();

        //! Stores the tile setting unparsed data. Useful to allow user code
        //! process additional custom options.
        const Setting* settingData;

        StereoMode stereoMode;
        CorrectionMode correctionMode;

        //! When set to true, eyes are inverted in stereo mode.
        bool invertStereo;

        String name;
        int id;

        //! The X position of this tile in the tile grid. Set by display 
        //! configurations that generate 2D tile grids.
        int gridX;
        //! The Y position of this tile in the tile grid. Set by display 
        //! configurations that generate 2D tile grids.
        int gridY;
        //! When set to true, this tile is part of a 2D tile grid.
        bool isInGrid;

        //Vector2i index;
        //Vector2i resolution;
        Vector2i pixelSize;

        //! 2d offset of window content
        Vector2i offset;

        //! Window position
        Vector2i position;

        //! The active region of this tile (i.e. the pixel tile rect where 
        //! rendering is taking place). The active rect is influenced by the 
        //! current view and may is used to determine the actual OS window
        //! position and size.
        Rect activeRect;
        //! Same information as active rect but in canvas coordinates
        //! I.e. this is the area occupied by this tile within the current canvas.
        //! So, activeCanvasRect(0,0,10,10) is a 10x10 rect at the top-left corner
        //! of the canvas, regardless of canvas position on the display.
        //! activeCanvasRect is used in various Camera, 2D and ui calculations.
        Rect activeCanvasRect;
        //! Updates this tile active rect and active canvas rect based on the global pixel viewport
        void updateActiveRect(const Rect& canvasPixelrRect);

        //! 2d position of this tile (normalized) with respect to the global canvas. 
        //! Used for mapping 2d interaction and for mapping physical tiles to logical views.
        //Vector4f viewport;

        //! Field for storing user-defined flags about this tile.
        uint flags;

        int device;
        Vector3f center;
        Vector2f size;
        float yaw;
        float pitch;
        bool drawStats;
        bool disableScene;
        bool disableOverlay;

        // Disable mouse event processing for this tile
        bool disableMouse;

        bool enabled;

        //! When set to true this tile is treated as outputting to a head 
        //! mounted display
        bool isHMD;

        //! When set to true render this tile offscreen.
        bool offscreen;

        //! Disable window borders for this tile only.
        bool borderless;

        //! Flip the y-coordinates and v-coordinates for the warp mesh (to flip the vertical axis to account for OpenGL's default origin) if enabled
        bool flipWarpMesh;

        //! Name of camera attached to this tile. Can be empty or 'default' for default camera
        String cameraName;

        //! Filename containing the warp mesh geometry for this tile. Can be empty if warping is disabled
        String warpMeshFilename;

        //! Filename containing the edge blend texture for this tile. Can be empty if edgeblending is disabled
        String edgeBlendFilename;

        //! Reference to camera attached to this tile. Set during display system initialization
        Camera* camera;

        Vector3f topLeft;
        Vector3f bottomLeft;
        Vector3f bottomRight;

        //! Convenience method to set the tile corners.
        void setCorners(
            const Vector3f& topLeft, 
            const Vector3f& bottomLeft, 
            const Vector3f& bottomRight)
        {
            this->topLeft = topLeft;
            this->bottomLeft = bottomLeft;
            this->bottomRight = bottomRight;
        }

        //! Convenience method to check for intersection between a ray and
        //! this tile. 
        bool rayIntersects(const Ray& ray);

        //! Returns the position in real-world coordinates of the specified
        //! pixel (where pixel 0,0 is in the top-left corner of the tile)
        Vector3f getPixelPosition(int x, int y);

        //! Set the resolution in pixels of this tile. Method used instead of
        // property because python API can't use Vector2i.
        void setPixelSize(int width, int height)
        { pixelSize = Vector2i(width, height); }
    };
}; // namespace omega

#endif
