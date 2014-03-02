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
#include "omegaToolkit/MouseManipulator.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////////////////////////
MouseManipulator::MouseManipulator():
	Actor("MouseManipulator"),
	myMoveButtonFlag(Event::Left),
	myRotateButtonFlag(Event::Right),
	myMoveSpeed(2.0f),
	myRotateSpeed(5.0f),
	myResizeSpeed(1.1f),
	myPointerAxisMultiplier(20.0f)
	{}
			
///////////////////////////////////////////////////////////////////////////////////////////////////
void MouseManipulator::handleEvent(const Event& evt)
{
	if(myNode == NULL) return;
	if(evt.getServiceType() == Service::Pointer && !evt.isProcessed())
	{
		myPointerButton1Pressed = false;
		myPointerButton2Pressed = false;
		myPointerEventReceived = true;
		
		myPointerEventData = 0;
		// Process mouse axes.
		DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
		Vector2i resolution = ds->getCanvasSize();

		float curX = evt.getPosition().x() / resolution[0];
		float curY = evt.getPosition().y() / resolution[1];

		float dx = (curX - myLastPointerPosition[0]) * myPointerAxisMultiplier;
		float dy = -(curY - myLastPointerPosition[1]) * myPointerAxisMultiplier;

		myPointerPosition[0] = dx;
		myPointerPosition[1] = dy;

		// Save current (normalized) pointer position.
		myLastPointerPosition[0] = curX;
		myLastPointerPosition[1] = curY;


		// We just care about Up / Down events.
		//if(evt.getType() != Event::Move)
		{
			myPointerEventType = evt.getType();
		}

		if(evt.isFlagSet(myMoveButtonFlag)) myPointerButton1Pressed = true;
		if(evt.isFlagSet(myRotateButtonFlag)) myPointerButton2Pressed = true;

		if(myPointerButton1Pressed || myPointerButton2Pressed) evt.setProcessed();
		
		//if(evt.getType() == Event::Zoom) myPointerEventData = evt.getExtraDataInt(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MouseManipulator::update(const UpdateContext& context)
{
	// Exit immediately if we received no pointer event or if there is no node attached to this
	// interactor
	if(!myPointerEventReceived || myNode == NULL) return;

	if(myPointerEventType == Event::Move)
	{
		// Manipulate object, if one is active.
		if(myPointerButton1Pressed)
		{
			Vector3f speed = Vector3f(
				myPointerPosition[0] * myMoveSpeed,
				myPointerPosition[1] * myMoveSpeed,
				0);
			myNode->translate(speed * context.dt);
		}
		else if(myPointerButton2Pressed)
		{
			float yaw = myPointerPosition[0] * myRotateSpeed;
			float pitch = -myPointerPosition[1] * myRotateSpeed;
			myNode->yaw(yaw * context.dt, Node::TransformWorld);
			myNode->pitch(pitch * context.dt, Node::TransformWorld);
		}
	}
	
	if(myPointerEventType == Event::Zoom)
	{
		// Manipulate object, if one is active.
		if(myPointerEventData != 0)
		{
			float sc = 0.0f;
			if(myPointerEventData < 0) sc = 0.9f;
			else sc = 1.1f;
			myNode->scale(sc, sc, sc);
		}
	}

	myPointerEventReceived = false;
	myPointerEventType = Event::Null;
}
