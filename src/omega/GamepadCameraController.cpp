/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *  Dennis                  koracas@gmail.com
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
 * What's in this file:
 *	A gamepad-based camera controller using the gamepad analog axes
 ******************************************************************************/
#include "omega/Camera.h"
#include "omega/GamepadCameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
GamepadCameraController::GamepadCameraController() :
myStrafeMultiplier(1.0f),
myYawMultiplier(-1.0f),
myPitchMultiplier(1.0f),
myRollMultiplier(-1.0f),
mySpeedVector(Vector3f::Zero()),
myTorque(Quaternion::Identity()),
myPitch(0),
myYaw(0),
myRoll(0),
goingDown(false),
goingUp(false),
accelerate(false),
deaccelerate(false)
{
}
	
///////////////////////////////////////////////////////////////////////////////
void GamepadCameraController::handleEvent(const Event& evt)
{
	//if(!isEnabled() || evt.isProcessed()) return;
	if(evt.getServiceType() == Service::Controller)
	{
		float x = evt.getExtraDataFloat(0);
		float y = evt.getExtraDataFloat(1);
		float roll = evt.getExtraDataFloat(5) - evt.getExtraDataFloat(4);
		float yaw = evt.getExtraDataFloat(2);
		float pitch = evt.getExtraDataFloat(3);
		if (evt.isButtonDown(omicron::EventBase::ButtonUp))
			goingUp = true;
		if (evt.isButtonDown(omicron::EventBase::ButtonDown))
			goingDown = true;
		if (evt.isButtonUp(omicron::EventBase::ButtonUp))
			goingUp = false;
		if (evt.isButtonUp(omicron::EventBase::ButtonDown))
			goingDown = false;
		if (evt.isButtonDown(omicron::EventBase::Button8))
			accelerate = true;
		if (evt.isButtonDown(omicron::EventBase::Button5))
			deaccelerate = true;
		if (evt.isButtonUp(omicron::EventBase::Button8))
			accelerate = false;
		if (evt.isButtonUp(omicron::EventBase::Button5))
			deaccelerate = false;

		float z = 0;
		float speedModifier = 1.0f;
		float tresh = 0.2f;

		if (goingUp)
			z += 2 * tresh;
		if (goingDown)
			z -= 2 * tresh;
		if (accelerate)
			speedModifier *= 64;
		if (deaccelerate)
			speedModifier /= 64;
		if(abs(x) < tresh) x = 0;
		if(abs(y) < tresh) y = 0;
		if(abs(z) < tresh) z = 0;
		if(abs(yaw) < tresh) yaw = 0;
		if(abs(pitch) < tresh) pitch = 0;
		if (abs(roll) < tresh) roll = 0;

		myYaw = yaw * myYawMultiplier;
		if(myFreeFlyEnabled)
		{
			myPitch = pitch * myPitchMultiplier;
			myRoll = roll * myRollMultiplier;
		}
		else
		{
			// If freefly is disabled and trigger is pressed, pitch control is
			// used to move camera up/down
			if(z != 0) z = pitch;
			speedModifier = 1;
		}
		mySpeedVector = Vector3f(x*speedModifier, z*speedModifier, y*speedModifier) *  CameraController::mySpeed;
	}
}

///////////////////////////////////////////////////////////////////////////////
void GamepadCameraController::update(const UpdateContext& context)
{
	if(!isEnabled()) return;
	Camera* c = getCamera();
	
	if(c != NULL)
	{
		Vector3f mv = mySpeedVector * context.dt;
		// The movement vector is oriented based on Camera AND Head orientation
		// To make it work reasonably well when using an untracked controller
		// in a head-trackeAd environment (we want 'forward' to be the direction
		// the user is looking at). 
		Quaternion o = c->getOrientation() * c->getHeadOrientation();
		mv = o * mv;
		//If freefly is disabled, only the x-z components are influenced by navigation.
		if(!myFreeFlyEnabled)
		{
			mv = Vector3f(mv[0], mySpeedVector[1] * context.dt, mv[2]);
		}

		c->translate(mv, Node::TransformWorld);
		Vector3f pyr(myPitch * context.dt, myYaw * context.dt, myRoll * context.dt);
		c->rotate(Math::quaternionFromEuler(pyr), Node::TransformLocal);
	}
}

