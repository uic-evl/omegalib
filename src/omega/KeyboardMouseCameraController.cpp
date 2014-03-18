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
 *	A camera controller using mouse and keyboard in FPS style:
 *  WASD keys are used to move, R,F to move up and down, mouse click and rotate
 *  to rotate the view.
 ******************************************************************************/
#include "omega/Camera.h"
#include "omega/KeyboardMouseCameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
KeyboardMouseCameraController::KeyboardMouseCameraController():
    myStrafeMultiplier(1.0f),
    myYawMultiplier(0.002f),
    myPitchMultiplier(0.002f),
    myYaw(0),
    myPitch(0),
    myMoveFlags(0),
    myRotating(false),
    myAltRotating(false),
    myHeadRotating(false)
{
}

///////////////////////////////////////////////////////////////////////////////
#define NAV_KEY(k, f) if(evt.isKeyDown(k)) myMoveFlags |= f; if(evt.isKeyUp(k)) myMoveFlags &= ~f; 
void KeyboardMouseCameraController::handleEvent(const Event& evt)
{
    if(!isEnabled() || evt.isProcessed()) return;

    if(evt.getServiceType() == Service::Keyboard)
    {
        NAV_KEY('d', MoveRight);
        NAV_KEY('a', MoveLeft);
        NAV_KEY('w', MoveForward);
        NAV_KEY('s', MoveBackward);
        NAV_KEY('r', MoveUp);
        NAV_KEY('f', MoveDown);

        myAltRotating = evt.isFlagSet(Event::Ctrl);
        myHeadRotating = evt.isFlagSet(Event::Alt);
    }
    else if(evt.getServiceType() == Service::Pointer)
    {
        if(evt.isButtonDown(Event::Left)) 
        {
            // if we are not currently rotating, reset the base orientation
            if(!myRotating) reset();
            myRotating = true;
        }
        else if(evt.isButtonUp(Event::Left))
        {
            myRotating = false;
        }
            
        if(myRotating)
        {
            Vector3f dpos = evt.getPosition() - myLastPointerPosition;
            myYaw -= dpos.x() * myYawMultiplier;
            myPitch -= dpos.y() * myPitchMultiplier;
        }
        myLastPointerPosition = evt.getPosition();
    }
}

///////////////////////////////////////////////////////////////////////////////
void KeyboardMouseCameraController::update(const UpdateContext& context)
{
    if(!isEnabled()) return;

    Camera* c = getCamera();

    Vector3f speed = computeSpeedVector(myMoveFlags, mySpeed, myStrafeMultiplier);
    if(myHeadRotating)
    {
        Quaternion no = Math::quaternionFromEuler(Vector3f(myPitch, myYaw, 0)); 
        c->setHeadOrientation(no);
    }
    else if(myRotating)
    {
        if(myAltRotating)
        {
            // Rotate using the XZ plane defined by the current camera orientation
            // (i.e. there is no privileged ground plane)
            Quaternion no = Math::quaternionFromEuler(Vector3f(myPitch, myYaw, 0)); 
            c->setOrientation(myInitialOrientation * no);
        }
        else
        {
            // Rotate keeping the XZ plane fixed in space
            // (i.e. the absolute XZ plane is the privileged ground plane,
            // making navigation work like a standard first person camera)
            c->setOrientation(
                AngleAxis(myYaw, Vector3f::UnitY()) * 
                myInitialOrientation * 
                AngleAxis(myPitch, Vector3f::UnitX()));
        }
    }
    c->translate(speed * context.dt, Node::TransformLocal);
}

///////////////////////////////////////////////////////////////////////////////
void KeyboardMouseCameraController::reset()
{
    myInitialOrientation = getCamera()->getOrientation();
    myYaw = 0;
    myPitch = 0;
    //Vector3f pyr = Math::quaternionToEuler(getCamera()->getOrientation());
    //myYaw = pyr[1];
    //myPitch = pyr[0];
}
