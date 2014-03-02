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
 *	A 3D model manipulator using a tracked wand
 *****************************************************************************/
#include "omegaToolkit/WandManipulator.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////
void WandManipulator::handleEvent(const Event& evt)
{
	if(evt.getServiceType() == Service::Wand && !evt.isProcessed())
	{
		// If a node is assigned to this actor and is selected, we consider mouse events consumed
		// by this actor.
		if(myNode != NULL && myNodeActive) evt.setProcessed();

		myPointerEventReceived = true;
		//myPointerEventData = 0;

		//myPointerPosition = Vector2f(evt.getPosition().x(), evt.getPosition().y());
		myPointerEventType = evt.getType();
		
		if(evt.isFlagSet(myRotateButtonFlag)) myButton2Pressed = true;
		else myButton2Pressed = false;

		if(evt.isFlagSet(myMoveButtonFlag))	myButton1Pressed = true;
		else myButton1Pressed = false;
		
		SystemManager::instance()->getDisplaySystem()->getViewRayFromEvent(evt, myPointerRay);

		if(evt.getExtraDataItems() >= 2 && evt.getExtraDataType() == Event::ExtraDataFloatArray)
		{
			float x = evt.getExtraDataFloat(0);
			float y = evt.getExtraDataFloat(1);
			
			// Thresholds
			if(x < 0.1f && x > -0.1f) x = 0;
			if(y < 0.1f && y > -0.1f) y = 0;
			
			myXAxis = x;
			myYAxis = y;
		}
		
		myWandOrientation = evt.getOrientation();
	}
}

///////////////////////////////////////////////////////////////////////////////
void WandManipulator::setup(const Setting& s)
{
	String movbtn = Config::getStringValue("moveButton", s, "Button5");
	myMoveButtonFlag = Event::parseButtonName(movbtn);
	String rotbtn = Config::getStringValue("rotateButton", s, "Button6");
	myRotateButtonFlag = Event::parseButtonName(rotbtn);
}

///////////////////////////////////////////////////////////////////////////////
bool WandManipulator::handleCommand(const String& cmd)
{
	return false;
	Vector<String> args = StringUtils::split(cmd);
	if(args[0] == "?")
	{
		// ?: print help
		omsg("WandManipulator");
		omsg("\t autonearfar [on|off] - (experimental) toggle auto near far Z on or off");
		omsg("\t depthpart [on <value>|off|near|far] - set the depth partition mode and Z threshold");
	}
	else if(args[0] == "interactor-move")
	{
		if(args.size() > 1)
		{
			if(args[1] == "on") myMoveEnabled = true;
			else if(args[1] == "off") myMoveEnabled = false;
		}
		ofmsg("WandManipulator: moveEnabled = %1%", %myMoveEnabled);
		// Mark command as handled
		return true;
	}
	else if(args[0] == "interactor-scale")
	{
		if(args.size() > 1)
		{
			if(args[1] == "on") myScaleEnabled = true;
			else if(args[1] == "off") myScaleEnabled = false;
		}
		ofmsg("WandManipulator: scaleEnabled = %1%", %myScaleEnabled);
		// Mark command as handled
		return true;
	}
	else if(args[0] == "interactor-rotate")
	{
		if(args.size() > 1)
		{
			if(args[1] == "on") myRotateEnabled = true;
			else if(args[1] == "off") myRotateEnabled = false;
		}
		ofmsg("WandManipulator: rotateEnabled = %1%", %myRotateEnabled);
		// Mark command as handled
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
void WandManipulator::update(const UpdateContext& context)
{
	// Exit immediately if we received no pointer event or if there is no node attached to this
	// interactor
	if(!myPointerEventReceived || myNode == NULL || !myNode->isVisible()) return;
	if(myPointerEventType == Event::Down && (myButton1Pressed || myButton2Pressed))
	{
		Vector3f handlePos;
		myStartWandOrientationInv = myWandOrientation.inverse();
		myStartOrientation = myWandOrientation.inverse() * myNode->getOrientation();
		if(myNode->hit(myPointerRay, &handlePos, SceneNode::HitBest))
		{
			myStartBSphere = myNode->getBoundingSphere();
			//myStartRayDirection = myPointerRay.getDirection();
			myHandlePosition = (handlePos - myNode->getPosition()); 
			myHandleDistance = (handlePos - myPointerRay.getOrigin()).norm();
			myNodeActive = true;
			
			ofmsg("handlepos %1% myHandlePosition %2%", %handlePos %myHandlePosition);
			ofmsg("Ray origin %1% Direction %2% Handle Distance: %3%", %myPointerRay.getOrigin() %myPointerRay.getDirection() %myHandleDistance);
		}
	}
	else if(myPointerEventType == Event::Up)
	{
		myNodeActive = false;
	}
	
	if(myPointerEventType == Event::Update)
	{
		// Manipulate object, if one is active.
		if(myNodeActive)
		{
			if(myButton1Pressed)
			{
				Quaternion newO = myStartWandOrientationInv * myWandOrientation;
				Vector3f hp  = newO * myHandlePosition;
				Vector3f newPos = myPointerRay.getPoint(myHandleDistance) - hp;
				myNode->setPosition(newPos);
				myNode->setOrientation(myWandOrientation * myStartOrientation);
			}
		}
		// For rotation and scaling, we do not need to point to the node directly.
		if(myButton2Pressed)
		{
			if(myRotateEnabled)
			{
				myNode->setOrientation(myWandOrientation * myStartOrientation);
			}

			if(myScaleEnabled)
			{
				if(myYAxis != 0)
				{
					float sc = 1.0f + myYAxis / 12.0f;
					myNode->scale(sc, sc, sc);
				}
			}
		}
	}
	
	myPointerEventReceived = false;
	myPointerEventType = Event::Null;
}


