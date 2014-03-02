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
#include "omegaToolkit/DefaultTwoHandsInteractor.h"

using namespace omega;
using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultTwoHandsInteractor::initialize(const String& observerUpdateServiceName)
{
	myActiveUserId = 0;
	if(observerUpdateServiceName != "")
	{
		ServiceManager* sm = SystemManager::instance()->getServiceManager();
		myObserverUpdateService =  sm->findService<ObserverUpdateServiceExt>(observerUpdateServiceName);
	}
	else
	{
		myObserverUpdateService = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultTwoHandsInteractor::handleEvent(const Event& evt) 
{
	if(evt.getSourceId() == myActiveUserId)
	{
		if(!evt.isExtraDataNull(RightHand) && evt.isExtraDataNull(LeftHand))
		{
			if(!myHandsValid) omsg("HANDS OK");

			myHandsValid = true;
			myLeftHand = evt.getExtraDataVector3(LeftHand);
			myRightHand = evt.getExtraDataVector3(RightHand);
		}
		else
		{
			if(myHandsValid) omsg("NO HANDS");
			myHandsValid = false;
		}
	}

	if(myObserverUpdateService != NULL) 
	{
		myActiveUserId = myObserverUpdateService->getSourceId();
	}

	if(myNode != NULL)
	{
		if(evt.getType() == Event::Down)
		{
			if(myHandsValid)
			{
				myNode->setSelected(true);
				myInitialOrientation = myNode->getOrientation();

				myInitialScale = myNode->getScale()[0] / (myLeftHand - myRightHand).norm();
				myInitialHandDirection = myLeftHand - myRightHand;
				myInitialHandDirection.normalize();

				Vector3f handCenter = ((myLeftHand + myRightHand) / 2);
				myInitialPosition = handCenter - myNode->getPosition();

				if(evt.isFlagSet(Event::Left))
				{
					myNode->setPosition(handCenter);
				}
				myInitialPosition = handCenter - myNode->getPosition();
			}
		}
		else if(evt.getType() == Event::Up)
		{
			myNode->setSelected(false);
		}

		if(evt.getSourceId() == myActiveUserId)
		{
			if(myHandsValid)
			{
				if(myNode->isSelected())
				{
					Vector3f pos = (myLeftHand + myRightHand) / 2;
					float size = (myLeftHand - myRightHand).norm() * myInitialScale;
					
					myNode->setPosition(pos - myInitialPosition);
					myNode->setScale(size, size, size);

					Vector3f dir = myLeftHand - myRightHand;
					dir.normalize();
					Quaternion q = Math::buildRotation(myInitialHandDirection, dir, Vector3f::Zero()) * myInitialOrientation;
					myNode->setOrientation(q);
				}
			}
		}
	}
}
