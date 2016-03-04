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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The Camera class: handles information about a view transformation, head
 *	tracking and optional target buffers for off screen rendering
 *	A camera can have a controller that is used to implement a navigation
 *	technique.
 ******************************************************************************/
#include "omega/RenderTarget.h"
#include "omega/Camera.h"
#include "omega/CameraOutput.h"
#include "omega/DisplaySystem.h"
#include "omega/ModuleServices.h"
#include "omega/WandCameraController.h"
#include "omega/GamepadCameraController.h"
#include "omega/MouseCameraController.h"
#include "omega/KeyboardMouseCameraController.h"
#include "omega/glheaders.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////
Camera::Camera(Engine* e, uint flags):
    SceneNode(e),
    myAutoAspect(false),
    myFlags(flags),
    myController(NULL),
    myControllerEnabled(false),
    myTrackingEnabled(false),
    myTrackerSourceId(-1),
    myTrackerUserId(-1),
    myHeadOrientation(Quaternion::Identity()),
    myHeadOffset(Vector3f::Zero()),
    myMask(0),
    myEyeSeparation(0.06f),
    myListener(NULL),
    myNearZ(0.1f),
    myFarZ(1000.0f),
    myViewPosition(0, 0),
    myViewSize(1, 1),
    myEnabled(true),
    myClearColor(true),
    myClearDepth(true),
    myBackgroundColor(Color(0.1f,0.1f, 0.15f, 1)),
    myCanvasOrientation(Quaternion::Identity()),
    myCanvasPosition(Vector3f::Zero()),
    myCanvasScale(Vector3f::Ones()),
    myDrawNextFrame(true),
    myMaxFps(-1),
    myTimeSinceLastFrame(0)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    myCustomTileConfig = new DisplayTileConfig(ds->getDisplayConfig());

    // set camera Id and increment the counter
    this->myCameraId = omega::CamerasCounter++;

    // Culling always enabled by default
    setCullingEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::setup(Setting& s)
{
    //set position of camera
    Vector3f camPos = Config::getVector3fValue("position", s, getPosition());
    setPosition(camPos);

    //set orientation of camera
    // NOTE: we want to either read orientation from the config or keep the default one.
    // Since orientation is expressed in yaw, pitch roll in the config file but there is no
    // way to get that from the camera (rotation is only as a quaternion) we cannot use the default
    // value in the Config::getVector3fValue.
    if(s.exists("orientation"))
    {
        Vector3f camOri = Config::getVector3fValue("orientation", s);
        setPitchYawRoll(camOri * Math::DegToRad);
    }

    myTrackerSourceId = Config::getIntValue("trackerSourceId", s, -1);
    if(myTrackerSourceId != -1) myTrackingEnabled = true;

    myTrackingEnabled = Config::getBoolValue("trackingEnabed", s, myTrackingEnabled);

    //setup camera controller.  The camera needs to be setup before this otherwise its values will be rewritten

    String controllerName;
    controllerName = Config::getStringValue("controller", s);
    StringUtils::toLowerCase(controllerName);

    if(controllerName != "")
    {
        CameraController* controller = NULL;
        //ofmsg("Camera controller: %1%", %controllerName);
        if(controllerName == "keyboardmouse") controller = new KeyboardMouseCameraController();
        if(controllerName == "mouse") controller = new MouseCameraController();
        if(controllerName == "wand") controller = new WandCameraController();
        if(controllerName == "gamepad") controller = new GamepadCameraController();

        setController(controller);
        if(myController != NULL)
        {
            myController->setup(s);
            setControllerEnabled(true);
            bool freeFly = Config::getBoolValue("freeFly", s, false);
            myController->setFreeFlyEnabled(freeFly);
        }
    }

    if(s.exists("headOffset"))
    {
        Setting& st = s["headOffset"];
        myHeadOffset.x() = (float)st[0];
        myHeadOffset.y() = (float)st[1];
        myHeadOffset.z() = (float)st[2];
    }
}

