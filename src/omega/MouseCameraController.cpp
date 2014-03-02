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
#include "omega/MouseCameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////
MouseCameraController::MouseCameraController():
	mySpeed(2.0f),
	myStrafeMultiplier(1.0f),
	myYawMultiplier(0.002f),
	myPitchMultiplier(0.002f),
	myYaw(0),
	myPitch(0),
	myMoving(false)
{
	myMoveDir = Vector3f::Zero();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MouseCameraController::handleEvent(const Event& evt)
{
	if(!isEnabled() || evt.isProcessed()) return;
	if(evt.getServiceType() == Service::Pointer)
	{
		if(evt.isFlagSet(Event::Left))
		{
			Vector3f dpos = evt.getPosition() - myLastPointerPosition;
			myYaw -= dpos.x() * myYawMultiplier;
			myPitch -= dpos.y() * myPitchMultiplier;
		}
		if(evt.isFlagSet(Event::Right))
		{
			Vector3f dpos = evt.getPosition() - myLastPointerPosition;
			myMoveDir = Vector3f(dpos.x(), 0, dpos.y()) * mySpeed;
		}
		// Mouse wheel.
		if(evt.getType() == Event::Zoom)
		{
			int wheel = evt.getExtraDataInt(0);
			myMoveDir += Vector3f(0, wheel * mySpeed, 0);
		}
		myLastPointerPosition = evt.getPosition();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MouseCameraController::update(const UpdateContext& context)
{
	if(!isEnabled()) return;
	
	Camera* c = getCamera();
	myTorque = c->getOrientation().slerp(context.dt * 0.2f, myTorque) * AngleAxis(myYaw, Vector3f::UnitY());
	
	if(c != NULL)
	{
		c->translate(myMoveDir * context.dt, Node::TransformLocal);
		//c->rotate(myTorque, Node::TransformWorld);
		//c->setOrientation(myTorque);
	}
	
	myMoveDir -= myMoveDir * context.dt * 10;
	
	reset();
	myYaw = 0;
	myPitch = 0;
}

