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
*	A camera controller using a 6DOF tracked wand.
******************************************************************************/
#include "omega/Camera.h"
#include "omega/DisplaySystem.h"
#include "omega/WandCameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
WandCameraController::WandCameraController():
    myRotateSpeed(0.02f),
    myYaw(0),
    myLastPointerPosition(0, 0, 0),
    mySpeed(0, 0, 0),
    myNavigating(false),
    myOverride(false),
    myTorque(Quaternion::Identity()),
    myNavigateButton(Event::Button6)
{
    myAxisCorrection = Quaternion::Identity(); 
}

///////////////////////////////////////////////////////////////////////////////
void WandCameraController::setup(Setting& s)
{
    String navbtn = Config::getStringValue("navigationButton", s, "Button7");
    myNavigateButton = Event::parseButtonName(navbtn);
    
    String ovrbtn = Config::getStringValue("overrideButton", s, "Button6");
    myOverrideButton = Event::parseButtonName(ovrbtn);

    myWandSourceId = Config::getIntValue("wandSourceId", s, -1);
}

///////////////////////////////////////////////////////////////////////////////
void WandCameraController::handleEvent(const Event& evt)
{
    if(!isEnabled()) return;
    
    int uid = getCamera()->getTrackerUserId();

    // Process this event if the event is a wand AND
    // - the camera has a used id that is the same as this event user id OR
    // - the controller wand id is the same ad the event source id OR
    // - the controller wand id is -1 (any wand)
    if(evt.getServiceType() == Service::Wand && 
        (evt.getUserId() == uid ||
         (uid == -1 &&
         (myWandSourceId == evt.getSourceId() || 
         myWandSourceId == -1 ))))
    {
        //ofmsg("Wand %1%", %evt.getUserId());
        if(evt.isFlagSet(myOverrideButton))
        {
            myOverride = true;
            return;
        }
        else myOverride = false;
        
        float x = evt.getExtraDataFloat(0);
        float y = evt.getExtraDataFloat(1);
        
        // Thresholds
        if(x < 0.1f && x > -0.1f) x = 0;
        if(y < 0.1f && y > -0.1f) y = 0;
        
        // Move forward using wand analog control
        Quaternion orientation = evt.getOrientation() * getCamera()->getCanvasOrientation();
        //mySpeed = orientation * Vector3f(0, 0, y / 2) * 
        //    CameraController::mySpeed;
        
        if(evt.isFlagSet(myNavigateButton)) 
        {
             myYaw = -x * myRotateSpeed;
           if(myNavigating == false)
            {
                myLastPointerPosition = evt.getPosition();
                myAxisCorrection = getCamera()->getDerivedOrientation();
                Quaternion o = myAxisCorrection * evt.getOrientation();
                myLastPointerOrientation = o.inverse() * getCamera()->getOrientation();
            }
            myNavigating = true;
        }
        else
        {
            myNavigating = false;
            myYaw = 0;
        }
        
        if(myNavigating)
        {
            // Move in any direction using wand position tracking.
            Vector3f dv = (evt.getPosition() - myLastPointerPosition) *
                CameraController::mySpeed * 4;
            mySpeed += getCamera()->getCanvasOrientation() * dv;

            if(myFreeFlyEnabled)
            {
                Quaternion o = myAxisCorrection * evt.getOrientation();
                myTorque = o * myLastPointerOrientation;
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////
void WandCameraController::update(const UpdateContext& context)
{
    if(!isEnabled() || myOverride) return;
    Camera* c = getCamera();
    myTorque = c->getOrientation().slerp(context.dt * 0.2f, myTorque) * 
        AngleAxis(myYaw, Vector3f::UnitY());
    
    if(c != NULL)
    {
        c->translate(mySpeed * context.dt, Node::TransformLocal);
        c->setOrientation(myTorque);
    }
    
    // Perform speed damping only if we run at least 10fps
    if(context.dt < 0.1f) mySpeed -= mySpeed * context.dt * 5;
    else mySpeed = Vector3f::Zero();
}