///////////////////////////////////////////////////////////////////////////////
void Camera::handleEvent(const Event& evt)
{
    if(myTrackingEnabled)
    {
        if(evt.getServiceType() == static_cast<enum Service::ServiceType>(Event::ServiceTypeMocap) &&
            ((myTrackerUserId != -1 && evt.getUserId() == myTrackerUserId) ||
                evt.getSourceId() == myTrackerSourceId))
        {
            // By convention (as of omicron 3.0), if this mocap event has int extra data,
            // the first field is a joint id. This will not break with previous versions
            // of omicron, but no joint data will be read here.
            // NOTE: we need this check because multiple trackables may
            // share the same user id. We only want the head.
            if(myTrackerUserId == -1 || (!evt.isExtraDataNull(0) &&
                evt.getExtraDataType() == Event::ExtraDataIntArray &&
                evt.getExtraDataInt(0) == Event::OMICRON_SKEL_HEAD))
            {
                myHeadOffset = evt.getPosition();
                myHeadOrientation = evt.getOrientation();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Camera::updateTraversal(const UpdateContext& context)
{
    // Update the view transform
    myHeadTransform = AffineTransform3::Identity();
    myHeadTransform.translate(myHeadOffset);
    myHeadTransform.rotate(myHeadOrientation);

    // BUG: if we attach a child node to the camera, isUpdateNeeded gets reset at the wrong
    // time and the camera view transform does not get updated.
    // Needs fixing, but for now best solution is to disable the check and always update
    // the view transform
    //if(isUpdateNeeded())
    {
        // Update view transform.
        myViewTransform = Math::makeViewMatrix(
            getDerivedPosition(), // + myHeadOffset,
            getDerivedOrientation());

        AffineTransform3 t = AffineTransform3::Identity();
    }

    // Queue a frame draw if on-demand frame drawing is enabled
    if(myMaxFps > 0)
    {
        myTimeSinceLastFrame += context.dt;
        if(myTimeSinceLastFrame > (1.0f / myMaxFps))
        {
            myTimeSinceLastFrame = 0;
            myDrawNextFrame = true;
        }
    }

    SceneNode::updateTraversal(context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::lookAt(const Vector3f& position, const Vector3f& upVector)
{
    Node::lookAt(position - myHeadOffset, upVector);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::focusOn(SceneNode* node)
{
    // Compute direction vector
    Vector3f dir = mPosition - node->getPosition();
    dir.normalize();

    const Sphere& bs = node->getBoundingSphere();
    ofmsg("Camera:focuson %1% %2%", %bs.getCenter() %bs.getRadius());
    mPosition = bs.getCenter() + Vector3f(0, 0, bs.getRadius() * 2) - myHeadOffset;
    lookAt(node->getPosition(), Vector3f::UnitY());
}

///////////////////////////////////////////////////////////////////////////////
CameraOutput* Camera::getOutput(uint contextId)
{
    oassert(contextId < GpuContext::MaxContexts);
    // Camera outputs are created on-demand here.
    if(myOutput[contextId] == NULL)
    {
        ofmsg("Camera::getOutput: creating camera output for context %1%", %contextId);
        myOutput[contextId] = new CameraOutput();
    }

    return myOutput[contextId].get();
}


///////////////////////////////////////////////////////////////////////////////
bool Camera::isEnabledInContext(const DrawContext& context)
{
    return isEnabledInContext(context.task, context.tile);
}

///////////////////////////////////////////////////////////////////////////////
bool Camera::isEnabledInContext(DrawContext::Task task, const DisplayTileConfig* tile)
{
    // If the camera is not enabled always return false.
    if(!isEnabled()) return false;

    // If the camera is not enabled for the current task, return false.
    if((task == DrawContext::SceneDrawTask &&
        !(myFlags & DrawScene)) ||
        (task == DrawContext::OverlayDrawTask &&
        !(myFlags & DrawOverlay))) return false;

    return overlapsTile(tile);
}

///////////////////////////////////////////////////////////////////////////////
bool Camera::overlapsTile(const DisplayTileConfig* tile)
{
    const Rect& cr = tile->displayConfig.getCanvasRect();
    Vector2f& vp = myViewPosition;
    Vector2f& vs = myViewSize;

    // View rect contains the camera view rectangle in pixel coordinates.
    Rect viewRect((int)(vp[0] * cr.width()), (int)(vp[1] * cr.height()),
        (int)(vs[0] * cr.width()), (int)(vs[1] * cr.height()));

    return viewRect.intersects(tile->activeCanvasRect);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::beginDraw(DrawContext& context)
{
    //ofmsg("  Camera %1% beginDraw", %getName());
    context.camera = this;

    //Vector2i canvasSize;
    if(myCustomTileConfig->enabled)
    {
        context.pushTileConfig(myCustomTileConfig);
    }

    // updateViewport will set up the viewport for side-by-side stereo modes.
    // Note that custom camera viewports and side-by-side stereo do not work
    // together yet. If side-by-side stereo is enabled, it will override camera
    // viewport settings.
    context.updateViewport();

    context.setupInterleaver();
    context.updateTransforms(
        myHeadTransform, myViewTransform,
        myEyeSeparation,
        myNearZ, myFarZ);

    CameraOutput* output = myOutput[context.gpuContext->getId()];
    if(output != NULL && output->isEnabled())
    {
        output->beginDraw(context);
        // Adjust the context viewport based on the output viewport.
        context.viewport = output->getReadbackViewport();
    }

    if(myListener != NULL) myListener->beginDraw(this, context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::endDraw(DrawContext& context)
{
    //ofmsg("  Camera %1% endDraw", %getName());
    CameraOutput* output = myOutput[context.gpuContext->getId()];
    if(output != NULL && output->isEnabled())
    {
        output->endDraw(context);
    }
    if(myCustomTileConfig->enabled)
    {
        context.popTileConfig();
    }
    if(myListener != NULL) myListener->endDraw(this, context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::startFrame(const FrameInfo& frame)
{
    //ofmsg("  Camera %1% startFrame", %getName());

    CameraOutput* output = myOutput[frame.gpuContext->getId()];
    if(output != NULL && output->isEnabled())
    {
        output->startFrame(frame);
    }

    if(myListener != NULL) myListener->startFrame(this, frame);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::finishFrame(const FrameInfo& frame)
{
    //ofmsg("  Camera %1% finishFrame", %getName());

    CameraOutput* output = myOutput[frame.gpuContext->getId()];
    if(output != NULL && output->isEnabled())
    {
        output->finishFrame(frame);
    }
    if(myListener != NULL) myListener->finishFrame(this, frame);

    // Reset on-demand draw frame flag.
    myDrawNextFrame = false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::clear(DrawContext& context)
{
    //ofmsg("  Camera %1% clear", %getName());
    if(myClearColor || myClearDepth)
    {
        RenderTarget* rt = NULL;
        CameraOutput* output = myOutput[context.gpuContext->getId()];
        if(output != NULL && output->isEnabled()) rt = output->getRenderTarget();

        context.camera = this;
        if(myCustomTileConfig->enabled)
        {
            // If we are using a camera output AND a custom tile configuration,
            // the tile pixel size is automatically set to the render target size.
            if(output != NULL && output->isEnabled())
            {
                myCustomTileConfig->pixelSize = output->getReadbackViewport().size();
            }
            context.pushTileConfig(myCustomTileConfig);
        }

        // If camera view is not overlayed to current tile, return.
        if(!overlapsTile(context.tile)) return;

        // Update view bounds THEN update the viewport.
        // updateViewport will set up the viewport for side-by-side stereo modes.
        // Note that custom camera viewports and side-by-side stereo do not work
        // together yet. If side-by-side stereo is enabled, it will override camera
        // viewport settings.
        context.updateViewport();
        
        Rect sr = context.drawInterface->getScissor();
        context.drawInterface->setScissor(context.viewport);

        if(rt != NULL) rt->bind();

        if(myClearColor)
        {
            // clear the depth and color buffers.
            const Color& b = myBackgroundColor;
            glClearColor(b[0], b[1], b[2], b[3]);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if(myClearDepth)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        if(myCustomTileConfig->enabled)
        {
            context.popTileConfig();
        }

        if(rt != NULL) rt->unbind();

        context.drawInterface->setScissor(sr);
    }
}

///////////////////////////////////////////////////////////////////////////////
Vector3f Camera::localToWorldPosition(const Vector3f& position)
{
    owarn("DEPRECATION WARNING: Camera::localToWorldPosition is deprecated.");
    owarn("use SceneNode::convertLocalToWorldPosition instead.");
    return convertLocalToWorldPosition(position);
}

///////////////////////////////////////////////////////////////////////////////
Quaternion Camera::localToWorldOrientation(const Quaternion& orientation)
{
    owarn("DEPRECATION WARNING: Camera::localToWorldOrientation is deprecated.");
    owarn("use SceneNode::convertLocalToWorldOrientation instead.");
    return convertLocalToWorldOrientation(orientation);
}

///////////////////////////////////////////////////////////////////////////////
Vector3f Camera::worldToLocalPosition(const Vector3f& position)
{
    owarn("DEPRECATION WARNING: Camera::worldToLocalPosition is deprecated.");
    owarn("use SceneNode::convertWorldToLocalToPosition instead.");
    return convertWorldToLocalPosition(position);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::setController(CameraController* value)
{
    if(myController != NULL)
    {
        ModuleServices::removeModule(myController);
    }

    myController = value;
    if(myController != NULL)
    {
        myController->setCamera(this);
        ModuleServices::addModule(myController);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Camera::setCanvasTransform(const Vector3f& position, const Quaternion& orientation, const Vector3f scale)
{
    myCanvasPosition = position;
    myCanvasOrientation = orientation;
    myCanvasScale = scale;

    needUpdate();
}

///////////////////////////////////////////////////////////////////////////////
void Camera::updateFromParent(void) const
{
    SceneNode::updateFromParent();
    mDerivedOrientation = mDerivedOrientation * myCanvasOrientation;
}
