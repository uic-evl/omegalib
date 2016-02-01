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
#ifndef __DISPLAY_CONFIG__
#define __DISPLAY_CONFIG__

#include "osystem.h"
#include "SystemManager.h"
#include "DisplayTileConfig.h"

namespace omega
{
    // Forward decl used in DisplayTileConfig
    class DisplayConfig;

    ///////////////////////////////////////////////////////////////////////////
    //! Interface for display configuration generators
    class DisplayConfigBuilder: public ReferenceType
    {
    public:
        virtual bool buildConfig(DisplayConfig& cfg, Setting& scfg) = 0;
        //! Called when the application active canvas changes
        virtual void onCanvasChange(DisplayConfig& cfg) {}
    };
    
    ///////////////////////////////////////////////////////////////////////////
    struct DisplayNodeConfig
    {
        static const int MaxNodeTiles = 64;
        //! When set to false, this node will not be started
        bool enabled;
        int numTiles;
        String hostname;
        int port;
        bool isRemote;
        DisplayTileConfig* tiles[MaxNodeTiles];
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Listener for canvas changes, register using DisplayConfig::setCanvasListener
    class ICanvasListener
    {
    public:
        //! Called when the canvas (ie the application global viewport) changes
        virtual void onCanvasChange() = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Stores omegalib display configuration data.
    class OMEGA_API DisplayConfig : public ReferenceType
    {
        friend class DisplaySystem;
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
        //! Enables tiles based on their name. Accepts a string of space-separated
        //! tile names. Any tile not passed in the string will be disabled.
        void setTilesEnabled(const String& tiles);

        //! Returns the position in real-world coordinates of the specified
        //! display pixel. 
        //! @return a pair <bool, Vector3f>. The boolean is set to true only if
        //! a point for the corresponding pixel could be found 
        //! (i.e. if the pixel is within the bounds of the display area)
        std::pair<bool, Vector3f> getPixelPosition(int x, int y);

        DisplayTileConfig* getTileFromPixel(int x, int y);

        //! Returns true if this configuration has a ray-point mapper
        //! function for the display geometry. Ray-point mapper functions
        //! are used to speed-up ray to display surface intersections, by
        //! using an ideal representation of the display geometry.
        bool hasRayPointMapper() { return !rayPointMapper.isNull(); }

        //! Computes the intersection of a ray with the display geometry. 
        //! @returns a display surface point in normalized coordinates, or
        //! (-1, -1) if no intersection was found.
        Vector2f rayToPoint(const Ray& r);

        //! Returns a view ray given a global (canvas) pointer position in pixel coordinates
        Ray getViewRay(Vector2i position);
        //! Returns a view ray given a local pointer positon and a tile index.
        Ray	getViewRay(Vector2i position, DisplayTileConfig* dtc);
        //! Computes a view ray from a pointer or wand event. Returns true if the ray has been generated succesfully, 
        //! false otherwise (i.e. because the event is not a wand or pointer event)
        bool getViewRayFromEvent(const Event& evt, Ray& ray, bool normalizedPointerCoords = false, Camera* = NULL);

        //! Gts/sets the canvas minimum and maximum boundaries.
        //! Normally, the minimum canvas point is (0,0) but in some settings
        //! (i.e. offset workspaces) the canvas starting point may be different.
        //! The canvas pixel rect is updated by the updateCanvasPixelSize method.
        const Rect& getCanvasRect() const;
        void setCanvasRect(const Rect& cr);
        //! Make sure the canvas for this display configuration is on top of
        //! any other application.
        void bringToFront() { _bringToFrontRequested = true; }
        //! Returns true if a bring to front has been requested in this frame.
        bool isBringToFrontRequested() { return _bringToFrontRequested; }

    public:
        // UGLY CONSTANTS.
        static const int MaxNodes = 64;
        
        DisplayConfig(): 
            disableConfigGenerator(false), latency(1), 
            enableSwapSync(true), forceMono(false), verbose(false),
            invertStereo(false),
            // At startup, request all active tile windows to be brought to front.
            _bringToFrontRequested(true),
            canvasListener(NULL),
            computeEyePosition(&DisplayConfig::defaultComputeEyePosition),
            canvasPosition(Vector3f::Zero()),
            canvasOrientation(Quaternion::Identity()),
            canvasScale(Vector3f::Ones()),
            openGLCoreProfile(false)
        {
            memset(tileGrid, 0, sizeof(tileGrid));
        }		
        
        //! When set to true, OpenGL is initialized in forward-compatible core mode.
        //! @version 10.4
        bool openGLCoreProfile;

        //! When set to true, eyes are inverted in stereo mode.
        bool invertStereo;

        bool disableConfigGenerator;

        //! When set to true, the Display system will output additional 
        //! diagnostic messages during startup and shutdown.
        bool verbose;

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

        //! Full display resolution in pixels. Will be calculated during
        //! setup.
        Vector2i displayResolution;

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
        
        
        typedef KeyValue<String, Ref<DisplayTileConfig> > Tile;
        //! Tile configurations.
        Dictionary<String, Ref<DisplayTileConfig> > tiles;

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
        Ref<RayPointMapper> rayPointMapper;

        //! Script command to call when the canvas changes
        String canvasChangedCommand;
        ICanvasListener* canvasListener;

        //! Stores the transformation that converts a default 'full screen' view
        //! into the view used by the current canvas rect. This transform is computed
        //! by configuration builders or canvas listeners to transform an immersive
        //! view and readjust it as the canvas changes. If this node transform is
        //! set to identity, the view will not follow a canvas, and the canvas will
        //! behave as a movable 2D window into the VR world.
        //! @remarks this value is used in DrawContext::updateTransforms
        //! We need to use a SceneNode here instead of a simple node because we
        //! still need to forward updateTraversals to the camera, and updateTraversal
        //! is implemented in SceneNode
        Vector3f canvasPosition;
        Quaternion canvasOrientation;
        Vector3f canvasScale;

        //! Function used to convert head-space eye positions into sensor-space
        //! (real world) eye positions. Used by DrawContext::updateTransforms
        //! Custom config builders can override this with custom implementations.
        //! For instance, the cylindrical config builder ovverides it to add support
        //! for CAVE2 panoptic stereo mode.
        Vector3f(*computeEyePosition)(
            const Vector3f headSpaceEyePosition, 
            const AffineTransform3& headTransform,
            const DrawContext& dc);

    private:
        //! default computeEyePosition implementation
        static Vector3f defaultComputeEyePosition(
            const Vector3f headSpaceEyePosition, 
            const AffineTransform3& headTransform,
            const DrawContext& dc);

        Rect _canvasRect;
        bool _bringToFrontRequested;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline const Rect& DisplayConfig::getCanvasRect() const
    {
        return _canvasRect;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline Vector2f DisplayConfig::rayToPoint(const Ray& r)
    {
        if(rayPointMapper != NULL) return rayPointMapper->getPointFromRay(r);
        return Vector2f(-1, -1);
    }

}; // namespace omega

#endif
