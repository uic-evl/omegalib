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
 *	The Camera class: handles information about a view transformation, head 
 *	tracking and optional target buffers for off screen rendering
 *	A camera can have a controller that is used to implement a navigation 
 *	technique.
 ******************************************************************************/
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/SceneNode.h"
#include "omega/RenderTarget.h"
#include "omega/CameraOutput.h"

namespace omega {
    //! Id to be assigned to crated cameras
    static int CamerasCounter = 0;

    class CameraController;
    class Camera;
    ///////////////////////////////////////////////////////////////////////////
    //! Implements a listener that can be attached to cameras to listen to draw
    //! methods. All user method implementations must be reentrant, since they
    //! can be called from mulitple threads.
    class ICameraListener
    {
    public:
        virtual void endDraw(Camera* cam, DrawContext& context) {}
        virtual void beginDraw(Camera* cam, DrawContext& context) { }
        virtual void startFrame(Camera* cam, const FrameInfo& frame) {}
        virtual void finishFrame(Camera* cam, const FrameInfo& frame) {}
    };

    ///////////////////////////////////////////////////////////////////////////
    //!	The Camera class handles information about a view transformation, head 
    //!	tracking and optional target buffers for off screen rendering
    //! A camera can have a controller that is used to implement a navigation 
    //! technique.
    class OMEGA_API Camera: public SceneNode
    {
    public:
        enum ViewMode
        {
            //! In immersive view mode, the scene does not follow view movement.
            Immersive,
            //! In classic view mode, the scene follows the view movement and size.
            Classic
        };

        enum CameraFlags
        {
            DrawScene = 1 << 1,
            DrawOverlay = 1 << 2,
            DefaultFlags = DrawScene | DrawOverlay
        };

        enum RenderPassMasks
        {
            DefaultRenderPassMask = 0x00000000,
            AllRenderPassMask = 0xffffffff
        };

    public:
        Camera(Engine* engine, uint flags = DefaultFlags);

        CameraOutput* getOutput(uint contextId);

        //! Returns a custom tile configuration for secondary cameras 
        //! that do no use default display tiles during rendering.
        DisplayTileConfig* getCustomTileConfig();

        void setup(Setting& s);
        void handleEvent(const Event& evt);

        void setPitchYawRoll(const Vector3f& yawPitchRoll);
                
        const AffineTransform3& getViewTransform();

        //void setProjection(float fov, float aspect, float nearZ, float farZ);
        void setNearFarZ(float near, float far);
        float getNearZ();
        float getFarZ();

        //! Returns a view ray given an origin point in normalized coordinates.
        //! @param normalizedPoint - the origin point for the ray in normalized ([0, 1]) coordinates
        //Ray getViewRay(const Vector2f& normalizedPoint);

        //bool getAutoAspect();
        //void setAutoAspect(bool value);

        //! Camera flags
        //@{
        //! When set to true, will draw all 3D scene render passes for 
        //! this camera. Set to true by default.
        void setSceneEnabled(bool value);
        bool isSceneEnabled();
        //! When set to true, will draw all 2D overlay render passes for 
        //! this camera. Set to true by default.
        void setOverlayEnabled(bool value);
        bool isOverlayEnabled();
        //@}

        //! Navigation management
        //@{
        void setController(CameraController* value);
        CameraController* getController() { return myController; }
        bool isControllerEnabled() { return myController != NULL && myControllerEnabled; }
        void setControllerEnabled(bool value) { myControllerEnabled = value; }
        //@}

        void focusOn(SceneNode* node);
        virtual void lookAt(const Vector3f& position, const Vector3f& upVector);

        //! Returns true if this camera is enabled in the specified draw context.
        bool isEnabledInContext(const DrawContext& context);
        bool isEnabledInContext(DrawContext::Task task, const DisplayTileConfig* tile);
        //! Returns true if this camera view area overlaps the specified tile.
        bool overlapsTile(const DisplayTileConfig* tile, const Vector2i& canvasSize);
        //! Set the camera enabled flag. If a camera is disabled it will never
        //! render. If it's enabled it will still be checked agains the active
        //! draw context.
        void setEnabled(bool value);
        bool isEnabled();

