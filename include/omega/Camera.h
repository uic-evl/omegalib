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
#include "omega/DrawContext.h"

namespace omega {
    //! Id to be assigned to crated cameras
    static int CamerasCounter = 0;

    class CameraController;
    class Camera;
    ///////////////////////////////////////////////////////////////////////////
    //! Implements a listener that can be attached to cameras to listen to draw
    //! methods. All user method implementations must be reentrant, since they
    //! can be called from mulitple threads.
    class OMEGA_API ICameraListener
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
        enum CameraFlags
        {
            DrawScene = 1 << 1,
            DrawOverlay = 1 << 2,
            CullingEnabled = 1 << 3,
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

        virtual void setup(Setting& s);
        virtual void handleEvent(const Event& evt);

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
        //! When set to false, disables all culling for this camera. 
        //! All drawables will attempt drawing, even the ones that 
        //! are outside of this camera frustum.
        //! This is useful to force drawing of all objects when we want to
        //! use vertex shaders with custom projections.
        //! By default, culling is enabled.
        void setCullingEnabled(bool value);
        bool isCullingEnabled();
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
        virtual bool isEnabledInContext(const DrawContext& context);
        virtual bool isEnabledInContext(DrawContext::Task task, const DisplayTileConfig* tile);
        //! Returns true if this camera view area overlaps the specified tile.
        bool overlapsTile(const DisplayTileConfig* tile);
        //! Set the camera enabled flag. If a camera is disabled it will never
        //! render. If it's enabled it will still be checked agains the active
        //! draw context.
        void setEnabled(bool value);
        //! Returns true if the frame is enabled, false otherwise
        //! @remarks even if the camera is enabled, this method can return
        //! false if on-demand frame drawing is on and the camera is not 
        //! currently scheduled to draw a frame
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
        //! Sets the tracker user id.
        //! @remarks If a tracker user ID is set (!= -1) and tracking is enabled
        //! this camera will ignore the tracker source id, and use any tracker
        //! source with the right user id. The camera controller may also process
        //! imput based on user id instead of source id.
        //! This is useful in supporting dynamic
        //! User-application control.
        int getTrackerUserId() { return myTrackerUserId; }
        void setTrackerUserId(int value) { myTrackerUserId = value; }
        void setEyeSeparation(float value) { myEyeSeparation = value; }
        float getEyeSeparation() { return myEyeSeparation; }
        //@}

        virtual void clear(DrawContext& context);
        virtual void endDraw(DrawContext& context);
        virtual void beginDraw(DrawContext& context);
        virtual void startFrame(const FrameInfo& frame);
        virtual void finishFrame(const FrameInfo& frame);

        int getCameraId();

        void setMask(uint mask) { myMask = mask; }
        uint getMask() { return myMask; }

        //! Gets or sets the camera listener. Currently, only one listener is
        //! supported. Setting an additional listener will replace the current one.
        void addListener(ICameraListener* listener);
        void removeListener(ICameraListener* listener);
        ICameraListener* getListener();

        //! View management
        //@{
        //! Gets the position of the view generated by this camera on the global
        //! canvas, in normalized coordinates. Default is (0,0)
        const Vector2f& getViewPosition() { return myViewPosition; }
        void setViewPosition(float x, float y);
        //! Gets the size of the view generated by this camera on the global
        //! canvas, in normalized coordinates. Default is (1,1)
        const Vector2f& getViewSize() { return myViewSize; }
        void setViewSize(float width, float height);
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

        //! On-demand drawing
        //@{
        //! Queues one frame for drawing. Use this to force a frame
        //! draw when MaxFps is set to 0.
        void queueFrameDraw();
        //! Set the maximum fps that this camera will render at.
        //! Use 0 to stop camera drawing and use queueFrameDraw to
        //! draw frames on-demand.
        //! Use -1 to disable the fps cap and let this camera draw
        //! at the maximum renderer speed (typically 60fps)
        void setMaxFps(float fps);
        float getMaxFps();
        //@}

        //! DEPRECATED
        //@{
        Vector3f localToWorldPosition(const Vector3f& position);
        Quaternion localToWorldOrientation(const Quaternion& orientation);
        Vector3f worldToLocalPosition(const Vector3f& position);
        //@}

        //! Update the canvas transform. Used to support dynamic immersive canvases
        void setCanvasTransform(const Vector3f& position, const Quaternion& orientation, const Vector3f scale);
        const Vector3f& getCanvasPosition() const;
        const Quaternion& getCanvasOrientation() const;
        const Vector3f& getCanvasScale() const;

    protected:
        void updateTraversal(const UpdateContext& context);
        //! Updates the specified draw context, computing an
        //! off-axis projection based on the tile and active eye
        //! in the draw context. Used by beginDraw.
        void updateTransforms(DrawContext& ctx);
        virtual void updateFromParent(void) const;

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
        int myTrackerUserId;

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

        // Canvas transform
        Vector3f myCanvasPosition;
        Quaternion myCanvasOrientation;
        Vector3f myCanvasScale;

        // On-demand drawing
        bool myDrawNextFrame;
        float myMaxFps;
        float myTimeSinceLastFrame;
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
    inline ICameraListener* Camera::getListener()
    { return myListener; }

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
        myViewPosition = Vector2f(x, y);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setViewSize(float x, float y)
    {
        myViewSize = Vector2f(x, y);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Camera::isEnabled()
    { return myEnabled && (myDrawNextFrame || myMaxFps < 0); }

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
    inline void Camera::setCullingEnabled(bool value)
    {
        if(value) myFlags |= CullingEnabled; else myFlags &= ~CullingEnabled;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Camera::isCullingEnabled()
    {
        return myFlags & CullingEnabled;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline const Vector3f& Camera::getCanvasPosition() const
    { return myCanvasPosition; }

    ///////////////////////////////////////////////////////////////////////////
    inline const Quaternion& Camera::getCanvasOrientation() const
    { return myCanvasOrientation; }

    ///////////////////////////////////////////////////////////////////////////
    inline const Vector3f& Camera::getCanvasScale() const
    { return myCanvasScale; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::queueFrameDraw() 
    { myDrawNextFrame = true; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Camera::setMaxFps(float fps)
    { myMaxFps = fps; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Camera::getMaxFps()
    { return myMaxFps; }


}; // namespace omega

#endif
