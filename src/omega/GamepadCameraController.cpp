/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#include "omega/Camera.h"
#include "omega/GamepadCameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////
GamepadCameraController::GamepadCameraController():
	myStrafeMultiplier(1.0f),
	myYawMultiplier(-1.0f),
	myPitchMultiplier(1.0f),
	mySpeedVector(Vector3f::Zero()),
	myTorque(Quaternion::Identity()),
	myPitch(0),
	myYaw(0)
{
}
	
///////////////////////////////////////////////////////////////////////////////////////////////////
void GamepadCameraController::handleEvent(const Event& evt)
{
	//if(!isEnabled() || evt.isProcessed()) return;
	if(evt.getServiceType() == Service::Controller)
	{
		float x = evt.getExtraDataFloat(0);
		float y = evt.getExtraDataFloat(1);
		float z = evt.getExtraDataFloat(4);
		float yaw = evt.getExtraDataFloat(2);
		float pitch = evt.getExtraDataFloat(3);
		float tresh = 0.2f;

		if(abs(x) < tresh) x = 0;
		if(abs(y) < tresh) y = 0;
		if(abs(z) < tresh) z = 0;
		if(abs(yaw) < tresh) yaw = 0;
		if(abs(pitch) < tresh) pitch = 0;
		
		myYaw = yaw * myYawMultiplier;
		myPitch = pitch * myPitchMultiplier;
		mySpeedVector = Vector3f(x, z, y) *  CameraController::mySpeed;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GamepadCameraController::update(const UpdateContext& context)
{
	if(!isEnabled()) return;
	Camera* c = getCamera();
	
	if(c != NULL)
	{
		c->translate(mySpeedVector * context.dt, Node::TransformLocal);
		c->rotate(Math::quaternionFromEuler(Vector3f(myPitch * context.dt, myYaw * context.dt, 0)), Node::TransformLocal);
		//c->setOrientation(Math::quaternionFromEuler(Vector3f(myPitch, myYaw, 0)));
	}
	
	// Perform speed damping only if we run at least 10fps
	//if(context.dt < 0.1f) mySpeedVector -= mySpeedVector * context.dt * 5;
	//else mySpeedVector = Vector3f::Zero();
}