        //! Observer control
        //@{
        void setHeadOffset(const Vector3f& value) { myHeadOffset = value; }
        void setHeadOrientation(const Quaternion& value) { myHeadOrientation = value; }
        const Vector3f& getHeadOffset() { return myHeadOffset; }
        const Quaternion& getHeadOrientation() { return myHeadOrientation; }
        const AffineTransform3& getHeadTransform();
        bool isTrackingEnabled() { return myTrackingEnabled; }
        void setTrackingEnabled(bool value) { myTrackingEnabled = value; }
        int getTrackerSourceId() { return myTrackerSourceId; }
        void setTrackerSourceId(int value) { myTrackerSourceId = value; }
        //! Set eye separation for stereo rendering
        void setEyeSeparation(float value) { myEyeSeparation = value; }
        float getEyeSeparation() { return myEyeSeparation; }
        //@}

        //! Converts a point from local to world coordinates using the camera position and orientation
        Vector3f localToWorldPosition(const Vector3f& position);
        //! converts an orientation to the world reference frame using the camera orientation
        Quaternion localToWorldOrientation(const Quaternion& orientation);

        //! Converts a point from world to local coordinates using the camera position and orientation
        Vector3f worldToLocalPosition(const Vector3f& position);

        void clear(DrawContext& context);
        void endDraw(DrawContext& context);
        void beginDraw(DrawContext& context);
        void startFrame(const FrameInfo& frame);
        void finishFrame(const FrameInfo& frame);

        int getCameraId();

        void setMask(uint mask) { myMask = mask; }
        uint getMask() { return myMask; }

        //! Gets or sets the camera listener. Currently, only one listener is 
        //! supported. Setting an additional listener will replace the current one.
        void addListener(ICameraListener* listener);
        void removeListener(ICameraListener* listener);

        //! View management
        //@{
        //! Gets the position of the view generated by this camera on the global
        //! canvas, in normalized coordinates. Default is (0,0)
        const Vector2f& getViewPosition() { return myViewPosition; }
        void setViewPosition(float x, float y);
        //! Gets the size of the view generated by this camera on the global
        //! canvas, in normalized coordinates. Default is (1,1)
        const Vector2f& getViewSize() { return myViewSize; }
        void setViewSize(float x, float y);
        void setReferenceView(float x, float y, float width, float height);
        //! Gets or sets the view mode
        void setViewMode(ViewMode mode);
        ViewMode getViewMode();
        //! Convenience function: returns the pixel rectangle corresponding
        //! to the current camera view.
        int getPixelViewX();
        int getPixelViewY();
        int getPixelViewWidth();
        int getPixelViewHeight();
        //@}

        //! Frame buffer clear
        //@{
        const Color& getBackgroundColor() { return myBackgroundColor; }
        void setBackgroundColor(const Color& value) { myBackgroundColor = value; }
        void clearColor(bool enabled) { myClearColor = enabled; }
        bool isClearColorEnabled() { return myClearColor; }
        void clearDepth(bool enabled) { myClearDepth = enabled; }
        bool isClearDepthEnabled() { return myClearDepth; }
        //@}

    protected:
        void updateTraversal(const UpdateContext& context);
        //! Updates the specified draw context, computing an 
        //! off-axis projection based on the tile and active eye 
        //! in the draw context. Used by beginDraw.
        void updateTransforms(DrawContext& ctx);
        //! Recomputed the view bounds for the current tile, updating the 
        //! viewMin and viewMax values in the context structure.
        void updateViewBounds(DrawContext& ctx, const Vector2i& canvasSize);

        void updateImmersiveViewTransform();
    
    private:
        // Camera flags, used to set a few binary draw options.
        uint myFlags;

        //! A custom tile configuration for secondary cameras that do no use
        //! default display tiles during rendering.
        Ref<DisplayTileConfig> myCustomTileConfig;

        //! View transform
        AffineTransform3 myViewTransform;

        //! Observer head offset (wrt camera position).
        Vector3f myHeadOffset;
        //! Observer head orientation (wrt camera position).
        Quaternion myHeadOrientation;
        //! Observer head transform
        AffineTransform3 myHeadTransform;

