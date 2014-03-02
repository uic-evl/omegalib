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
#include "omegaToolkit/ControllerManipulator.h"

using namespace omegaToolkit;

#define SET_BUTTON_STATE(btn, index) if(evt.getExtraDataFloat(index) > 0) { myButtonState[btn] = true; evt.setProcessed(); } else myButtonState[btn] = false;

///////////////////////////////////////////////////////////////////////////////////////////////////
ControllerManipulator::ControllerManipulator():
	Actor("ControllerManipulator")
{
	memset(myButtonState, 0, sizeof(bool) * MaxButtons);
	myAnalog1Position = Vector2f::Zero();
	myAnalog2Position = Vector2f::Zero();
	myTrigger = 0;
	myYaw = 0;
	myPitch = 0;
	myRoll = 0;
	myScale = 1.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ControllerManipulator::handleEvent(const Event& evt)
{
	if(evt.getServiceType() == Service::Controller && evt.getSourceId() == 0)
	{
		float n = 1000;
		float x = evt.getExtraDataFloat(1) / n;
		float y = evt.getExtraDataFloat(2) / n;
		float yaw = evt.getExtraDataFloat(3) / n;
		float pitch = evt.getExtraDataFloat(4) / n;
		float trigger = evt.getExtraDataFloat(5) / n;
		float tresh = 0.2f;

		if(x < tresh && x > -tresh) x = 0;
		if(y < tresh && y > -tresh) y = 0;
		if(yaw < tresh && yaw > -tresh) yaw = 0;
		if(pitch < tresh && pitch > -tresh) pitch = 0;
		if(trigger < tresh && trigger > -tresh) trigger = 0;

		myAnalog1Position = Vector2f(x, -y);
		myAnalog2Position = Vector2f(yaw, pitch);
		myTrigger = trigger;

		SET_BUTTON_STATE(Button1, 6);
		SET_BUTTON_STATE(Button2, 7);
		SET_BUTTON_STATE(Button3, 8);
		SET_BUTTON_STATE(Button4, 9);
		SET_BUTTON_STATE(RSButton, 10);
		SET_BUTTON_STATE(RSButton, 11);

		//ofmsg("%1% %2% %3% %4%", %x %y %yaw %pitch);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ControllerManipulator::update(const UpdateContext& context)
{
	if(myNode == NULL) return;

	if(myButtonState[RSButton])
	{
		float speed = 1.0f;
		Vector3f move(myAnalog1Position[0] * speed, myAnalog1Position[1] * speed, myTrigger * speed);

		myYaw += myAnalog2Position[1] * context.dt;
		myRoll += myAnalog2Position[0] * context.dt;

		move = myNode->getPosition() + move * context.dt;
		myNode->setPosition(move);
		
		float scaleSpeed = 0.99f;
		float scaleChange = 1.0f;
		if(myButtonState[Button1])
		{
			scaleChange = scaleSpeed;
		}
		else if(myButtonState[Button3])
		{
			scaleChange = 1.0f / scaleSpeed;
		}
		myScale *= scaleChange;
		myNode->setScale(myScale, myScale, myScale);

		Quaternion quat =  AngleAxis(myYaw, Vector3f::UnitX()) * AngleAxis(0, Vector3f::UnitY()) * AngleAxis(myRoll, Vector3f::UnitZ());
		myNode->setOrientation(quat);
	}
}
