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
#ifndef __KEYBOARD_MOUSE_CAMERA_CONTROLLER_H__
#define __KEYBOARD_MOUSE_CAMERA_CONTROLLER_H__

#include "osystem.h"
#include "ApplicationBase.h"
#include "omega/CameraController.h"

namespace omega {
    class Camera;

    ///////////////////////////////////////////////////////////////////////////
    //! Implements a camera controller using mouse and keyboard in FPS style:
    //! WASD keys are used to move, R,F to move up and down, mouse click and rotate
    //! to rotate the view.
    class OMEGA_API KeyboardMouseCameraController: public CameraController
    {
    public:
        KeyboardMouseCameraController();
        void update(const UpdateContext& context);
        void handleEvent(const Event& evt);
        virtual void reset();

    private:
        // Navigation stuff.
        float myStrafeMultiplier;
        float myYawMultiplier;
        float myPitchMultiplier;
        uint myMoveFlags;
        bool myRotating;
        bool myAltRotating;
        bool myHeadRotating;
        Vector3f myLastPointerPosition;
        Quaternion myInitialOrientation;
        float myYaw;
        float myPitch;
    };
}; // namespace omega

#endif