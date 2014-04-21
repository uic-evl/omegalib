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
 *	Classes used to describe a display system configuration (network, screens, 
 *	system geometry etc.)
 ******************************************************************************/
#ifndef __DISPLAY_CONFIG__
#define __DISPLAY_CONFIG__

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

    // Forward decl used in DisplayTileConfig
    class DisplayConfig;

    ///////////////////////////////////////////////////////////////////////////
    //! Public interface of objects providing a ray to display point conversion
    //! function.
    class IRayToPointConverter
    {
    public:
        //! Returns a 2D point at the intersection between the ray and the
        //! display surface. The 2D point is always in normalized coordinates.
        virtual std::pair<bool, Vector2f> getPointFromRay(const Ray& r) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Interface for display configuration generators
    class DisplayConfigBuilder: public ReferenceType
    {
    public:
        virtual bool buildConfig(DisplayConfig& cfg, Setting& scfg) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API DisplayTileConfig: public ReferenceType
    {
    public:
      enum StereoMode { Mono, LineInterleaved, ColumnInterleaved, PixelInterleaved, SideBySide, Default };

        DisplayTileConfig(): 
            drawStats(false), 
            disableScene(false), 
            disableOverlay(false), 
            stereoMode(Mono),
            enabled(false),
            camera(NULL),
            id(0),
            flags(0),
            invertStereo(false),
            isInGrid(false),
            isHMD(false),
            settingData(NULL),
            offset(Vector2i::Zero()),
            position(Vector2i::Zero())
            {
            }

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

        //! Name of camera attached to this tile. Can be empty or 'default' for default camera
        String cameraName;
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

    ///////////////////////////////////////////////////////////////////////////
    struct DisplayNodeConfig
    {
        static const int MaxNodeTiles = 64;
        int numTiles;
        String hostname;
        int port;
        bool isRemote;
        DisplayTileConfig* tiles[MaxNodeTiles];
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Stores omegalib display configuration data.
    class DisplayConfig: public ReferenceType
    {
    public:
        static void LoadConfig(Setting& s, DisplayConfig& cfg);

        //! Modifies the display configuration to run on the tile subset 
        //! specified in MultiInstanceConfig. This call modifies enabled tiles 
        //! and port assignments in the display configuration and Assings the 
        //! application an instance id. The instance id is written in the id 
        //! field of MultiInstanceConfig and is returned by this call.
        int setupMultiInstance(MultiInstanceConfig* mic);

        //! Returns true if the specified host is running a tile in the specified section. 
        bool isHostInTileSection(const String& hostname, int tilex, int tiley, int tilew, int tileh);

        //! Enables or disables tiles in the specified rectangle. Tiles must
        //! be part of the tile grid.
        void setTilesEnabled(int tilex, int tiley, int tilew, int tileh, bool enabled);

        //! Returns the position in real-world coordinates of the specified
        //! display pixel. 
        //! @return a pair <bool, Vector3f>. The boolean is set to true only if
        //! a point for the corresponding pixel could be found 
        //! (i.e. if the pixel is within the bounds of the display area)
        std::pair<bool, Vector3f> getPixelPosition(int x, int y);

        DisplayTileConfig* getTileFromPixel(int x, int y);

    public:
        // UGLY CONSTANTS.
        static const int MaxNodes = 64;
        
        DisplayConfig(): 
            disableConfigGenerator(false), latency(1), 
            enableSwapSync(true), forceMono(false), verbose(false),
            invertStereo(false),
            rayToPointConverter(NULL)
        {
            memset(tileGrid, 0, sizeof(tileGrid));
        }		

        //! When set to true, eyes are inverted in stereo mode.
        bool invertStereo;

        bool disableConfigGenerator;

        //! When set to true, the Display system will output additional 
        //! diagnostic messages during startup and shutdown.
        bool verbose;

        Vector2i canvasPixelSize;

        //! Display configuration type.
        //String configType;

        //! Number of horizontal / vertical tiles in the display system
        //Vector2i numTiles;
        int latency;

        //! (Used only for planar configurtions) Index of the tile whose center
        //! will be used as the origin of the display system geometry.
        Vector2i referenceTile;
        //! Offset of reference tile center wrt world origin.
        Vector3f referenceOffset;

        //! Size of tile in meters.
        Vector2f tileSize;
        //! Size of tile bezel in meters.
        Vector2f bezelSize;

        //! Tile resolution in pixels.
        Vector2i tileResolution;

        //! When set to true, window positions will be computed automatically 
        //! in a multiwindow setting.
        //bool autoOffsetWindows;
        //! Offset of the first window in pixels (valid for multiwindow settings)
        Vector2i windowOffset;

        //! Global stereo mode. Will be used by tiles that specify 'Default" 
        //! as their stereo mode.
        DisplayTileConfig::StereoMode stereoMode;

        //! Enable vsync on all tiles
        bool enableVSync;
        //! Enable swap sync on cluster displays
        bool enableSwapSync;
             

        //! Enable fullscreen rendering.
        bool fullscreen;

        //! Disable window borders
        bool borderless;

        // Display fps on each tile.
        bool drawFps;

        //! Runtime settings
        //@{
        //! Runtime flag: when set to true, observer orientation will still use camera orientation even
        bool panopticStereoEnabled;
        //! Runtime flag:When set to true, all tiles will be forced to render in mono mode
        bool forceMono;
        //@}
        
        
        typedef KeyValue<String, DisplayTileConfig*> Tile;
        //! Tile configurations.
        Dictionary<String, DisplayTileConfig*> tiles;

        //! Total display resolution. Will be computed automatically during the 
        //! setup process, users should leave this blank.
        //Vector2i displayResolution;
        int numTiles;

        //! Number of nodes for a multimachine display system.
        int numNodes;
        //! Node configurations for a multimachine display system.
        DisplayNodeConfig nodes[MaxNodes];
        //! Interval in milliseconds between node launcher commands
        int launcherInterval; 
        //! Node launcher command.
        String nodeLauncher;
        //! Node killer command.
        String nodeKiller;
        //! Default port used to connect to nodes
        int basePort;

        //! The tile grid is needed for 2d interaction with tiles. and for 
        //! applications running on tile subsets.
        //! Configuration generators fill this up.
        DisplayTileConfig* tileGrid[128][128];
        //! The number of horizontal and vertical tiles in the tile grid.
        //! Configuration generators fill this up together with tileGrid;
        Vector2i tileGridSize;

        Ref<DisplayConfigBuilder> configBuilder;
        IRayToPointConverter* rayToPointConverter;
    };
}; // namespace omega

#endif
