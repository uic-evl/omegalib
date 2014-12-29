/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omegaToolkit/DefaultMouseInteractor.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultMouseInteractor::handleEvent(const Event& evt)
{
	if((evt.getServiceType() == Service::Pointer 
	|| evt.getServiceType() == Service::Wand) && !evt.isProcessed())
	{
		// If a node is assigned to this actor and is selected, we consider mouse events consumed
		// by this actor.
		if(myNode != NULL && myNodeActive) evt.setProcessed();

		myPointerButton1Pressed = false;
		myPointerButton2Pressed = false;
		myPointerEventReceived = true;
		myPointerEventData = 0;
		myPointerPosition = Vector2f(evt.getPosition().x(), evt.getPosition().y());
		// We just care about Up / Down events.
		//if(evt.getType() != Event::Move)
		{
			myPointerEventType = evt.getType();
		}
		if(evt.isFlagSet(myMoveButtonFlag)) myPointerButton1Pressed = true;
		if(evt.isFlagSet(myRotateButtonFlag)) myPointerButton2Pressed = true;
		
		SystemManager::instance()->getDisplaySystem()->getViewRayFromEvent(evt, myPointerRay);

		// if(evt.getExtraDataItems() == 2)
		// {
			// myPointerRay.setOrigin(evt.getExtraDataVector3(0));
			// myPointerRay.setDirection(evt.getExtraDataVector3(1));
		// }
		if(evt.getType() == Event::Zoom) myPointerEventData = evt.getExtraDataInt(0);

		updateNode();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultMouseInteractor::updateNode()
{
	// Exit immediately if we received no pointer event or if there is no node attached to this
	// interactor
	if(!myPointerEventReceived || myNode == NULL || !myNode->isVisible()) return;
	if(myPointerEventType == Event::Down)
	{
		Vector3f handlePos;
		if(myNode->hit(myPointerRay, &handlePos, SceneNode::HitBest))
		{
			myStartBSphere = myNode->getBoundingSphere();
			myStartOrientation = myNode->getOrientation();
			myStartScale = myNode->getScale()[0];
			myHandlePosition = handlePos; 
			myHandleDistance = (myHandlePosition - myPointerRay.getOrigin()).norm();
			myNodeActive = true;
			//ofmsg("Ray origin %1% Direction %2% Handle Distance: %3%", %myPointerRay.getOrigin() %myPointerRay.getDirection() %myHandleDistance);
		}
	}
	else if(myPointerEventType == Event::Up)
	{
		myNodeActive = false;
	}
	else if(myPointerEventType == Event::Move)
	{
		// Manipulate object, if one is active.
		if(myNodeActive)
		{
			if(myPointerButton1Pressed)
			{
				Vector3f newPos = myPointerRay.getPoint(myHandleDistance); //- (myHandlePosition - myStartBSphere.getCenter());
				newPos = - myHandlePosition + myStartBSphere.getCenter() + newPos;
				// If node has a parent, covernt world-space coordinates to 
				// local space coordinates.
				Node* parent = myNode->getParent();
				if(parent) newPos = parent->convertWorldToLocalPosition(newPos);
				myNode->setPosition(newPos);
			}
			else if(myPointerButton2Pressed)
			{
				Vector3f dir1 = myPointerRay.getPoint(myHandleDistance) - myStartBSphere.getCenter();
				Vector3f dir2 = myHandlePosition - myStartBSphere.getCenter();
				dir1.normalize();
				dir2.normalize();
				Quaternion rot = Math::buildRotation(-dir2, -dir1, Vector3f::Zero() );
				myNode->setOrientation(rot * myStartOrientation);

			}
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