        //! Eye separation
        float myEyeSeparation;
        
        //! Tracking stuff
        bool myTrackingEnabled;
        int myTrackerSourceId;

        //Transform3 myProjection;

        //! Field of view (in radians)
        //float myFov;
        //float myAspect;
        float myNearZ;
        float myFarZ;
        //! When set to true, the aspect is computed depending on the height & width of the camera render target.
        bool myAutoAspect;

        // Offscren rendering stuff
        Ref<CameraOutput> myOutput[GpuContext::MaxContexts];
        //DrawContext myDrawContext[GpuContext::MaxContexts];

        // Navigation stuff.
        //Ref<CameraController> myController;
        // Changed to normal pointer to break include loop Camera>CameraController>EngineModule>Engine>Camera
        CameraController* myController;

        bool myControllerEnabled;

        // Camera Id
        int myCameraId;

        uint myMask;

        bool myEnabled;

        // Camera listener. Right now only one listener is supported.
        ICameraListener* myListener;

        Color myBackgroundColor;
        bool myClearDepth;
        bool myClearColor;

        // View stuff
        Vector2f myViewPosition;
        Vector2f myViewSize;
        Vector2f myReferenceViewPosition;
        Vector2f myReferenceViewSize;
        ViewMode myViewMode;
        AffineTransform3 myImmersiveViewTransform;
    };

    ///////////////////////////////////////////////////////////////////////////
    //inline bool Camera::getAutoAspect()
    //{ return myAutoAspect; }

    /////////////////////////////////////////////////////////////////////////////
    //inline void Camera::setAutoAspect(bool value)
    //{ myAutoAspect = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline DisplayTileConfig* Camera::getCustomTileConfig()
    { return myCustomTileConfig; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setPitchYawRoll(const Vector3f& pitchYawRoll) 
    { 
        SceneNode::setOrientation(Math::quaternionFromEuler(pitchYawRoll));
    }

    ///////////////////////////////////////////////////////////////////////////
    inline const AffineTransform3& Camera::getHeadTransform()
    { return myHeadTransform;	}

    ///////////////////////////////////////////////////////////////////////////
    inline const AffineTransform3& Camera::getViewTransform()
    { return myViewTransform; }

    ///////////////////////////////////////////////////////////////////////////
    inline int Camera::getCameraId()
    { return myCameraId; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::addListener(ICameraListener* listener)
    { myListener = listener; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::removeListener(ICameraListener* listener)
    { myListener = NULL; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setEnabled(bool value)
    { myEnabled = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setNearFarZ(float nr, float fr)
    { myNearZ = nr; myFarZ = fr; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Camera::getNearZ()
    { return myNearZ; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Camera::getFarZ()
    { return myFarZ; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setViewPosition(float x, float y) 
    {
        myViewPosition = Vector2f(x, y); if(myViewMode == Immersive) updateImmersiveViewTransform();
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setViewSize(float x, float y) 
    {
        myViewSize = Vector2f(x, y); if(myViewMode == Immersive) updateImmersiveViewTransform(); 
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setReferenceView(float x, float y, float width, float height)
    {
        myReferenceViewPosition = Vector2f(x, y);
        myReferenceViewSize = Vector2f(width, height);
        if(myViewMode == Immersive) updateImmersiveViewTransform();
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Camera::isEnabled()
    { return myEnabled; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setSceneEnabled(bool value)
    { if(value) myFlags |= DrawScene; else myFlags &= ~DrawScene; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Camera::isSceneEnabled()
    { return myFlags & DrawScene; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setOverlayEnabled(bool value)
    { if(value) myFlags |= DrawOverlay; else myFlags &= ~DrawOverlay; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Camera::isOverlayEnabled()
    { return myFlags & DrawOverlay; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setViewMode(ViewMode mode)
    { myViewMode = mode; }

    ///////////////////////////////////////////////////////////////////////////
    inline Camera::ViewMode Camera::getViewMode()
    { return myViewMode; }

}; // namespace omega

#endif